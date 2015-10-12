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
 *	setup.c
 *
 *       prepare for Flash ROM write
 */

#include "flash.h"

EXPORT void ChangeMemAttr( UW top, UW end, UW attr );

/* Update Flash ROM page table so that it can be written to. */
LOCAL void flashwr_pagetable(BOOL writable)
{
	MEMSEG	*mp;
	UW	attr;

	mp = MemArea(MSA_FROM, 1);
	attr = (writable) ? (PGA_RW | PGA_D | PGA_S) : (mp->pa & 0x000fffff);
	ChangeMemAttr(mp->top, mp->end, attr);

	return;
}

/*
 * check Flash ROM write-protect status
 *       return value      E_OK            writable (OK)
 *                         E_PROTECT       write-protected
 */
EXPORT ER flashwr_protect( UW addr, W nsec )
{
	return E_OK;
}

/*
 * set up Flash ROM write
 */
EXPORT void flashwr_setup( BOOL reset )
{
        /* invalidate cache
           keep MMU enabled so that we can use WKRAM */
	DisableCache();

        /* page table is modified so that ROM area can be written to. */
	flashwr_pagetable(TRUE);
}

/*
 * post processing after Flash ROM write completed
 */
EXPORT void flashwr_done( void )
{
        /* restore the page table setting to the original. */
	flashwr_pagetable(FALSE);

        /* validate cache */
	EnableCache();
}
