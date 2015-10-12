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
 *	@(#)strftime.c
 *
 *       T2EX: calendar functions
 *       dt_strftime
 */

#include <basic.h>
#include <ctype.h>
#include <errno.h>
#include <t2ex/datetime.h>
#include "internal.h"

/* Update timezone information */
Inline ER update_tz( struct tzinfo* tz )
{
	if (tz->tzname[0][0] == '\0') {
		ER er = dt_getsystz(tz);
		if (er < E_OK) {
			return er;
		}
	}
	return (tz->tzname[0][0] != '\0') ? E_OK : EX_INVAL;
}

/* Calculate ISO 8601 week-based date */
LOCAL void update_gv(int gv[2], const struct tm *t)
{
	int k, v, d;

	/* Already calculated */
	if (gv[1] >= 0) return;

	k = (t -> tm_wday - t -> tm_yday + (53 * 7) + 6) % 7; /* day of the week of January 1st (Mon=0, Tue=1, ...) */
	v = ((k > 3) ? 1 : 0) + (t -> tm_yday + k + 1) / 7;

	if (v > 0) {
		/* The given date belongs to this week-based year */
		gv[0] = t->tm_year;
		gv[1] = v;
	}
	else {
		/* The given date belongs to the previous week-based year */
		d = 365 + (isleap(1900 + t -> tm_yday - 1) ? 1 : 0); /* the number of days for the previous year */
		k = ( k + (53 * 7) - d) % 7; /* day of the week of the previous January 1st */
		v = ((k > 3) ? 1 : 0) + (d + t -> tm_yday + k + 1) / 7;

		gv[0] = t->tm_year - 1;
		gv[1] = v;
	}
}

/* outputs a result for a single `%' specification */
LOCAL int put( const char c, const struct tm *t, char **p, int *siz, struct tzinfo* tz, int gv[2] )
{
	ER er;
	int k; const char *q;

	switch (c) {
	case 'd' :
	case 'm' :
	case 'y' :
	case 'H' :
	case 'I' :
	case 'M' :
	case 'S' :
	case 'C' :
	case 'U' :
	case 'W' :
	case 'V' :
	case 'g' :
		if (c == 'd') { /* day of the month [01,31] */
			k = t -> tm_mday;
		}
		else if (c == 'm') { /* month [01,12] */
			k = t -> tm_mon + 1;
		}
		else if (c == 'y') { /* year (2 digits) */
			k = t -> tm_year % 100;
		}
		else if (c == 'H') { /* hour [00,23] */
			k = t -> tm_hour;
		}
		else if (c == 'I') { /* hour [01,12] */
			k = ((t -> tm_hour + 11) % 12) + 1;
		}
		else if (c == 'M') { /* minute [00,59] */
			k = t -> tm_min;
		}
		else if (c == 'S') { /* second [00,59] */
			k = t -> tm_sec;
		}
		else if (c == 'C') { /* century */
			k = (t -> tm_year + 1900) / 100;
		}
		else if (c == 'U') { /* week number of the year [00,53] */
			k = (t -> tm_wday - t -> tm_yday + (53 * 7) + 6) % 7; /* day of the week of January 1st (Mon=0, Tue=1, ...) */
			k = (t -> tm_yday + k + 1) / 7;
		}
		else if (c == 'W') { /* week number of the year [00,53] */
			k = (t -> tm_wday - t -> tm_yday + (53 * 7) + 5) % 7; /* day of the week of January 1st (Tue=0, Wed=1, ...) */
			k = (t -> tm_yday + k + 1) / 7;
		}
		else if (c == 'V') { /* week number of the year [01,53] */
			update_gv(gv, t);
			k = gv[1];
		}
		else /*if (c == 'g')*/ { /* week-based year (2 digits) */
			update_gv(gv, t);
			k = (gv[0] + 1900) % 100;
		}
		*siz -= 2; if (*siz < 0) return -1;
		*(*p)++ = ((k / 10) % 10) + '0';
		*(*p)++ = ((k) % 10) + '0';
		break;
	case 'u' :
	case 'w' :
		/* day of the week (%w: Sun=0, Mon=1, ..., Sat=6 / %u: Sun=1, Mon=2, ..., Sat=7) */
		k = (t -> tm_wday) + ((c == 'u') ? 1 : 0);
		if (--*siz < 0) return -1;
		*(*p)++ = ((k) % 10) + '0';
		break;
	case 'j' :
		/* day of the year [001,366] */
		k = t -> tm_yday + 1;
		*siz -= 3; if (*siz < 0) return -1;
		*(*p)++ = ((k / 100) % 10) + '0';
		*(*p)++ = ((k / 10) % 10) + '0';
		*(*p)++ = ((k) % 10) + '0';
		break;
	case 'Y' :
	case 'G' :
		if (c == 'Y') { /* year (4 digits) */
			k = t -> tm_year + 1900;
		}
		else /*if (c == 'G')*/ { /* week-based year (4 digits) */
			update_gv(gv, t);
			k = gv[0] + 1900;
		}
		*siz -= 4; if (*siz < 0) return -1;
		*(*p)++ = ((k / 1000) % 10) + '0';
		*(*p)++ = ((k / 100) % 10) + '0';
		*(*p)++ = ((k / 10) % 10) + '0';
		*(*p)++ = ((k) % 10) + '0';
		break;
	case 'e' :
		/* day of the month [ 1,31] */
		k = t -> tm_mday;
		*siz -= 2; if (*siz < 0) return -1;
		*(*p)++ = ((k >= 10) ? ((k / 10) % 10) + '0' : ' ');
		*(*p)++ = ((k) % 10) + '0';
		break;
	case 'a' :
	case 'A' :
	case 'b' :
	case 'h' :
	case 'B' :
	case 'p' :
		if (c == 'a') { /* day of the week (short) */
			k = 3;
			q = _dt_weeknm + ((sizeof(_dt_weeknm) / 7) * t -> tm_wday);
		}
		else if (c == 'A') { /* day of the week (full) */
			k = sizeof(_dt_weeknm) / 7;
			q = _dt_weeknm + (k * t -> tm_wday);
		}
		else if (c == 'b' || c == 'h') { /* month (short) */
			k = 3;
			q = _dt_monthnm + ((sizeof(_dt_monthnm) / 12) * t -> tm_mon);
		}
		else if (c == 'B') { /* month (full) */
			k = sizeof(_dt_monthnm) / 12;
			q = _dt_monthnm + (k * t -> tm_mon);
		}
		else /*if (c == 'p')*/ { /* AM/PM */
			k = sizeof(_dt_ampm) / 2;
			q = _dt_ampm + (k * (t -> tm_hour >= 12));
		}
		while (k-- > 0) {
			if (*q <= ' ') break;
			if (--*siz < 0) return -1;
			*(*p)++ = *q++;
		}
		break;
	case 'x':
	case 'D':
		/* date (mm/dd/yy) */
		put( 'm', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = '/';
		put( 'd', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = '/';
		put( 'y', t, p, siz, tz, gv );
		break;
	case 'F':
		/* date (yyyy/mm/dd) */
		put( 'Y', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = '-';
		put( 'm', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = '-';
		put( 'd', t, p, siz, tz, gv );
		break;
	case 'X':
	case 'T':
	case 'R':
		/* time (hh:mm:ss/hh:mm) */
		put( 'H', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = ':';
		put( 'M', t, p, siz, tz, gv );
		if (c == 'R') break;
		if (--*siz < 0) return -1;
		*(*p)++ = ':';
		put( 'S', t, p, siz, tz, gv );
		break;
	case 'r':
		/* time (hh:mm:ss pp) */
		put( 'I', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = ':';
		put( 'M', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = ':';
		put( 'S', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = ' ';
		put( 'p', t, p, siz, tz, gv );
		break;
	case 'c':
		/* date and time */
		put( 'a', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = ' ';
		put( 'b', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = ' ';
		put( 'd', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = ' ';
		put( 'X', t, p, siz, tz, gv );
		if (--*siz < 0) return -1;
		*(*p)++ = ' ';
		put( 'Y', t, p, siz, tz, gv );
		break;
	case 'z':
		/* offset from UTC */
		er = update_tz(tz);
		if (er < E_OK) return -1;
		k = tz->daylight ? tz->dst_offset : tz->offset;
		if (k >= 0) {
			*(*p)++ = '-';
		}
		else {
			*(*p)++ = '+';
			k = -k;
		}
		if (--*siz < 0) return -1;
		*(*p)++ = ((k / (60 * 60) / 10) % 10) + '0';
		if (--*siz < 0) return -1;
		*(*p)++ = ((k / (60 * 60)) % 10) + '0';
		if (--*siz < 0) return -1;
		k %= (60 * 60);
		*(*p)++ = ((k / 60 / 10) % 10) + '0';
		if (--*siz < 0) return -1;
		*(*p)++ = ((k / 60) % 10) + '0';
		break;
	case 'Z':
		/* system timezone name */
		er = update_tz(tz);
		if (er < E_OK) return -1;
		q = tz->tzname[tz->daylight ? 1 : 0];
		for (k = 0; q[k] != '\0' && k < TZNAME_MAX; ) {
			if (--*siz < 0) return -1;
			*(*p)++ = q[k++];
		}
		break;
	case 't':
		/* tab character */
		if (--*siz < 0) return -1;
		*(*p)++ = '\t';
		break;
	}
	return 0;
}

/*
  strftime
*/
EXPORT int dt_strftime( char *s, size_t max, const char *format, const struct tm *tm, const struct tzinfo* tz )
{
	struct tzinfo systz;
	int c, n; const char *p; char d;
	int gv[2] = {-1, -1};

	if (tz == NULL) {
		systz.tzname[0][0] = '\0';
		tz = &systz;
	}
	else if (tz->tzname[0][0] == '\0') {
		return EX_INVAL;
	}

	n = max;
	for( p = format; (c = _dt_spec( &p, &d )); ) {
		if (c == 1) {
			if (--n < 0) goto err_ret;
			*s++ = d;
		}
		else {
			if (put( c, tm, &s, &n, (struct tzinfo*)tz, gv ) < 0) goto err_ret;
		}
	}

	*s = '\0';
	return max - n;

err_ret:
	/* Not enough space to store the output */
	return EX_RANGE;
}
