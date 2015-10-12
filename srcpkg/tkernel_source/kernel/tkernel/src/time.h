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
 *	time.h
 *
 *	C language: date and time
 */

#ifndef _TIME_
#define _TIME_

#include "stdtype.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	__size_t
typedef __size_t	size_t;
#undef	__size_t
#endif

#define NULL		0

typedef	int	clock_t;	/* Processor usage time */
typedef	int	time_t;		/* Date and time */

struct tm {
	int	tm_sec;		/* Seconds: 0 - 61
				   (including up to two leap seconds) */
	int	tm_min;		/* Minutes: 0 - 59 */
	int	tm_hour;	/* Hours: 0 - 23 */
	int	tm_mday;	/* Days: 1 - 31 */
	int	tm_mon;		/* Months (from January): 0 - 11 */
	int	tm_year;	/* Years (from 1900): 0 - */
	int	tm_wday;	/* Days (from Sunday): 0 - 6 */
	int	tm_yday;	/* Days (from January 1): 0 - 365 */
	int	tm_isdst;	/* Summer time flag: >0 for summer time,
				   =0 for normal time, <0 for unknown */
};

#ifdef __cplusplus
}
#endif
#endif /* _TIME_ */
