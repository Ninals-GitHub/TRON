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
 *	excentry.S (T2EX)
 *	T2EX: page fault handlers (em1d)
 */

#define _in_asm_source_
#include <machine.h>
#include <tk/asm.h>
#include <sys/sysinfo.h>
#include "memdef.h"

/* ------------------------------------------------------------------------ */

/*
 * Prefetch abort handler (TA_ASM)
 *
 *		+---------------+
 *	sp  ->	|R12=ip		|
 *		|R14=lr		| <- address of the aborted instruction
 *		|SPSR		|
 *		+---------------+
 */
	.text
	.balign	4
	.globl	Csym(asmIAbortHdr)
Csym(asmIAbortHdr):
	mrc	p15, 0, ip, cr5, c0, 1	// IFSR
	ldr	lr, =FSR_TypeMaskAll
	and	ip, ip, lr		// IFSR

	mov	ip, sp			// ip = ABT mode stack pointer

	cpsid	IMASK, #PSR_SVC		// enter SVC mode, disable interrupt

	stmfd	sp!, {r0-r3, fp, lr}	// save registers
	mov	fp, sp
	bic	sp, sp, #7		// align stack pointer to 8 byte boundaries
	mov	r1, ip			// r1 = ExcStack

	ldr	ip, =TASKINDP
	ldr	r3, [ip]
	add	r3, r3, #1		// enter task-independent portion
	str	r3, [ip]

	ldr	r0, =EIT_IABORT
	bl	Csym(PageFaultHdr)	// call PageFaultHdr(vecno, sp)

	cpsid	IMASK			// disable interrupt

	ldr	ip, =TASKINDP
	ldr	r3, [ip]
	sub	r3, r3, #1		// leave task-independent portion
	str	r3, [ip]

	mov	sp, fp
	cmp	r0, #0
	ldmfd	sp!, {r0-r3, fp, lr}	// restore registers
	bne	ia_goto_monitor

	TK_RET_INT PSR_ABT		// tk_ret_int()

  ia_goto_monitor:
	cps	#PSR_ABT
	ldr	lr, =Csym(DefaultHandlerEntry)
	ldr	lr, [lr]
	ldr	ip, =EITVEC(EIT_IABORT)
	bx	lr

/*
 * Data abort handler (TA_ASM)
 *
 *		+---------------+
 *	sp  ->	|R12=ip		|
 *		|R14=lr		| <- address of the aborted instruction
 *		|SPSR		|
 *		+---------------+
 */
	.text
	.balign	4
	.globl	Csym(asmDAbortHdr)
Csym(asmDAbortHdr):
	mrc	p15, 0, ip, cr5, c0, 0	// DFSR
	ldr	lr, =FSR_TypeMaskAll
	and	lr, lr, ip
	cmp	lr, #FSR_ICacheM
	mrceq	p15, 0, ip, cr5, c0, 1	// IFSR
	biceq	ip, ip, #FSR_WnR
	ldr	lr, =FSR_TypeMaskAll
	and	ip, ip, lr

	mov	ip, sp			// ip = ABT mode stack pointer

	cpsid	IMASK, #PSR_SVC		// enter SVC mode, disable interrupt

	stmfd	sp!, {r0-r3, fp, lr}	// save registers
	mov	fp, sp
	bic	sp, sp, #7		// align stack pointer to 8 byte boundary
	mov	r1, ip			// r1 = ExcStack

	ldr	ip, =TASKINDP
	ldr	r3, [ip]
	add	r3, r3, #1		// enter task-independent portion
	str	r3, [ip]

	ldr	r0, =EIT_DABORT
	bl	Csym(PageFaultHdr)	// call PageFaultHdr(vecno, sp)

	cpsid	IMASK			// disable interrupt

	ldr	ip, =TASKINDP
	ldr	r3, [ip]
	sub	r3, r3, #1		// leave task-independent portion
	str	r3, [ip]

	mov	sp, fp
	cmp	r0, #0
	ldmfd	sp!, {r0-r3, fp, lr}	// restore registers
	bne	da_goto_monitor

	TK_RET_INT PSR_ABT		// tk_ret_int()

  da_goto_monitor:
	cps	#PSR_ABT
	ldr	lr, =Csym(DefaultHandlerEntry)
	ldr	lr, [lr]
	ldr	ip, =EITVEC(EIT_DABORT)
	bx	lr

/* ------------------------------------------------------------------------ */
