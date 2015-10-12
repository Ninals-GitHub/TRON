/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */


/*
 *	sharemgr.c	Console/Low-level serial I/O driver
 *
 *	Device common manager : system-independent (partial system-dependent)
 */

#include <basic.h>
#include <libstr.h>
#include <tk/tkernel.h>
#include <tk/util.h>
#include <device/share.h>

#include "svc/ifshare.h"

/* Register/Deregister subsystem */
IMPORT	ER	con_def_subsys(W svc, W pri, void *svcent, void *brkent);

/*
 *	(common) Interrupt handler registration entry
 */
#define	N_IHVEC		256			/* The number of interrupt vectors	*/
#define	N_IHENT		64			/* The number of interrupt entries - 1 */

LOCAL	struct {
	UB	vec[N_IHVEC];			/* Interrupt vector	*/
	UB	nxt[N_IHENT];			/* Connection table		*/
	UB	cnt[N_IHENT];			/* Registration count		*/
	FP	ent[N_IHENT];			/* Interrupt handler	*/
#if CPU_MIPS
	void*	gp[N_IHENT];			/* "gp" of interrupt handler	*/
#endif
} IHT;

#if	CPU_SH3 || CPU_SH4
#define	IHVEC(dintno)		((dintno) >> 5)
#define	BAD_DINTNO(dintno)	((dintno) & 0x1f)
#else
#define	IHVEC(dintno)		(dintno)
#define	BAD_DINTNO(dintno)	(0)
#endif

LOCAL	FastLock	sha_lock;		/* Lock for exclusive access control	*/

/*
 *	Call the interrupt handler
 *
 *	* It is assumed that "gp" shall not be used for the access to IHT.
 *	* "Asm()" is used to inform compiler of the register's status of use.
 *	   It shall never be deleted since it is meaningful even if it seems to be apparently useless.
 */
Inline void call_inthdr( UINT dintno, VW inf, W i )
{
#if CPU_MIPS
	register void*	gp	asm("gp");
	void*		_gp;

	_gp = gp;
	Asm(""::"r"(gp = IHT.gp[i]));
	(*IHT.ent[i])(dintno, inf);
	Asm(""::"r"(gp = _gp));;
#else
	(*IHT.ent[i])(dintno, inf);
#endif
}

/*
 *	(common) Interrupt handler entry
 */
LOCAL	void	_inthdr(UINT dintno, VW inf)
{
	W	i;

	if ((i = IHVEC(dintno)) < N_IHVEC) {
		/* Sequentially execute the interrupt handler in order of registration */
		for (i = IHT.vec[i]; i != 0; i = IHT.nxt[i]) {
			call_inthdr(dintno, inf, i);
		}
	}
}
/*
 *	Register/Deregister (common) interrupt handler
 *		(intno >= 0 : register, < 0 : delete)
 */
LOCAL	ER	_def_inthdr(INT dintno, FP inthdr, void *gp)
{
	ER	err;
	W	del, vec, cn, pn, n;
	T_DINT	dint;

	/* Parameter check*/
	if ((del = dintno) < 0) dintno = - dintno;

	vec = IHVEC(dintno);
	if (BAD_DINTNO(dintno) || vec <= 0 || vec >= N_IHVEC
				|| inthdr == NULL) return E_PAR;

	/* Check the registration */
	for (pn = 0, cn = IHT.vec[vec]; cn != 0 && IHT.ent[cn] != inthdr;
					cn = IHT.nxt[pn = cn]);
	err = E_OK;
	if (del < 0) {		/* Deregistration */
		if (cn == 0) return E_NOEXS;	/* Unregistered */

		if (--IHT.cnt[cn] <= 0) {	/* Last registration */
			/* Delete the registration entry */
			n = IHT.nxt[cn];
			if (pn == 0) {		/* First */
				if (n == 0) {	/* Last */
					/* Delete the interrupt handler */
					err = tk_def_int(dintno, NULL);
					if (err < E_OK) return err;
					err = 1;	/* Last deletion */
				}
				IHT.vec[vec] = n;
			} else {
				IHT.nxt[pn] = n;
			}
			IHT.nxt[cn] = 0;		/* Entry deletion */
			IHT.cnt[cn] = 0;
			IHT.ent[cn] = NULL;
		}

	} else if (cn == 0) {	/* New creation */
		/* Search the empty entry for registration */
		for (cn = 1; cn < N_IHENT && IHT.ent[cn] != NULL; cn++);
		if (cn >= N_IHENT) return E_LIMIT;	/* There is no empty */

		/* Register the entry */
		IHT.cnt[cn] = 1;
		IHT.nxt[cn] = 0;
		IHT.ent[cn] = inthdr;
#if CPU_MIPS
		IHT.gp[cn] = gp;
#endif
		if (pn != 0) {
			/* Register at the end of the registered entry */
			IHT.nxt[pn] = cn;
		} else {
			/* First registration : register the interrupt handler */
			dint.intatr = TA_HLNG;
			dint.inthdr = (void*)_inthdr;
			IHT.vec[vec] = cn;
			err = tk_def_int(dintno, &dint);
			if (err < E_OK) {
				IHT.vec[vec] = 0;
				IHT.ent[cn] = NULL;
				IHT.cnt[cn] = 0;
				return err;
			}
			err = 1;	/* First registration */
		}
	} else {		/* Multiple registrations */
		IHT.cnt[cn]++;
	}
	return err;
}
/*
 *	Device common manager SVC entry
 */
EXPORT	ER	devshare_entry(void *para, W fn, void *gp)
{
	ER	ret;

	Lock(&sha_lock);	/* Exclusive lock */

	switch ( fn ) {
	case CONSIO_DEF_INTHDR_FN:
		{ CONSIO_DEF_INTHDR_PARA *p = para;
		ret = _def_inthdr(p->dintno, p->inthdr, gp); }
		break;
	default:
		ret = E_RSFN;
	}

	Unlock(&sha_lock);	/* Release the exclusive lock */

	return ret;
}
/*
 *	Start-up/ End of device common manager
 */
EXPORT	ER	devshare_startup(BOOL StartUp)
{
	if (!StartUp) {		/* End processing*/

		/* The console driver and the subsystem ID are shared.
		   Therefore, subsystem registration shall never be deregistered.*/

		/* Delete lock */
		DeleteLock(&sha_lock);

		return E_OK;
	}

	/* Initialization */
	memset(&IHT, 0, sizeof(IHT));

	/* Create lock*/
	CreateLock(&sha_lock, "SHAR");

	/* Register subsystem*/
	return con_def_subsys(CONSIO_SVC, CONSIO_PRI, devshare_entry, NULL);
}
