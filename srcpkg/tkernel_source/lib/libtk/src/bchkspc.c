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
 *	@(#)bchkspc.c (libtk)
 *
 *	Address check
 */

#include <basic.h>
#include <sys/util.h>
#include <tk/syslib.h>

/*
 * Check address space
 *	Checks to see that the memory area from address to the
 *	len byte is valid.
 */
EXPORT ER CheckSpaceR( void *address, W len )
{
	return ChkSpaceR(address, len);
}
EXPORT ER CheckSpaceRW( void *address, W len )
{
	return ChkSpaceRW(address, len);
}
EXPORT ER CheckSpaceRE( void *address, W len )
{
	return ChkSpaceRE(address, len);
}
