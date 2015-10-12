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
 *	@(#)localtime.c
 *
 *       T2EX: calendar functions
 *       dt_localtime, dt_localtime_ms, dt_localtime_us
 */

#include <basic.h>
#include <errno.h>
#include <tk/tkernel.h>
#include <t2ex/datetime.h>
#include "internal.h"

LOCAL BOOL chkDST( const struct tm* tm, union dsttimespec tspec, BOOL start )
{
	int i;
	long v, w;

	switch (tspec.j.type) {
	case DSTTIMESPEC_JULIAN_NL:
		if (isleap(tm->tm_year + 1900)
		    && tm->tm_yday > (_dt_mdays[0] + _dt_mdays[1])) {
			v = (tm->tm_yday - 1) * 24 * 60 * 60 + tm->tm_hour * 60 * 60 
				+ tm->tm_min * 60 + tm->tm_sec;
			return start ? (v >= tspec.j.offset) : (v <= tspec.j.offset);
		}
		/* FALLTHROUGH */

	case DSTTIMESPEC_JULIAN:
		v = tm->tm_yday * 24 * 60 * 60 + tm->tm_hour * 60 * 60 
			+ tm->tm_min * 60 + tm->tm_sec;
		return start ? (v >= tspec.j.offset) : (v <= tspec.j.offset);

	case DSTTIMESPEC_MWD:
		if (tm->tm_mon + 1 != tspec.m.m) {
			return start ? (tm->tm_mon + 1 > tspec.m.m) : (tm->tm_mon + 1 < tspec.m.m);
		}

		w = (tm->tm_wday - (tm->tm_mday - 1) % 7 + 7) % 7; /* wday for the first day of the month */
		v = (tspec.m.d - w + 7) % 7; /* First day of the month (origin:0) which satisfies wday = tspec.m.d */

		/* Calculate yday for tspec.m */
		w = (v + 7 * (tspec.m.n - 1)); /* Day of the month for tspec.m */
		for (i = 0; i < tm->tm_mon; i++) {
			w += _dt_mdays[i];
		}

		w = w * 24 * 60 * 60 + tspec.m.offset;
		v = tm->tm_yday * 24 * 60 * 60 + tm->tm_hour * 60 * 60 
			+ tm->tm_min * 60 + tm->tm_sec;
		return start ? (v >= w) : (v <= w);

	default:
		return FALSE;
	}
}

Inline BOOL isDST( struct tm* tm, const struct tzinfo* tz )
{
	if (tz->daylight) {
		return FALSE;
	}

	return chkDST(tm, tz->dst_start, TRUE) && chkDST(tm, tz->dst_end, FALSE);
}

EXPORT ER dt_localtime_us( SYSTIM_U tim_u, const struct tzinfo* tz, struct tm* result )
{
	struct tzinfo systz;
	ER er;

	if (tz == NULL) {
		/* If tz is not specified, use system timezone */
		er = dt_getsystz(&systz);
		if (er < E_OK) {
			return er;
		}
		tz = &systz;
	}

	/* First try with standard (non-DST) time */
	er = dt_gmtime_us(tim_u - ((SYSTIM_U)tz->offset) * 1000000, result);
	if (er < E_OK) {
		return er;
	}

	/* Check if the local time is during the DST period */
	if (!isDST(result, tz)) {
		result->tm_isdst = 0;
		return E_OK;
	}

	/* Try again with DST */
	er = dt_gmtime_us(tim_u - ((SYSTIM_U)tz->dst_offset) * 1000000, result);
	if (er < E_OK) {
		return er;
	}

	result->tm_isdst = 1;
	return E_OK;
}

EXPORT ER dt_localtime( time_t tims, const struct tzinfo* tz, struct tm* result )
{
	return dt_localtime_us(s_to_us(tims), tz, result);
}

EXPORT ER dt_localtime_ms( const SYSTIM* tim, const struct tzinfo* tz, struct tm* result )
{
	return dt_localtime_us(ms_to_us(tim), tz, result);
}
