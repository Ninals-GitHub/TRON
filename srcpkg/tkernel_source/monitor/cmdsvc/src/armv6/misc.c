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
 *	misc.c
 *
 */

#include "../cmdsvc.h"
#include <sys/rominfo.h>

/*
 * Invoking user reset initialization routine
 */
EXPORT void callUserResetInit( void )
{
	UW	wp = (UW)ROMInfo->resetinit;

	if (  invalidPC2(wp)
	  || !inMemArea(wp, wp+4, MSA_ROM|MSA_FROM)
	  ||  inMemArea(wp, wp+4, MSA_MON) ) return; /* invalid */

	callExtProg(ROMInfo->resetinit);
}

/*
 * Prepare ROM kernel execution
 *      It means that we don't execute ROM kernel immediately, but we prepare so that upon return from the ordinary monitor,
 *       it gets executed.
 */
EXPORT ER bootROM( void )
{
	UW	wp = (UW)ROMInfo->kernel;

	if (  invalidPC2(wp)
	  || !inMemArea(wp, wp+4, MSA_ROM|MSA_FROM)
	  ||  inMemArea(wp, wp+4, MSA_MON) ) return E_NOEXS; /* invalid */

        /* set boot configuration */
	setUpBoot(ROMInfo->kernel, NULL);

	return E_OK;
}
