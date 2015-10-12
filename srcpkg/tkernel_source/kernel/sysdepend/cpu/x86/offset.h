/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	offset.h (EM1-D512)
 *	TCB Offset Definition
 */

#ifndef _OFFSET_
#define _OFFSET_

/* Adjust offset of TCB member variables in offset.h for cpu_support.S */
#include <tk/config.h>

/*	TCB.wrdvno	*/
#if defined(NUM_PORID)
#define TCBSZ_POR	(4)	/* = sizeof(RNO) */
#else
#define TCBSZ_POR	(0)
#endif

/*	TCB.mtxlist	*/
#if defined(NUM_MTXID)
#define TCBSZ_MTX	(4)	/* = sizeof(MTXCB*) */
#else
#define TCBSZ_MTX	(0)
#endif

/*	TCB.winfo.xxx	*/
#if defined(NUM_PORID)
#define TCBSZ_WINFO	(16)
#else
#if defined(NUM_FLGID)
#define TCBSZ_WINFO	(12)
#else
#if defined(NUM_MBFID)||defined(NUM_MPLID)
#define TCBSZ_WINFO	(8)
#else
#if defined(NUM_SEMID)||defined(NUM_MBXID)||defined(NUM_MPFID)
#define TCBSZ_WINFO	(4)
#else
#define TCBSZ_WINFO	(0)
#endif
#endif
#endif
#endif

#define _ALIGN_CPU(x)	(((x)+3)&0xFFFFFFFC)	/* EM1-D512 : 32 bit CPU */


/*----------------------------------------------------------------------*/
/*	machine dependent data						*/
/*----------------------------------------------------------------------*/
#define	TCB_winfo	(68)		/* tskque - wercd		*/
#define	TCB_wtmeb	_ALIGN_CPU(TCB_winfo+TCBSZ_WINFO)
#define	TCBsz_wtmeb2istack	(28+TCBSZ_MTX+TCBSZ_POR+76)
					/* wtmeb - istack		*/
#define TCBSZ_GP	(0)		/* No global pointer support	*/

/*----------------------------------------------------------------------*/
/*	offset data in TCB						*/
/*----------------------------------------------------------------------*/
#define TCB_isstack	(TCB_wtmeb+TCBsz_wtmeb2istack)
#define TCB_tskctxb	_ALIGN_CPU(TCB_isstack+4+TCBSZ_GP)

#define TCB_tskid	8
#define TCB_tskatr	16
#define TCB_state	43
#define TCB_reqdct	36
#define CTXB_lsid	8
#define TA_FPU		0
#define CTXB_ssp	0
#define CTXB_uatb	4
#define CTXB_svc_ssp	12

#endif /* _OFFSET_ */
