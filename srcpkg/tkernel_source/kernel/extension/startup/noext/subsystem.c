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
 *	subsystem.c (extension)
 *	Subsystem start/stop
 */

#include "sysinit.h"

/*
 * Subsystem initialization
 *	The initialization sequence is called before kernel startup.
 *	Normally there is no need for any processing.
 */
EXPORT ER init_subsystems( void )
{
	ER	ercd = E_OK;

	/* Describe subsystem initialization sequence */

	return ercd;
}

/*
 * Subsystem start
 *	Called from initial task context after kernel startup.
 *	Drivers and subsystems used by the extensions are
 *	started at this point. */
EXPORT ER start_subsystems( void )
{
	ER	ercd = E_OK;

	/* Describe subsystem startup sequence */

	return ercd;
}

/*
 * End subsystems
 */
EXPORT ER finish_subsystems( void )
{
	ER	ercd = E_OK;

	/* Describe subsystem end sequence */

	return ercd;
}
