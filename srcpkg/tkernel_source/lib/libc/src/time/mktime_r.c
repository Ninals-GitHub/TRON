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
 *	@(#)mktime_r.c
 *
 */

#include <basic.h>
#include <errno.h>
#include <time.h>
#include <t2ex/datetime.h>

#define	TM_YEAR_BASE	1900
#define	EPOCH_JD	2440588	// 1970-01-01

#define	TZOFS(tz)	(-(tz).offset)	// time offset in seconds (east = positive, west = negative)

#define	TZDST(tz)	((tz).daylight)

LOCAL	int	gc2jd(int y, int m, int d)
{
	int	q, j;
	m -= 3; if (m < 0) { m += 12; y--; }
	j = 1721120;
	if (y < 0) {
		q = ((400 - 1) - y) / 400;
		j -= q * (365 * 400 + 97); y += q * 400;
	}
	j += (y * 365) + (y / 4) - (y / 100) + (y / 400);
	q = m / 5; j += q * (30 * 5 + 3); m -= q * 5;
	q = m / 2; j += q * (30 * 2 + 1); m -= q * 2;
	j += (m * 31) + d - 1;
	return j;
}

LOCAL	void	jd2gc(int j, int *yp, int *mp, int *dp)
{
	int	q, y, m;
	j -= 1721120;
	if (j < 0) {
		q = (((365 * 400 + 97) - 1) - j) / (365 * 400 + 97);
		j += q * (365 * 400 + 97); y = -q * 400;
	} else {
		q = j / (365 * 400 + 97);
		j -= q * (365 * 400 + 97); y = q * 400;
	}
	q = j / (365 * 100 + 24); if (q == 4) q--; j -= q * (365 * 100 + 24); y += q * 100;
	q = j / (365 * 4 + 1); j -= q * (365 * 4 + 1); y += q * 4;
	q = j / 365; if (q == 4) q--; j -= q * 365; y += q;
	q = j / (30 * 5 + 3); j -= q * (30 * 5 + 3); m = q * 5;
	q = j / (30 * 2 + 1); j -= q * (30 * 2 + 1); m += q * 2;
	q = j / 31; j -= q * 31; m += q + 3;
	if (m > 12) { y++; m -= 12; }
	*yp = y; *mp = m; *dp = j + 1;
}

LOCAL	ER	get_tz(struct tzinfo *tz, errno_t *eno)
{
	ER	er;

	er = dt_getsystz( tz );
	if (er < 0) goto e1;
	return 0;

e1:	if (eno) *eno = EINVAL;
	return er;
}

EXPORT	struct tm	*gmtime_r_eno(const time_t *clock, struct tm *result, errno_t *eno)
{
	int	jd, y, m, d;
	time_t	jdt;

	result->tm_usec = 0;
	result->tm_sec = *clock % 60;
	result->tm_min = (*clock / 60) % 60;
	result->tm_hour = (*clock / (60 * 60)) % 24;

	jd = jdt = (*clock / (24 * 60 * 60)) + EPOCH_JD;
	if (jd != jdt) goto e1;
	jd2gc( jd, &y, &m, &d );
	result->tm_mday = d;
	result->tm_mon = m - 1;
	result->tm_year = y - TM_YEAR_BASE;

	result->tm_wday = (jd + 1) % 7;
	result->tm_yday = jd - gc2jd( y, 1, 1 );
	result->tm_isdst = 0;

	return result;

e1:	if (eno) *eno = EOVERFLOW;
	return NULL;
}

EXPORT	struct tm	*gmtime_r(const time_t *clock, struct tm *result)
{
	return gmtime_r_eno(clock, result, NULL);
}

EXPORT	struct tm	*localtime_r_eno(const time_t *clock, struct tm *result, errno_t *eno)
{
	time_t	t;
	int	d;
	struct tzinfo	tz;

	if (get_tz( &tz, eno ) != 0) goto e2;
	d = TZOFS(tz);
	t = *clock + d;
	if (d >= 0) {
		if (t < *clock) goto e1;
	} else {
		if (t >= *clock) goto e1;
	}
	if (gmtime_r_eno( &t, result, eno ) == NULL) goto e2;
	result->tm_isdst = TZDST(tz);
	return result;

e1:	if (eno) *eno = EOVERFLOW;
e2:	return NULL;
}

EXPORT	struct tm	*localtime_r(const time_t *clock, struct tm *result)
{
	return localtime_r_eno(clock, result, NULL);
}

EXPORT	time_t     mktime_eno(struct tm *tm, errno_t *eno)
{
	int	y;
	time_t	t;
	struct tzinfo	tz;

	y = tm->tm_year + TM_YEAR_BASE;
	if (y <= -5000000 || y >= 5000000
		|| tm->tm_mon < 0 || tm->tm_mon >= 12) goto e1;
	t = tm->tm_sec + (tm->tm_min * 60) + (tm->tm_hour * (60 * 60))
		+ ((time_t)(gc2jd(y, tm->tm_mon + 1, tm->tm_mday) - EPOCH_JD)
		* (24 * 60 * 60));
	if (gmtime_r_eno( &t, tm, eno ) == NULL) goto e2;
	if (get_tz( &tz, eno ) != 0) goto e2;
	tm->tm_isdst = TZDST(tz);
	return t - TZOFS(tz);

e1:	if (eno) *eno = EOVERFLOW;
e2:	return -1;
}

EXPORT	time_t     mktime(struct tm *tm)
{
	return mktime_eno(tm, NULL);
}
