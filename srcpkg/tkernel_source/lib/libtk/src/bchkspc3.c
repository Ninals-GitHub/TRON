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
 *	@(#)bchkspc3.c (libtk)
 *
 *	Address check
 */

#include <basic.h>
#include <sys/util.h>
#include <tk/syslib.h>

/*
 * Check address space (B string)
 *	Checks to see that the memory area from str through to
 *	either '\0' or the max byte is valid.
 *	If max = 0, the number of bytes (max) is ignored.
 *	If the memory area is valid, the value returned is either
 *	the number of bytes or the max value (through to the max
 *	byte with max !=0 and no '\0').
 *	If the memory area is not valid, the error code is returned.
 */
EXPORT ER CheckBStrSpaceR( UB *str, W max )
{
	return ChkSpaceBstrR(str, max);
}
EXPORT ER CheckBStrSpaceRW( UB *str, W max )
{
	return ChkSpaceBstrRW(str, max);
}
