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
 *	@(#)time.h
 *
 */

#ifndef _TIME_H_
#define	_TIME_H_

#include <basic.h>
#include <errno.h>
#include <sys/types.h>

/* Calendar time */
struct tm {
	int	tm_usec;	/* microseconds [0, 999999] */
	int	tm_sec;		/* seconds [0, 60] */
	int	tm_min;		/* minutes [0, 59] */
	int	tm_hour;	/* hours [0, 23] */
	int	tm_mday;	/* day of the month [1, 31] */
	int	tm_mon;		/* month [0, 11] */
	int	tm_year;	/* year (since 1900) */
	int	tm_wday;	/* day of the week (0: Sunday) [0, 6] */
	int	tm_yday;	/* day in the year */
	int	tm_isdst;	/* daylight saving time (+: DST, 0: non-DST, -: unknown) */
};

#ifdef __cplusplus
extern "C" {
#endif

IMPORT	char	*asctime_r(const struct tm *tm, char *buf);

IMPORT	char	*ctime_r(const time_t *clock, char *buf);

IMPORT	struct tm	*gmtime_r(const time_t *clock, struct tm *result);
IMPORT	struct tm	*gmtime_r_eno(const time_t *clock, struct tm *result, errno_t *eno);
IMPORT	struct tm	*localtime_r(const time_t *clock, struct tm *result);
IMPORT	struct tm	*localtime_r_eno(const time_t *clock, struct tm *result, errno_t *eno);
IMPORT	time_t	mktime(struct tm *tm);
IMPORT	time_t	mktime_eno(struct tm *tm, errno_t *eno);
IMPORT	time_t	time(time_t *tloc);
IMPORT	double	difftime(time_t time1, time_t time0);
IMPORT	size_t	strftime(char *s, size_t max, const char *format, const struct tm *tm);
IMPORT	char	*strptime(const char *str, const char *format, struct tm *tm);

#ifdef __cplusplus
}
#endif
#endif /* _TIME_H_ */

