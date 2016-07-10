/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/04.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *    Modified by Nina Petipa at 2015/11/03
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	usermain.c (usermain)
 *	User Main (T2EX)
 */

#include <basic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

#include <cpu.h>

#include <device/std_x86/vga.h>
#include <device/std_x86/kbd.h>

/* Device drivers */
IMPORT ER ConsoleIO( INT ac, UB *av[] );
IMPORT ER ClockDrv( INT ac, UB *av[] );
IMPORT ER SysDiskDrv( INT ac, UB *av[] );
IMPORT ER ScreenDrv( INT ac, UB *av[] );
IMPORT ER KbPdDrv( INT ac, UB *av[] );
IMPORT ER LowKbPdDrv( INT ac, UB *av[] );
IMPORT ER NetDrv( INT ac, UB *av[] );

/* T2EX extension modules */
IMPORT ER dt_main( INT ac, UB* av[] );
IMPORT ER pm_main( INT ac, UB* av[] );
IMPORT ER fs_main( INT ac, UB* av[] );
IMPORT ER so_main( INT ac, UB* av[] );

/* Application program */
IMPORT void appl_main(void);

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
	ER	err = E_OK;
	
	/* Start the device drivers */
#ifdef DRV_CONSOLE
	//err = ConsoleIO(0, NULL);
	tm_putstring(err >= E_OK ? "ConsoleIO - OK\n" : "ConsoleIO - ERR\n");
	vd_printf(err >= E_OK ? "ConsoleIO - OK\n" : "ConsoleIO - ERR\n");
#endif
#ifdef DRV_CLOCK
	err = ClockDrv(0, NULL);
	tm_putstring(err >= E_OK ? "ClockDrv - OK\n" : "ClockDrv - ERR\n");
	vd_printf(err >= E_OK ? "ClockDrv - OK\n" : "ClockDrv - ERR\n");
#endif
#ifdef DRV_SYSDISK
	err = SysDiskDrv(0, NULL);
	tm_putstring(err >= E_OK ? "SysDiskDrv - OK\n" : "SysDiskDrv - ERR\n");
	vd_printf(err >= E_OK ? "SysDiskDrv - OK\n" : "SysDiskDrv - ERR\n");
#endif
#ifdef DRV_SCREEN
	//err = ScreenDrv(0, NULL);
	tm_putstring(err >= E_OK ? "ScreenDrv - OK\n" : "ScreenDrv - ERR\n");
	vd_printf(err >= E_OK ? "ScreenDrv - OK\n" : "ScreenDrv - ERR\n");
#endif
#ifdef DRV_KBPD
	//err = KbPdDrv(0, NULL);
	tm_putstring(err >= E_OK ? "KbPdDrv - OK\n" : "KbPdDrv - ERR\n");
	vd_printf(err >= E_OK ? "KbPdDrv - OK\n" : "KbPdDrv - ERR\n");
#endif
#ifdef DRV_LOWKBPD
	//err = LowKbPdDrv(0, NULL);
	tm_putstring(err >= E_OK ? "LowKbPdDrv - OK\n" : "LowKbPdDrv - ERR\n");
	vd_printf(err >= E_OK ? "LowKbPdDrv - OK\n" : "LowKbPdDrv - ERR\n");
#endif
#ifdef DRV_NET
	err = NetDrv(0, NULL);
	tm_putstring(err >= E_OK ? "NetDrv - OK\n" : "NetDrv - ERR\n");
	vd_printf(err >= E_OK ? "NetDrv - OK\n" : "NetDrv - ERR\n");
#endif
	err = initKeyboard();
	
	if (err)vd_printf("err:initKeyborad\n");
	initVga();
	
	/* Start the T2EX extension modules */
#ifdef	USE_T2EX_DT
	err = dt_main(0, NULL);
	tm_putstring(err >= E_OK ? "dt_main(0) - OK\n":"dt_main(0) - ERR\n");
	vd_printf(err >= E_OK ? "dt_main(0) - OK\n":"dt_main(0) - ERR\n");
#endif
#ifdef	USE_T2EX_PM
	err = pm_main(0, NULL);
	tm_putstring(err >= E_OK ? "pm_main(0) - OK\n":"pm_main(0) - ERR\n");
	vd_printf(err >= E_OK ? "pm_main(0) - OK\n":"pm_main(0) - ERR\n");
#endif
#ifdef	USE_T2EX_FS
	err = fs_main(0, NULL);
	tm_putstring(err >= E_OK ? "fs_main(0) - OK\n":"fs_main(0) - ERR\n");
	vd_printf(err >= E_OK ? "fs_main(0) - OK\n":"fs_main(0) - ERR\n");
#endif
#ifdef	USE_T2EX_NET
	err = so_main(0, NULL);
	tm_putstring(err >= E_OK ? "so_main(0) - OK\n":"so_main(0) - ERR\n");
	vd_printf(err >= E_OK ? "so_main(0) - OK\n":"so_main(0) - ERR\n");
#endif
	/* Initialize stdio */
	libc_stdio_init();

	/* Start the T2EX application */
	tm_putstring("*** T2EX Application program start !!\n");

	appl_main();

	/* Shutdowm the T2EX extension modules */
#ifdef	USE_T2EX_DT
	err = dt_main(-1, NULL);
	tm_putstring(err >= E_OK ? "dt_main(-1) - OK\n":"dt_main(-1) - ERR\n");
#endif
#ifdef	USE_T2EX_PM
	err = pm_main(-1, NULL);
	tm_putstring(err >= E_OK ? "pm_main(-1) - OK\n":"pm_main(-1) - ERR\n");
#endif
#ifdef	USE_T2EX_FS
	err = fs_main(-1, NULL);
	tm_putstring(err >= E_OK ? "fs_main(-1) - OK\n":"fs_main(-1) - ERR\n");
#endif
#ifdef	USE_T2EX_NET
	err = so_main(-1, NULL);
	tm_putstring(err >= E_OK ? "so_main(-1) - OK\n":"so_main(-1) - ERR\n");
#endif

	/* Stop the device drivers */
#ifdef DRV_NET
	NetDrv(-1, NULL);
#endif
#ifdef DRV_LOWKBPD
	LowKbPdDrv(-1, NULL);
#endif
#ifdef DRV_KBPD
	KbPdDrv(-1, NULL);
#endif
#ifdef DRV_SCREEN
	ScreenDrv(-1, NULL);
#endif
#ifdef DRV_SYSDISK
	SysDiskDrv(-1, NULL);
#endif
#ifdef DRV_CLOCK
	ClockDrv(-1, NULL);
#endif
#ifdef DRV_CONSOLE
	ConsoleIO(-1, NULL);
#endif

	return 0;
}

