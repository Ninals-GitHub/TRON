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
 *	@(#)knlinit.c (libtk)
 *
 *	KnlInit() is always linked as it is called from the
 *	manager startup part.
 *	Note that adding too many processing can make the
 *	program quite large.
 */

#include <basic.h>
#include <sys/commarea.h>
#include <tk/sysmgr.h>

/*
 * Kernel utility initialization
 */
EXPORT void KnlInit( void )
{
	if ( __CommArea == NULL ) {
		/* Get kernel common data area */
		_GetKernelCommonArea(&__CommArea);
	}
}
