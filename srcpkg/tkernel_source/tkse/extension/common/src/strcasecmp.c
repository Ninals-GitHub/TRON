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
 *	strcasecmp.c (common)
 *
 */

#include <basic.h>


Inline	char	_internal_tolower(const char c);

Inline char _internal_tolower(const char c)
{
	return ((c >= 'A') && (c <= 'Z')) ? (char)(c + ('a' - 'A')) : c;
}

int strcasecmp( const char *s1, const char *s2 )
{
	W	n;

	for ( ;; ) {
		if ( *s1 != *s2 ) {
			n = _internal_tolower(*s1) - _internal_tolower(*s2);
			if ( n != 0 ) {
				return n;
			}
		}
		if ( *(s1++) == '\0' ) {
			break;
		}
		s2++;
	}
	return 0;
}
