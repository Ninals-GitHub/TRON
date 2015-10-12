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
 *	cpuctrl.c
 *
 *       ARM CPU control
 */

#include "sysdepend.h"

/*
 * Location of the 1st level page table
 */
EXPORT UW* const	TopPageTable = (UW*)PAGETBL_BASE;

/* ------------------------------------------------------------------------ */
/*
 *       cache control
 *       acts on the whole address space.
 */

/*
 * turn on cache
 */
EXPORT void EnableCache( void )
{
	setCacheMMU(ENB_CACHEMMU);
}

/*
 * turn off cache
 */
EXPORT void DisableCache( void )
{
        // MMU can NOT be turned off with this CPU.
	setCacheMMU(DIS_CACHEONLY);
}

/* ------------------------------------------------------------------------ */
/*
 *      processing on monitor entry
 */

/*
 * entry
 *       info, return value is meaningless
 */
EXPORT W enterMonitor( UW info )
{
        /* cache and MMU is flushed */
	setCacheMMU(ENB_CACHEMMU);

	return 0;
}

/*
 * exit
 *       only in the case of system control processor (CP15)
 *       info is the cache and MMU mode
 *       return value is meaningless
 */
EXPORT W leaveMonitor( UW info )
{
        /* restore cache && MMU to the original state. */
	setCacheMMU(info);

	return 0;
}

/* ------------------------------------------------------------------------ */
