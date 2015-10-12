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
 *	main.c	Console/Low-level serial I/O driver
 *
 *	Main entry  : system-independent
 */

#include <basic.h>
#include <tk/tkernel.h>

/* Device common manager */
IMPORT	ER	devshare_startup( BOOL StartUp );

/* Console driver */
IMPORT	ER	console_startup( BOOL StartUp );

/* Low-level serial I/O driver */
IMPORT	ER	startup_serial_io( BOOL StartUp );

/*
 *	Register/Deregister the subsystem
 */
EXPORT	ER	con_def_subsys(W svc, W pri, void *svcent, void *brkent)
{
	T_DSSY	dssy;

	dssy.ssyatr    = TA_NULL;
	dssy.ssypri    = pri;
	dssy.svchdr    = (FP)svcent;
	dssy.breakfn   = (FP)brkent;
	dssy.startupfn = NULL;
	dssy.cleanupfn = NULL;
	dssy.eventfn   = NULL;
	dssy.resblksz  = 0;
	if (! svcent) return tk_def_ssy(svc, NULL);	/* Delete only */
	if (brkent) tk_def_ssy(svc, NULL);		/* Delete once */
	return tk_def_ssy(svc, &dssy);			/* Registration */
}

/*
 *	Console initialization/End processing entry
 */
EXPORT	ER	ConsoleIO( INT ac, UB *av[] )
{
	BOOL	start;

	start = (ac >= 0) ? TRUE : FALSE;

	/* Startup/Exit the device common manager */
	devshare_startup(start);

	/* Startup/Exit the low-level serial I/O */
	startup_serial_io(start);

	/* Startup/ Exit the console driver */
	console_startup(start);

	return E_OK;
}
