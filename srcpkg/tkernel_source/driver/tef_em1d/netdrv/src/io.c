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
 *	@(#)io.c
 *
 */

#include	"netdrv.h"

/*
 *	Memory block outputs (8 bit)	cnt : number of bytes
 */
EXPORT	void	OutMemB(UW iob, W ix, UB *buf, W cnt)
{
	UW	port = iob + (ix * IOSTEP);

	while (--cnt >= 0) {
		out_b(port, *buf++);
	}
}

/*
 *	Memory block inputs (8 bit)	cnt : number of bytes
 */
EXPORT	void	InMemB(UW iob, W ix, UB *buf, W cnt)
{
	UW	port = iob + (ix * IOSTEP);

	while (--cnt >= 0) {
		*buf++ = in_b(port);
	}
}

/*
 *	Memory block outputs (16 bit)	cnt : number of half words
 */
EXPORT	void	OutMemH(UW iob, W ix, UH *buf, W cnt)
{
	UW	port = iob + (ix * IOSTEP);

	while (--cnt >= 0) {
		out_h(port, *buf++);
	}
}

/*
 *	Memory block inputs (16 bit)	cnt : number of half words
 */
EXPORT	void	InMemH(UW iob, W ix, UH *buf, W cnt)
{
	UW	port = iob + (ix * IOSTEP);

	while (--cnt >= 0) {
		*buf++ = in_h(port);
	}
}

/*
 *	Memory block outputs (32 bit)	cnt : number of words
 */
EXPORT	void	OutMemW(UW iob, W ix, UW *buf, W cnt)
{
	UW	port = iob + (ix * IOSTEP);

	while (--cnt >= 0) {
		out_w(port, *buf++);
	}
}

/*
 *	Memory block inputs (32 bit)	cnt : number of words
 */
EXPORT	void	InMemW(UW iob, W ix, UW *buf, W cnt)
{
	UW	port = iob + (ix * IOSTEP);

	while (--cnt >= 0) {
		*buf++ = in_w(port);
	}
}

