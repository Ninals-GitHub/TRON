/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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

	ldr	r5, =__data_org		// Initialization of system 'data' area (ROM startup)
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
	ldr	r5, =__data_usr_org	// Initialization of user 'data' area (ROM startup)
	ldr	r6, =__data_usr_start
	cmp	r5, r6
	beq	nocopy_usr_data
	ldr	r7, =_edata_usr
	cmp	r6, r7
	bhs	nocopy_usr_data
  copy_usr_data:
	ldr	r4, [r5], #4
	str	r4, [r6], #4
	cmp	r6, r7
	blo	copy_usr_data
  nocopy_usr_data:

	ldr	r4, =0			// Clear system 'bss' area
	ldr	r5, =__bss_start
	ldr	r6, =_end
	cmp	r5, r6
	bhs	nobss
  clrbss:
	str	r4, [r5], #4
	cmp	r5, r6
	blo	clrbss
  nobss:
	ldr	r4, =0			// Clear user 'bss' area
	ldr	r5, =__bss_usr_start
	ldr	r6, =_end_usr
	cmp	r5, r6
	bhs	nobss_usr
  clrbss_usr:
	str	r4, [r5], #4
	cmp	r5, r6
	blo	clrbss_usr
  nobss_usr:

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
