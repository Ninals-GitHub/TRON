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
 *	icrt0.S (EM1-D512)
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

	.text
	.balign	4
	.globl	START
	.type	START, %function
START:
	/* Use the stack set by the monitor */
	ldr	ip, =PSR_SVC|PSR_DI|PSR_F	// SVC mode/Interrupt disable
	msr	cpsr_xc, ip

	/* MMU is already ON. The initial setting is done.
	   Only cache should be ON */
	mrc	p15, 0, r4, cr1, c0
	orr	r4, r4, #CR1_C
	orr	r4, r4, #CR1_I|CR1_Z
	mcr	p15, 0, r4, cr1, c0
	ldr	r4, =0
	mcr	p15, 0, r4, cr7, c7, 0	// Cache flush

	ldr	r5, =__data_org		// Initialization of 'data' area (ROM startup)
	ldr	r6, =__data_start
	subs	r10, r5, r6		// If r10 != 0, start Rom
	beq	nocopy_data
	ldr	r7, =_edata
	cmp	r6, r7
	bhs	nocopy_data
  copy_data:
	ldr	r4, [r5], #4
	str	r4, [r6], #4
	cmp	r6, r7
	blo	copy_data
  nocopy_data:

	ldr	r4, =0			// Clear 'bss' area
	ldr	r5, =__bss_start
	ldr	r6, =_end
	cmp	r5, r6
	bhs	nobss
  clrbss:
	str	r4, [r5], #4
	cmp	r5, r6
	blo	clrbss
  nobss:

	ldr	ip, =monitor_stacktop	// Monitor stack pointer for re-startup
	str	sp, [ip]		// Save
	bic	sp, sp, #7		// align stack modulo 8 bytes.

	ldr	ip, =RAM_TOP		// Low level memory manager initial setting
	ldr	r5, [ip]
	cmp	r6, r5			// _end or RAM_TOP
	movhi	r5, r6			// Either of High addresses
	ldr	ip, =lowmem_top
	str	r5, [ip]		// lowmem_top = _end or RAM_TOP
	ldr	ip, =RAM_END
	ldr	r5, [ip]
	ldr	ip, =lowmem_limit
	str	r5, [ip]		// lowmem_limit = RAM_END

	cmp	r10, #0
	blne	Csym(ROM_startup)	// Initialization at ROM startup

	bl	Csym(main)		// System startup

  l_end:				// Not suppose to return from 'main,'
	b	l_end			// but, just in case, prepare for out of control.
