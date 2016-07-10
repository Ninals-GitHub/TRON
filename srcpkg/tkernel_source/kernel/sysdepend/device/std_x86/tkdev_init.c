/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	tkdev_init.c (X86)
 *	T-Kernel Device-Dependent Initialization/Finalization
 */

#include <tk/kernel.h>
#include <tk/sysdef.h>
#include <tk/syslib.h>
#include <tm/tmonitor.h>
#include <tk/tkdev_conf.h>

#include <device/std_x86/pic.h>

extern void read_masks(void);

/*
 * Target system-dependent initialization
 */
EXPORT ER tkdev_initialize( void )
{
	/* -------------------------------------------------------------------- */
	/* initialize programmable interrupt controller				*/
	/* -------------------------------------------------------------------- */
	initPic();

	return E_OK;
}

/*
 * Target system-dependent finalization
 *	Normally jump to ROM monitor.
 *	No return from this function.
 */
EXPORT void tkdev_exit( void )
{
	disint();
	tm_exit(0);	/* Turn off power and exit */

	/* Not suppose to return from 'tm_exit,' but just in case */
	for ( ;; ) {
		tm_monitor();  /* To T-Monitor */
	}
}
