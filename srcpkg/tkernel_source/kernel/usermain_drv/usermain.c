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
 *	usermain.c (usermain)
 *	User Main
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

IMPORT ER ConsoleIO( INT ac, UB *av[] );
IMPORT ER ClockDrv( INT ac, UB *av[] );
IMPORT ER SysDiskDrv( INT ac, UB *av[] );
IMPORT ER ScreenDrv( INT ac, UB *av[] );
IMPORT ER KbPdDrv( INT ac, UB *av[] );
IMPORT ER LowKbPdDrv( INT ac, UB *av[] );

/*
 * Entry routine for the user application.
 * At this point, Initialize and start the user application.
 *
 * Entry routine is called from the initial task for Kernel,
 * so system call for stopping the task should not be issued
 * from the contexts of entry routine.
 * We recommend that:
 * (1)'usermain()' only generates the user initial task.
 * (2)initialize and start the user application by the user
 * initial task.
 */
EXPORT	INT	usermain( void )
{
	ER ercd;

        /* start the device driver */
	ercd = ConsoleIO(0, NULL);
	tm_putstring(ercd >= E_OK ? "ConsoleIO - OK\n" : "ConsoleIO - ERR\n");

	ercd = ClockDrv(0, NULL);
	tm_putstring(ercd >= E_OK ? "ClockDrv - OK\n" : "ClockDrv - ERR\n");

	ercd = SysDiskDrv(0, NULL);
	tm_putstring(ercd >= E_OK ? "SysDiskDrv - OK\n" : "SysDiskDrv - ERR\n");

	ercd = ScreenDrv(0, NULL);
	tm_putstring(ercd >= E_OK ? "ScreenDrv - OK\n" : "ScreenDrv - ERR\n");

	ercd = KbPdDrv(0, NULL);
	tm_putstring(ercd >= E_OK ? "KbPdDrv - OK\n" : "KbPdDrv - ERR\n");

	ercd = LowKbPdDrv(0, NULL);
	tm_putstring(ercd >= E_OK ? "LowKbPdDrv - OK\n" : "LowKbPdDrv - ERR\n");

	tm_putstring((UB*)"Push any key to shutdown the T-Kernel.\n");
	tm_getchar(-1);

        /* stop the device driver */
	LowKbPdDrv(-1, NULL);
	KbPdDrv(-1, NULL);
	ScreenDrv(-1, NULL);
	SysDiskDrv(-1, NULL);
	ClockDrv(-1, NULL);
	ConsoleIO(-1, NULL);

	return 0;
}
