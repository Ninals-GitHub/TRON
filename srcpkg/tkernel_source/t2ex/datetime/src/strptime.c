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
 *	@(#)strptime.c
 *
 *       T2EX: calendar functions
 *       dt_strptime
 */

#include <basic.h>
#include <ctype.h>
#include <errno.h>
#include <tk/tkernel.h>
#include <t2ex/datetime.h>
#include "internal.h"

/* FIXME: check overflow, etc. */
LOCAL int parse_uint( const char **p )
{
	char c;
	int v, len;

	for (v = 0, len = 0;; len++, (*p)++) {
		c = **p;
		if (!isdigit(c)) {
			break;
		}
		v = (v * 10) + (c - '0');
	}

	if (len <= 0) {
		return EX_INVAL;
	}
	return v;
}

Inline int compare( const char* s1, const char* s2, int len )
{
	int i;

	for (i = 0; i < len; i++) {
		if (*s2 == ' ' || *s2 == '\0')
			return i;
		if (toupper(*s1++) != toupper(*s2++))
			return i;
	}

	return len;
}

#define UPD_DATE	1	/* date */
#define HAVE_MON	2	/* month */
#define HAVE_MDAY	4	/* day of the month */
#define HAVE_WDAY	8	/* day of the week */
#define HAVE_YDAY	16	/* day of the year */
#define HAVE_CENTURY	32	/* century number */

/* parse string for a `%' specification */
LOCAL int get( const char c, struct tm *t, const char **p, unsigned int* flags )
{
	ER er;
	int i;

	switch (c) {
	case 'd' :
	case 'e' :
		/* day of the month */
		er = parse_uint(p);
		if (er < 1 || er > 31) return -1;
		t -> tm_mday = er;
		*flags |= UPD_DATE|HAVE_MDAY;
		break;
	case 'm' :
		/* month [01,12] */
		er = parse_uint(p);
		if (er < 1 || er > 12) return -1;
		t -> tm_mon = er;
		*flags |= UPD_DATE|HAVE_MON;
		break;
	case 'y' :
		/* year (2 digits) */
		er = parse_uint(p);
		if (er < 0 || er > 99) return -1;
		if (*flags & HAVE_CENTURY) {
			t -> tm_year = ((t -> tm_year) / 100) * 100 + er;
		}
		else {
			t -> tm_year = (er >= 85) ? er : (er + 100);
		}
		*flags |= UPD_DATE;
		break;
	case 'C' :
		/* century number (2 digits) */
		er = parse_uint(p);
		if (er < 0 || er > 99) return -1;
		t -> tm_year = (er - 19) * 100 + (t -> tm_year % 100);
		*flags |= UPD_DATE|HAVE_CENTURY;
		break;
	case 'H' :
		/* hour [00,23] */
		er = parse_uint(p);
		if (er < 0 || er > 23) return -1;
		t -> tm_hour = er;
		break;
	case 'I' :
		/* hour [01,12] */
		er = parse_uint(p);
		if (er < 1 || er > 12) return -1;
		t -> tm_hour = (er + 11) % 12 + 1;
		break;
	case 'M' :
		/* minute [00,59] */
		er = parse_uint(p);
		if (er < 0 || er > 59) return -1;
		t -> tm_min = er;
		break;
	case 'S' :
		/* second [00,59] */
		er = parse_uint(p);
		if (er < 0 || er > 59) return -1;
		t -> tm_sec = er;
		break;
	case 'U' :
	case 'W' :
		/* week number of the year [00,53] (simply ignored in this implementation) */
		er = parse_uint(p);
		if (er < 0 || er > 53) return -1;
		break;
	case 'u':
	case 'w':
		/* day of the week (%w: Sun=0, Mon=1, ..., Sat=6 / %u: Sun=1, Mon=2, ..., Sat=7) */
		er = parse_uint(p) - ((c == 'u') ? 1 : 0);
		if (er < 0 || er > 6) return -1;
		t -> tm_wday = er;
		*flags |= UPD_DATE|HAVE_WDAY;
	case 'j' :
		/* day of the year [001,366] */
		er = parse_uint(p);
		if (er < 1 || er > 366) return -1;
		t -> tm_yday = (er - 1);
		*flags |= UPD_DATE|HAVE_YDAY;
	case 'Y' :
		/* year */
		er = parse_uint(p);
		if (er < 1900) return -1;
		t -> tm_year = (er - 1900);
		*flags |= UPD_DATE|HAVE_CENTURY;
		break;
	case 'a' :
	case 'A' :
		/* day of the week (short/full) */
		for (i = 0; i < 7; i++) {
			er = compare(*p, _dt_weeknm + (sizeof(_dt_weeknm) / 7) * i, (sizeof(_dt_weeknm) / 7));
			if (er == (sizeof(_dt_weeknm) / 7)
			    || _dt_weeknm[(sizeof(_dt_weeknm) / 7) * i + er] == ' '
			    || (er == 3 && !isalpha((*p)[er]))) {
				*p += er;
				break;
			}
		}
		if (i == 7) return -1;
		t -> tm_wday = i;
		*flags |= UPD_DATE|HAVE_WDAY;
		break;
	case 'b' :
	case 'B' :
	case 'h' :
		/* month (short/full) */
		for (i = 0; i < 12; i++) {
			er = compare(*p, _dt_monthnm + (sizeof(_dt_monthnm) / 12) * i, (sizeof(_dt_monthnm) / 7));
			if (er == (sizeof(_dt_monthnm) / 12)
			    || _dt_monthnm[(sizeof(_dt_monthnm) / 12) * i + er] == ' '
			    || (er == 3 && !isalpha((*p)[er]))) {
				*p += er;
				break;
			}
		}
		if (i == 12) return -1;
		t -> tm_mon = i;
		*flags |= UPD_DATE|HAVE_MON;
		break;
	case 'p' :
		/* AM/PM */
		for (i = 0; i < 2; i++) {
			er = compare(*p, _dt_ampm + (sizeof(_dt_ampm) / 2) * i, (sizeof(_dt_ampm) / 2));
			if (er == (sizeof(_dt_ampm) / 2)) {
				*p += er;
				break;
			}
		}
		if (i == 2) return -1;
		t -> tm_hour = (t -> tm_hour % 12) + (i * 12);
		break;
	case 'x':
	case 'D':
		/* date (mm/dd/yy) */
		if (get( 'm', t, p, flags ) < 0) return -1; 
		if (*(*p)++ != '/') return -1;
		if (get( 'd', t, p, flags ) < 0) return -1; 
		if (*(*p)++ != '/') return -1;
		if (get( 'y', t, p, flags ) < 0) return -1;
		break;
	case 'X':
	case 'T':
	case 'R':
		/* time (hh:mm:ss) */
		if (get( 'H', t, p, flags ) < 0) return -1; 
		if (*(*p)++ != ':') return -1;
		if (get( 'M', t, p, flags ) < 0) return -1; 
		if (c == 'R') break;
		if (*(*p)++ != ':') return -1;
		if (get( 'S', t, p, flags ) < 0) return -1;
		break;
	case 'r':
		/* time (hh:mm:ss pp) */
		if (get( 'I', t, p, flags ) < 0) return -1; 
		if (*(*p)++ != ':') return -1;
		if (get( 'M', t, p, flags ) < 0) return -1; 
		if (*(*p)++ != ':') return -1;
		if (get( 'S', t, p, flags ) < 0) return -1;
		if (*(*p)++ != ' ') return -1;
		if (get( 'p', t, p, flags ) < 0) return -1;
		break;
	case 'c':
		/* date and time */
		if (get( 'a', t, p, flags ) < 0) return -1;
		if (*(*p)++ != ' ') return -1;
		if (get( 'b', t, p, flags ) < 0) return -1;
		if (*(*p)++ != ' ') return -1;
		if (get( 'd', t, p, flags ) < 0) return -1;
		if (*(*p)++ != ' ') return -1;
		if (get( 'X', t, p, flags ) < 0) return -1;
		if (*(*p)++ != ' ') return -1;
		if (get( 'Y', t, p, flags ) < 0) return -1;
		break;
	}
	return 0;
}

/*
  strptime
*/
EXPORT int dt_strptime( const char* s, const char* format, struct tm* tm )
{
	int i, c, t; const char *p; char d;
	unsigned int flags = 0;

	for( p = format; (c = _dt_spec( &p, &d )); ) {
		if (c == 1) {
			if (*s++ != d) goto err_ret;
		}
		else {
			if (get( c, tm, &s, &flags ) < 0) goto err_ret;
		}
	}

	if (flags & UPD_DATE) {
		/* recalculate tm_yday or tm_mon+tm_mday when necessary */
		if (!(flags & HAVE_YDAY)) {
			tm->tm_yday = tm->tm_mday - 1;
			for (i = 0; i < tm->tm_mon; i++) {
				tm->tm_yday += (i == 1 && isleap(tm->tm_year + 1900)) ? 29 : _dt_mdays[i];
			}
		}
		else if (!(flags & HAVE_MON) || !(flags & HAVE_MDAY)) {
			tm->tm_mday = tm->tm_yday;
			for (i = 0; i < tm->tm_mon; i++) {
				t = (i == 1 && isleap(tm->tm_year + 1900)) ? 29 : _dt_mdays[i];
				if (tm->tm_mday < t) break;
			}
			tm->tm_mday++;
			tm->tm_mon = i;
		}

		/* recalculate tm_wday when necessary */
		if (!(flags & HAVE_WDAY)) {
			tm->tm_wday = weekday(_dt_dcount(tm->tm_year + 1900) + tm->tm_yday);
		}
	}

	return E_OK;

err_ret:
	/* Parse failed */
	return EX_INVAL;
}
