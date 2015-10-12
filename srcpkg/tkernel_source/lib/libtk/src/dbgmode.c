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
 *	@(#)dbgmode.c (libtk)
 *
 *	Kernel utilities
 *	Get debug mode
 */

#include <basic.h>
#include <sys/util.h>
#include <sys/sysinfo.h>

EXPORT BOOL _isDebugMode( void )
{
	return SCInfo.bm.c.debug;
}
