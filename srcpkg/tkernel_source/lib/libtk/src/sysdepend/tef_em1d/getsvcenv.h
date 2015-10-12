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
 *	@(#)getsvcenv.h (libtk/EM1-D512)
 *
 *	Get extended SVC call environment
 */

#include <sys/sysinfo.h>

Inline UW getsvcenv( void )
{
	return SCInfo.taskmode;
}
