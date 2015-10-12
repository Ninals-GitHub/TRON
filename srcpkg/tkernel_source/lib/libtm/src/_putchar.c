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
 *	@(#)_putchar.c (libtm)
 */

#include <tm/tmonitor.h>
#include <sys/consio.h>

EXPORT int _PutChar( int c )
{
	return ( tm_putchar(c) < 0 )? -1: 1;
}
