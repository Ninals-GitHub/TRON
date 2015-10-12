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
 *	@(#)bchkspc2.c (libtk)
 *
 *	Address check
 */

#include <basic.h>
#include <sys/util.h>
#include <tk/syslib.h>

/*
 * Address space check (TC string)
 *	Checks to see that the memory area from str through to
 *	either TNULL or the max character is valid.
 *	If max = 0, the number of characters (max) is ignored.
 * 	If the memory area is valid, the value returned is either
 *	the number of characters or the max value (through to the
 *	max character with max !=0 character and no TNULL).
 *	If the memory area is not valid, the error code is returned.
*/
EXPORT ER CheckStrSpaceR( TC *str, W max )
{
	return ChkSpaceTstrR(str, max);
}
EXPORT ER CheckStrSpaceRW( TC *str, W max )
{
	return ChkSpaceTstrRW(str, max);
}
