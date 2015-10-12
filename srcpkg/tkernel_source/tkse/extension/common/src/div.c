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
 *	div.c (common)
 *
 *	Division library (ANSI compliant)
 */

#include <basic.h>
#include <stdlib.h>


/*
 * Integer division
 */
div_t div( int num, int den )
{
	div_t	d;
	d.quot = abs(num) / abs(den);
	if ( (num >= 0) != (den >= 0) ) {
		d.quot = -d.quot;
	}
	d.rem = num - (d.quot * den);
	return d;
}
