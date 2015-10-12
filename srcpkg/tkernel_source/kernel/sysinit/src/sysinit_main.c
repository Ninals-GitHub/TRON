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
 *	sysinit_main.c (sysinit)
 *	Initialize System
 */

#include "sysinit.h"
#include "patch.h"
#include <tk/util.h>
#include <sys/rominfo.h>
#include <sys/debug.h>

/*
 * Initialize sequence before T-Kernel starts
 *	Perform preparation necessary to start T-Kernel.
 */
IMPORT ER init_system( void );

/*
 * Start T-Kernel
 *	Start T-Kernel/OS and the initial task specified by 'ctsk'.
 *	
 *	T-Kernel/SM doesn't start when the initial task starts,
 *	so services of T-Kernel/SM are not available at this point.
 *	Never return from this function.
 */
IMPORT void t_kernel_main( T_CTSK *ctsk );

/*
 * Start System
 *	Start following steps.
 *	1. Start T-Kernel/SM.
 *	2. Make the task priority to '138'.
 *	3. Call start_subsystems().
 *	   At this point, start each subsystem and each device driver.
 *	Return from function after starting.
 */
IMPORT void start_system( void );

/*
 * Stop System
 *	1. Call finish_subsystems().
 *	   At this point, stop each subsystem and each device driver.
 *	2. Stop T-Kernel/SM and T-Kernel/OS.
 *	3. Do power down or re-start according to 'fin'.
 *	Never return from this function.
 *
 *	fin  =	 0 : Power off
 *		-1 : reset and re-start	(Reset -> Boot -> Start)
 *		-2 : fast re-start		(Start)
 *		-3 : Normal re-start		(Boot -> Start)
 *
 *	fin = -2 or -3 are not always supported.
 */
IMPORT void shutdown_system( INT fin );

/*
 * Main initial task sequence (sysmain)
 */
IMPORT INT init_task_main( void );

/* ------------------------------------------------------------------------ */

/*
 * Initial task
 */
EXPORT void init_task(void)
{

	INT fin;
	
#if USE_SYSDEPEND_PATCH1
	/* System-dependent processes (before start_system) */
	sysdepend_patch1();
#endif

	/* Start system */
	start_system();

#if USE_SYSDEPEND_PATCH2
	/* System-dependent processes (after start_system) */
	sysdepend_patch2();
#endif
	
	/* Initial task main */
	fin = init_task_main();

	/* Stop System */
	shutdown_system(fin);	/* Never return */
}

/*
 * Initial task creation parameter
 */
IMPORT const T_CTSK c_init_task;

/*
 * Entry for starting Kernel
 */
EXPORT int main( void )
{
	int err;
	
	DO_DEBUG( tm_monitor(); )

	/* Initialize sequence before T-Kernel starts */
	if (err = init_system()) {
		return(err);
	}

	/* Start T-Kernel */
	t_kernel_main((T_CTSK *)&c_init_task);	/* Never return */

	return 0;
}
