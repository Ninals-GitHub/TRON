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
 *	devinit.c (EM1-D512)
 *	Device-Dependent Initialization
 */

#include "sysinit.h"
#include <tk/kernel.h>
#include <tk/sysdef.h>
#include <sys/sysinfo.h>
#include <sys/rominfo.h>
#include <sys/util.h>
#include <device/devconf.h>
#include <libstr.h>

#include <cpu/x86/cpu_insn.h>

#include <debug/vdebug.h>

EXPORT	UB	*BootDeviceName;	/* Boot device name */
//LOCAL	UB	bootdevnm[L_DEVNM + 1];

/* Saved data of system common information */
EXPORT	SysCommonInfo	SaveSCInfo;

/*
 * Display the progress of start processing
 *	0x10 - 0x1F : Initialization (before kernel starts)
 *	0x20 - 0x2F : Kernel start processing
 *	0x30 - 0x3F : Start processing (after kernel starts)
 */
EXPORT void DispProgress( W n )
{
}

/* ------------------------------------------------------------------------ */

/*
 * Initialization at before startup
 *	This function is called before 'main()'.
 *	(same as 'SBOOT' for disk boot)
 */
EXPORT ER before_startup( void )
{
	/* -------------------------------------------------------------------- */
	/* initialize system common area for x86				*/
	/* -------------------------------------------------------------------- */
	initSysCommonArea();

	return E_OK;
}

/*
 * Set stack by exception mode
 */
#if 0
LOCAL void setExcStack( UW mode, UW stack )
{
	return;
}
#endif

/*
 * Initialization before T-Kernel starts
 */
#define SYSCONF_VAL_MAX (16)

EXPORT ER init_device( void )
{
	return E_OK;
}

/* ------------------------------------------------------------------------ */
/*
 * Start processing after T-Kernel starts
 *	Called from the initial task contexts.
 */
EXPORT ER start_device( void )
{
	return E_OK;
}

/* ------------------------------------------------------------------------ */
/*
 * System finalization
 *	Called just before system shutdown.
 *	Execute finalization that must be done before system shutdown.
 */
EXPORT ER finish_device( void )
{
	return E_OK;
}

/* ------------------------------------------------------------------------ */
/*
 *	Re-starting processing
 */

/*
 * Re-starting processing
 *	mode = -1		Reset and re-start	(cold boot)
 *	mode = -2		Re-start		(warm boot)
 *	mode = -3		Reboot			(normal boot)
 *	mode = 0xFFhhmmss	Re-start at hh:mm:ss
 *				0 <= hh < 24, 0 <= mm,ss < 60
 */
EXPORT ER restart_device( W mode )
{


	return E_PAR;
}
