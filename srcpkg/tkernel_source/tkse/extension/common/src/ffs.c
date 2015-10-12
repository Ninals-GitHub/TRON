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
 *	ffs.c (common)
 *
 */

#include <basic.h>


/*
 * Return the position of set bit (bit 1).
 * Search upward with the position of the least significant bit defined as 1,
 * and return the position of the first found set bit.
 * If there is no set bit, return 0.
 */
int ffs( int i )
{
	W	n;

	if ( i == 0 ) {
		return 0;
	}

	for ( n = 1; (i & 1) == 0; ++n ) {
		i = (int)((UW)i >> 1);
	}
	return n;
}
