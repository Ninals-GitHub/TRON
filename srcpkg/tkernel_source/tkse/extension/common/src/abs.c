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
 *	abs.c (common)
 *
 *	Absolute value (ANSI compliant)
 */


/*
 * Integer absolute value
 */
int abs( int j )
{
	return ( j < 0 )? -j: j;
}
