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
 *	@(#)datetime.h
 *
 *	T2EX: calendar functions
 */

#ifndef _T2EX_DATETIME_
#define _T2EX_DATETIME_

#include <basic.h>
#include <stdint.h>
#include <tk/tkernel.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum length of timezone name */
#define TZNAME_MAX	8

/* Timezone time specification */
#define DSTTIMESPEC_JULIAN	( 0 )	/* Julian day in seconds, leap day included */
#define DSTTIMESPEC_JULIAN_NL	( 1 )	/* Julian day in seconds, leap day excluded */
#define DSTTIMESPEC_MWD		( 2 )	/* Number of months, weeks, and days */

union dsttimespec {
	uint32_t	v;

	/* Julian day */
	struct julian {
		unsigned int	type: 2;	/* = DSTTIMESPEC_JULIAN or DSTTIMESPEC_JULIAN_NL */
		int		offset: 30;	/* time offset from January 1st, 0:00 (localtime) in seconds [0,31622400] */
	} j;

	/* Number of months, weeks, and days */
	struct monthweekday {
		unsigned int	type: 2;	/* = DSTTIMESPEC_MWD */
		unsigned int	m: 4;		/* month [1,12] */
		unsigned int	n: 4;		/* number of weeks [1,5] */
		unsigned int	d: 4;		/* number of days [0,6] */
		int		offset: 18;	/* time offset (seconds) [0,86400] */
	} m;
};

/* Timezone information */
struct tzinfo {
	char	tzname[2][TZNAME_MAX+1];	/* timezone name */
	long	offset;				/* time difference from UTC (seconds) */
	int	daylight;			/* daylight saving time */

	/* DST specific fields (valid only if daylight > 0) */
	long	dst_offset;			/* time difference from UTC (seconds) when DST */
	union	dsttimespec	dst_start;	/* DST start time */
	union	dsttimespec	dst_end;	/* DST end time */
};

/* Library functions */
IMPORT ER dt_main( INT ac, UB* av[] );
IMPORT ER dt_tzset( struct tzinfo* tz, const char* spec );
IMPORT ER dt_localtime( time_t tims, const struct tzinfo* tz, struct tm* result );
IMPORT ER dt_localtime_ms( const SYSTIM* tim, const struct tzinfo* tz, struct tm* result );
IMPORT ER dt_localtime_us( SYSTIM_U tim_u, const struct tzinfo* tz, struct tm* result );
IMPORT int dt_strftime( char* s, size_t max, const char* format, const struct tm* tm, const struct tzinfo* tz );
IMPORT int dt_strptime( const char* str, const char* format, struct tm* tm );
IMPORT ER dt_mktime( struct tm* tm, const struct tzinfo* tz, time_t* result );
IMPORT ER dt_mktime_ms( struct tm* tm, const struct tzinfo* tz, SYSTIM* result );
IMPORT ER dt_mktime_us( struct tm* tm, const struct tzinfo* tz, SYSTIM_U* result );
IMPORT ER dt_gmtime( time_t tim, struct tm* result );
IMPORT ER dt_gmtime_ms( const SYSTIM* tim_m, struct tm* result );
IMPORT ER dt_gmtime_us( SYSTIM_U tim_u, struct tm* result );

/*
 * Definition for interface library automatic generation (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<t2ex/datetime.h>

[PREFIX]
DT
***/

/* [BEGIN SYSCALLS] */
/* Calendar function */
/* ALIGN_NO 1 */
IMPORT ER dt_setsystz( const struct tzinfo* tz );
IMPORT ER dt_getsystz( struct tzinfo* tz );
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif /* _T2EX_DATETIME_ */

