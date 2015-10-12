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
 *	@(#)fsrcvmode.c (libtk)
 *
 *	Kernel utilities
 *	Get disk repair mode
 */

#include <basic.h>
#include <sys/util.h>
#include <sys/sysinfo.h>

EXPORT BOOL _isFsrcvMode( void )
{
	return SCInfo.bm.c.fsrcv;
}
