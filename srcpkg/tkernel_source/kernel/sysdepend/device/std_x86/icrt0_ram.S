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
 *	icrt0_ram.S (EM1-D512)
 *	System Startup
 */

#include <machine.h>
#include <tk/asm.h>

#if _Csym == 0
#define	START	_start
#else
#define	START	start
#endif

	/* Low level memory manager information */
	.comm	Csym(lowmem_top), 4	// Head of area (Low address)
	.comm	Csym(lowmem_limit), 4	// End of area (High address)

	.lcomm	monitor_stacktop, 4	// Monitor stack pointer for re-startup

#define	SYSINFO		0x30004400	// System common information
#define	RAM_TOP		(SYSINFO + 0)	// Head of ext. RAM free area
#define	RAM_END		(SYSINFO + 4)	// End of ext. RAM free area
#define	SYSCONF		(SYSINFO + 8)	// SYSCONF top

	.text
	.balign	4
	.globl	START
	.type	START, %function
START:
  l1:
	jmp	Csym(ROM_startup)	// Initialization at ROM startup

  l2:
	jmp	Csym(main)		// System startup

  l_end:				// Not suppose to return from 'main,'
	jmp	l_end			// but, just in case, prepare for out of control.
