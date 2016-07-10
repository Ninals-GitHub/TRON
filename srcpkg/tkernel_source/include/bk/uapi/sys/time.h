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
 *    Modified by Nina Petipa at 2015/12/15
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
 *	@(#)time.h
 *
 */

#ifndef	__BK_SYS_TIME_H__
#define	__BK_SYS_TIME_H__

#include <bk/typedef.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	time unit conversion
----------------------------------------------------------------------------------
*/
#define	TIME_SEC_TO_MS(sec)		(sec * 1000)
#define	TIME_MS_TO_US(ms)		(ms * 1000)
#define	TIME_SEC_TO_US(sec)		(TIME_MS_TO_US(TIME_SEC_TO_MS(sec)))

/*
----------------------------------------------------------------------------------
	time structs
----------------------------------------------------------------------------------
*/
struct timeval {
	long	tv_sec;		/* Second					*/
	long	tv_usec;	/* Microsecond					*/
};

struct timespec {
	time_t	tv_sec;		/* Second					*/
	long	tv_nsec;	/* Nanosecond					*/
};

/*
----------------------------------------------------------------------------------
	time zone
----------------------------------------------------------------------------------
*/
struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich			*/
	int	tz_dsttime;	/* type of dst correction			*/
};

/*
----------------------------------------------------------------------------------
	clock ids
----------------------------------------------------------------------------------
*/
typedef int	clockid_t;

#define	CLOCK_REALTIME			0
#define	CLOCK_MONOTONIC			1
#define	CLOCK_PROCESS_CPUTIME_ID	2
#define	CLOCK_THREAD_CPUTIME_ID		3
#define	CLOCK_MONOTONIC_RAW		4
#define	CLOCK_REALTIME_COARSE		5
#define	CLOCK_MONOTONIC_COARSE		6
#define	CLOCK_BOOTTIME			7
#define	CLOCK_REALTIME_ALARM		8
#define	CLOCK_BOOTTIME_ALARM		9
#define	CLOCK_SGI_CYCLE			10
#define	CLOCK_TAI			11

#define	MAX_CLOCK			16
#define	CLOCK_MASK			(CLOCK_REALTIME | CLOCK_MONOTONIC)
#define	CLOCK_MONO			CLOCK_MONOTONIC

/*
==================================================================================

	Management 

==================================================================================
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	/* __SYS_TIME_H__ */

