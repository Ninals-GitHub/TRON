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
 *	strchr.c (common)
 *
 */

#include <basic.h>

char* strchr(const char *str, int ch)
{
	const UB	*p = (const unsigned char*)str;
	UB		c;

	while ( (c = *p) != ch ) {
		if ( c == '\0' ) {
			return NULL;
		}
		p++;
	}
	return (char*)p;
}
