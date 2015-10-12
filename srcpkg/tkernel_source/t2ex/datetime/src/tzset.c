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
 *	@(#)tzset.c
 *
 *       T2EX: calendar functions
 *       dt_tzset
 */

#include <basic.h>
#include <ctype.h>
#include <errno.h>
#include <t2ex/datetime.h>

Inline ER add_offset( union dsttimespec* v, int offset )
{
	switch (v->j.type) {
	case DSTTIMESPEC_JULIAN:
	case DSTTIMESPEC_JULIAN_NL:
		v->j.offset += offset;
		return E_OK;
	case DSTTIMESPEC_MWD:
		v->m.offset += offset;
		return E_OK;
	default:
		return EX_INVAL;
	}
}

/* FIXME: check overflow, etc. */
LOCAL ER parse_uint( int* val, const char* spec, int i )
{
	int v, len;

	for (v = 0, len = 0;; i++, len++) {
		if (!isdigit(spec[i])) {
			break;
		}
		v = (v * 10) + (spec[i] - '0');
	}

	if (len <= 0) {
		return EX_INVAL;
	}

	*val = v;
	return i;
}

LOCAL ER parse_offset( long* offset, const char* spec, int i )
{
	BOOL plus = TRUE;
	int h, m = 0, s = 0;

	if (spec[i] == '\0') {
		return EX_INVAL;
	}

	/* Parse +/- sign */
	if (spec[i] == '+') {
		i++;
	}
	else if (spec[i] == '-') {
		plus = FALSE;
		i++;
	}

	/* Parse hour */
	i = parse_uint(&h, spec, i);
	if (i < E_OK) {
		return EX_INVAL;
	}

	/* Parse ":" */
	if (spec[i] != ':') {
		goto skip;
	}
	i++;

	/* Parse minute */
	i = parse_uint(&m, spec, i);
	if (i < E_OK) {
		return EX_INVAL;
	}

	/* Parse ":" */
	if (spec[i] != ':') {
		goto skip;
	}
	i++;

	/* Parse second */
	i = parse_uint(&s, spec, i);
	if (i < E_OK) {
		return EX_INVAL;
	}

skip:
	/* FIXME: check values */

	*offset = (h * 3600 + m * 60 + s) * (plus ? 1 : -1);
	return i;
}

LOCAL ER parse_rule( union dsttimespec* tspec, const char* spec, int i )
{
	ER er;
	union dsttimespec v;
	long offset;
	int tmp;

	/* Parse date */
	switch (spec[i]) {
	case 'J':
		/* J<num> */
		v.j.type = DSTTIMESPEC_JULIAN_NL;
		er = parse_uint(&tmp, spec, ++i);
		if (er < E_OK) {
			return er;
		}
		if (tmp < 0 || tmp > 365) {
			return EX_INVAL;
		}
		v.j.offset = tmp * 24 * 60 * 60;
		i = er;
		break;

	case 'M':
		/* M<num>.<num>.<num> */
		v.m.type = DSTTIMESPEC_MWD;
		er = parse_uint(&tmp, spec, ++i);
		if (er < E_OK) {
			return er;
		}
		if (tmp < 1 || tmp > 12) {
			return EX_INVAL;
		}
		v.m.m = tmp;
		i = er;
		if (spec[i++] != '.') {
			return EX_INVAL;
		}

		er = parse_uint(&tmp, spec, i);
		if (er < E_OK) {
			return er;
		}
		if (tmp < 1 || tmp > 5) {
			return EX_INVAL;
		}
		v.m.n = tmp;
		i = er;
		if (spec[i++] != '.') {
			return EX_INVAL;
		}

		er = parse_uint(&tmp, spec, i);
		if (er < E_OK) {
			return er;
		}
		if (tmp < 0 || tmp > 6) {
			return EX_INVAL;
		}
		v.m.d = tmp;
		i = er;
		break;

	default:
		/* <num> */
		v.j.type = DSTTIMESPEC_JULIAN;
		er = parse_uint(&tmp, spec, i);
		if (er < E_OK) {
			return er;
		}
		if (tmp < 1 || tmp > 365) {
			return EX_INVAL;
		}
		v.j.offset = (tmp - 1) * 24 * 60 * 60; /* `-1' to convert to zero-based offset */
		i = er;
		break;
	}

	/* Parse "/" */
	if (spec[i] != '/') {
		offset = 2 * 60 * 60; /* 2:00:00 if time omitted */
		goto skip;
	}
	i++;

	/* Parse time */
	er = parse_offset(&offset, spec, i);
	if (er < E_OK) {
		return er;
	}
	i = er;

skip:
	add_offset(&v, offset);
	*tspec = v;
	return i;
}

EXPORT ER dt_tzset( struct tzinfo* tz, const char* spec )
{
	int i, ts;
	ER er;

	/* ":characters" format not supported */
	if (spec[0] == ':') {
		return EX_INVAL;
	}

	/* Parse std */
	for (i = 0, ts = 0; isalpha(spec[i]) && ts < TZNAME_MAX; i++) {
		tz->tzname[0][ts++] = toupper(spec[i]);
	}
	if (ts <= 0) {
		/* No std specified */
		return EX_INVAL;
	}
	tz->tzname[0][ts] = '\0';

	/* Parse offset */
	er = parse_offset(&(tz->offset), spec, i);
	if (er < E_OK) {
		return er;
	}
	i = er;

	/* Parse dst */
	for (ts = 0; isalpha(spec[i]) && ts < TZNAME_MAX; i++) {
		tz->tzname[1][ts++] = toupper(spec[i]);
	}
	if (ts <= 0) {
		/* No dst specified */
		tz->daylight = 0;
		return E_OK;
	}
	tz->daylight = 1;
	tz->tzname[1][ts] = '\0';

	/* Parse offset */
	er = parse_offset(&(tz->dst_offset), spec, i);
	if (er >= E_OK) {
		i = er;
	}
	else {
		/* 1 hour before standard offset if omitted */
		tz->dst_offset = tz->offset - (1 * 60 * 60);
	}

	/* Parse "," */
	if (spec[i++] != ',') {
		return EX_INVAL;
	}

	/* Parse rule */
	er = parse_rule(&(tz->dst_start), spec, i);
	if (er < E_OK) {
		return EX_INVAL;
	}
	i = er;

	/* Parse "," */
	if (spec[i++] != ',') {
		return EX_INVAL;
	}

	/* Parse rule */
	er = parse_rule(&(tz->dst_end), spec, i);
	if (er < E_OK) {
		return EX_INVAL;
	}	
	i = er;

	return (spec[i] == '\0') ? E_OK : EX_INVAL;
}
