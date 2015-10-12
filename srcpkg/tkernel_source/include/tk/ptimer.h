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
 *	ptimer.h
 *
 *	Physical Timer
 */

#ifndef __TK_PTIMER_H__
#define __TK_PTIMER_H__

#include <basic.h>
#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * mode of StartPhysicalTimer
 */
#define	TA_ALM_PTMR	0	/* one shot timer */
#define	TA_CYC_PTMR	1	/* cyclic timer */
 
/*
 * Definition of Physical Timer Handler
 */
typedef struct t_dptmr {
	void	*exinf;		/* extended information */
	ATR	ptmratr;	/* physical timer attribute */
	FP	ptmrhdr;	/* physical timer handler address */
} T_DPTMR;

#define TA_ASM		0x00000000U	/* written in assembly langauge */
#define TA_HLNG		0x00000001U	/* written in high-level language */
 
/*
 * Configuration Information of Physical Timer
 */
typedef struct t_rptmr {
	UW	ptmrclk;	/* clock frequency of physical timer (Hz) */
	UW	maxcount;	/* maximum count */
	BOOL	defhdr;		/* can define a physical handler for it */
} T_RPTMR;

IMPORT ER StartPhysicalTimer( UINT ptmrno, UW limit, UINT mode );
IMPORT ER StopPhysicalTimer( UINT ptmrno );
IMPORT ER GetPhysicalTimerCount( UINT ptmrno, UW *p_count );
IMPORT ER DefinePhysicalTimerHandler( UINT ptmrno, CONST T_DPTMR *pk_dptmr );
IMPORT ER GetPhysicalTimerConfig( UINT ptmrno, T_RPTMR *pk_rptmr );

#ifdef __cplusplus
}
#endif
#endif /* __TK_PTIMER_H__ */
