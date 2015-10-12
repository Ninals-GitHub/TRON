/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	@(#)inthdr.c
 *
 */

#include	"netdrv.h"

#define	NIC_INTVEC	IV_GPIO(41)
#define	EXBIntMode(x)	SetIntMode(x, IM_ENA | IM_LEVEL | IM_LOW | IM_ASYN)
#define	EXBIntEna(x)	EnableInt(x)
#define	EXBIntDis(x)	DisableInt(x)
#define	EXBIntClr(x)	/* */
#define	EXBIntEnd(x)	/* */

/*
 *	Network adapter table for fixed vector interrupt
 */
LOCAL	NetInf	*irqinf[MAX_NETDEV] = {NULL};

/*
 *	Interrupt handler (fixed vector interrupt)
 */
LOCAL	void	FixIntHdr(INTVEC vec)
{
	INT	i;
	NetInf	*inf;

	for (i = 0; i < MAX_NETDEV; i++) {
		inf = irqinf[i];
		if (inf != NULL && inf->di.intno == vec &&
				inf->di.stat >= E_OK ) {  /* Target */
			(*(inf->inthdr))(inf);	/* Execute interrupt handler */
			break;
		}
	}
	/* IRC interrupt finish processing */
	EXBIntClr(vec);
	EXBIntEnd(vec);
}

/*
 *	Define interrupt handler
 */
EXPORT	ER	DefIntHdr(NetInf *inf, BOOL regist)
{
	ER	er;
	T_DINT	dint;
	W	vec;

	dint.intatr = TA_HLNG;

	/* Fixed vector hardware */
	if ((vec = inf->di.intno) <= 0) {
		inf->di.intno = vec = NIC_INTVEC;
	}
	irqinf[inf->netno] = (regist) ? inf : NULL;
	dint.inthdr = (VP)FixIntHdr;
	er = tk_def_int(vec, (regist) ? &dint : NULL);

	if (er >= E_OK && regist) {
		EXBIntMode(vec);
		EXBIntEna(vec);
	} else {
		EXBIntDis(vec);
	}

	return er;
}

