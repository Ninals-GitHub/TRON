/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	sysstartup.c (sysinit)
 *	Start/Stop System
 */

#include "sysinit.h"
#include <tk/kernel.h>
#include <bk/bk.h>
#include <bk/bprocess.h>
#include <bk/memory/slab.h>
#include <bk/memory/vm.h>

/*
 * Subsystem
 */
IMPORT ER init_subsystems( void );
IMPORT ER start_subsystems( void );
IMPORT ER finish_subsystems( void );

/*
 * Manager/Driver
 */
IMPORT ER init_segmgr( void );			/* Segment management */
IMPORT ER start_segmgr( void );			/* Segment management */
IMPORT ER finish_segmgr( void );		/* Segment management */
IMPORT ER init_memmgr( void );			/* Memory management */
IMPORT ER start_memmgr( void );			/* Memory management */
IMPORT ER finish_memmgr( void );		/* Memory management */
IMPORT ER SystemManager( INT ac, UB *av[] );	/* T-Kernel/SM */
IMPORT ER init_Imalloc( void );			/* T-Kernel/SM */
IMPORT void _InitLibtk(void);			/* libtk */
IMPORT void _FinishLibtk(void);			/* libtk */
IMPORT void t_kernel_exit( void );		/* T-Kernel/OS */

/*
 * Initial task priority
 */
#define InitTaskPri	(138)

/*
 * Initialize sequence before T-Kernel starts
 */
EXPORT ER init_system( void )
{
	ER	ercd;

	/* Platform dependent initialize sequence */
	DispProgress(0x10);
	ercd = init_device();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Initialize subsystem */
	DispProgress(0x11);
	ercd = init_subsystems();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Initialize segment manager */
	DispProgress(0x12);
	ercd = init_segmgr();
	if ( ercd < E_OK ) {
		goto err_ret;
	}
#ifdef _BTRON_
	ercd = init_bk_early();
	if (ercd < E_OK) {
		vd_printf("error init_bk_early\n");
		goto err_ret;
	}

#endif
	/* Initialize memory manager */
	DispProgress(0x13);
	ercd = init_memmgr();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

#ifdef _BTRON_
	/* Initialize slab allocator */
	DispProgress(0x14);
	ercd = init_slab_allocator();
	if ( ercd < E_OK ) {
		vd_printf("init_slab_allocator\n");
		goto err_ret;
	}
	
	/* Initialize bk */
	DispProgress(0x15);
	ercd = init_bk();
	if ( ercd < E_OK ) {
		vd_printf("init_bk\n");
		goto err_ret;
	}
#else
	/* Initialize Imalloc */
	DispProgress(0x14);
	ercd = init_Imalloc();
	if ( ercd < E_OK ) {
		goto err_ret;
	}
#endif

	return(ercd);

err_ret:
#if USE_KERNEL_MESSAGE
	tm_putstring((UB*)"!ERROR! init_kernel\n");
#endif
	//tm_monitor(); /* Stop */
	return(ercd);
}

/*
 * Start system
 */
EXPORT void start_system( void )
{
	ER	ercd;

	/* Initialize segment manager */
	DispProgress(0x30);
	ercd = start_segmgr();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

#ifndef _BTRON_
	/* Start memory manager */
	DispProgress(0x31);
	ercd = start_memmgr();
	if ( ercd < E_OK ) {
		goto err_ret;
	}
#endif

	/* Initialize system manager */
	DispProgress(0x32);
	ercd = SystemManager(0, NULL);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Initialize library (libtk)
	   V/K/Smalloc is available after this. */
	DispProgress(0x33);
	_InitLibtk();

	/* Lower the initial task priority to initialize sequence
	   for the subsystem task at first. */
	tk_chg_pri(TSK_SELF, InitTaskPri);

	/* Start system dependent sequence */
	DispProgress(0x34);
	ercd = start_device();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Start subsystem */
	DispProgress(0x35);
	ercd = start_subsystems();
	if ( ercd < E_OK ) {
		goto err_ret;
	}
	
#ifdef _BTRON_
	ercd = init_bk_lately();
	if (ercd < E_OK) {
		vd_printf("error init_bk_lately\n");
		goto err_ret;
	}

#endif

	return;

err_ret:
#if USE_KERNEL_MESSAGE
	tm_putstring((UB*)"!ERROR! start_system\n");
#endif
	tm_monitor();	/* Stop */
}

/*
 * Stop system
 */
EXPORT void shutdown_system( INT fin )
{
	/* Stop subsystem */
	finish_subsystems();

	/* Platform dependent finalize sequence */
	finish_device();

	/* Stop library (libtk) */
	_FinishLibtk();

	/* Shutdown message output */
#if USE_KERNEL_MESSAGE
	if ( fin >= 0 ) {
	  tm_putstring((UB*)"\n<< SYSTEM SHUTDOWN >>\n");
	}
#endif
	/* Stop T-Kernel/SM */
	SystemManager(-1, NULL);

#ifndef _BTRON_
	/* Stop memory manager */
	finish_memmgr();
#endif

	/* Stop segment manager */
	finish_segmgr();

	if ( fin < 0 ) {
		/* Re-start sequence (platform dependent) */
		restart_device(fin);
	}

	/* Stop system */
	t_kernel_exit();
}
