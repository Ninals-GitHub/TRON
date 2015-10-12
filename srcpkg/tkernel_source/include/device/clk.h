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
 *	clk.h
 *
 *	Clock driver
 */

#ifndef __DEVICE_CLK_H__
#define __DEVICE_CLK_H__

#include <basic.h>
#include <tk/devmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CLOCK/data number */
typedef enum {
	/* Common attribute */
	DN_CKEVENT	= TDN_EVENT,
	/* Individual attribute */
	DN_CKDATETIME	= -100,
	DN_CKAUTOPWON	= -101,
	/* Model-dependent function */
	DN_CKREGISTER	= -200
} ClockDataNo;

/* Access of nonvolatile register */
typedef struct {
	W	nreg;		/* Number of accessed registers */
	struct ck_reg {
		W	regno;	/* Intended register number */
		UW	data;	/* Intended data */
	} c[1];
} CK_REGS;
#define	CK_REGS_SZ(nreg)	( sizeof(W) + sizeof(struct ck_reg) * (nreg) )

/* Calendar date & time definitions */
#ifndef __date_tim__
#define __date_tim__
typedef struct {
	W	d_year; 	/* Offset from 1900 (85 -)		*/
	W	d_month;	/* Month (1 - 12, 0)			*/
	W	d_day;		/* Day (1 - 31)				*/
	W	d_hour; 	/* Hour (0 - 23)			*/
	W	d_min;		/* Minute (0 - 59)			*/
	W	d_sec;		/* Second (0 - 59)			*/
	W	d_week; 	/* Week (1 - 54)	(*)Not used	*/
	W	d_wday; 	/* Day of week ( 0 - 6; 0 is Sunday)	*/
	W	d_days; 	/* Day (1 - 366)	(*)Not used	*/
} DATE_TIM;
#endif /* __date_tim__ */

/* Event notification */
typedef T_DEVEVT_ID	ClockEvt;

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_CLK_H__ */
