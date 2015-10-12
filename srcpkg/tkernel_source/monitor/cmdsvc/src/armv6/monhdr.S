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
 *	monhdr.S
 *
 *       Monitor handler (after ARMv6)
 */

#define	_in_asm_source_

#include <machine.h>
#include <sys/sysinfo.h>
#include "cpudep.h"

/*
 * Monitor entry (registered as default handler)
 *		+---------------+
 *       sp  -> |R3             | <- only in the case of interrupt
 *		+---------------+
 *		|R12=ip		|
 *              |R14=lr         | <- return address from exception / interupt
 *		|SPSR		|
 *		+---------------+
 *      registers upon handler invocation
 *       r3 = varies according machine type and situation (only in the case of interrupt)
 *       ip = vector table address
 *       lr = indeterminate
 */
	.text
	.balign	4
	.globl	Csym(_defaultHdr)
	.type	Csym(_defaultHdr), %function
Csym(_defaultHdr):
        // save register
	//	regStack[0-7]		r0 .. r7
	//		[8,9]		Entry cspr, ip
	//		[10]		return cspr
	//		[11]		return r15(pc)
	//		[12-18]	   USR:	r8 ..r12, sp, lr
	//		[19-26]	   FIQ:	spsr, r8 ..r12, sp, lr
	//		[27-29]	   IRQ:	spsr, sp, lr
	//		[30-32]	   ABT:	spsr, sp, lr
	//		[33-35]	   UND:	spsr, sp, lr
	//		[36-38]	   SVC:	spsr, sp, lr
	//		[39]       CP15: SCTLR             (CP15.c1.0.c0.0)
	//		[40-42]          TTBR0,TTBR1,TTBCR (CP15.c2.0.c0.0 - 2)
	//		[43]             DACR              (CP15.c3.0.c0.0)
	//		[44-45]          DFSR,IFSR         (CP15.c5.0.c0.0 - 1)
	//		[46-47]          DFAR,IFAR         (CP15.c6.0.c0.0,2)
	//		[48]             CTXIDR            (CP15.c13.0.c0.1)

        // save r0 .. r7
	ldr	lr, =Csym(regStack)
	stmia	lr!, {r0-r2}		// r0 .. r2

        // restore R3 inside stack in the case of interrupt
	mrs	r1, cpsr		// cpsr -> r1
	and	r0, r1, #PSR_M(31)
	cmp	r0, #PSR_FIQ
	cmpne	r0, #PSR_IRQ
	ldmeqfd sp!, {r3}		// in the case of interrupt

	stmia	lr!, {r3-r7}		// r3 .. r7
	mov	r7, lr			// regStack -> r7

        // save the status on entry (cpsr, ip)
	cpsid	aif			// disable FIQ and IRQ
	stmia	r7!, {r1, ip}		// Entry cspr & ip saved

        // restore ip, lr, spsr from the values inside stack and return
	ldr	r2, =EIT_VECTBL
	sub	r0, ip, r2
	mov	r0, r0, asr #2		// interrupt/exception vector number -> r0
	ldmfd	sp!, {ip, lr}		// restore ip and lr
	ldmfd	sp!, {r2}		// r2 <- spsr restored
	stmia	r7!, {r2, lr}		// save spsr, lr(pc)

        // save registers of each mode
	stmia	r7, {r8-r12,sp,lr}^	// usr: r8 .. r12,sp,lr
	add	r7, r7, #(4*7)

	cps	#PSR_FIQ
	mrs	r3, spsr
	stmia	r7!, {r3, r8-r12,sp,lr}	// fiq: spsr, r8 .. r12,sp,lr

	cps	#PSR_IRQ
	mrs	r3, spsr
	stmia	r7!, {r3, sp, lr}	// irq: spsr, sp, lr

	cps	#PSR_ABT
	mrs	r3, spsr
	stmia	r7!, {r3, sp, lr}	// abt: spsr, sp, lr

	cps	#PSR_UND
	mrs	r3, spsr
	stmia	r7!, {r3, sp, lr}	// und: spsr, sp, lr

	cps	#PSR_SVC
	mrs	r3, spsr
	stmia	r7!, {r3, sp, lr}	// svc: spsr, sp, lr

	mrc	p15, 0, r2, cr1, cr0, 0
	mrc	p15, 0, r3, cr2, cr0, 0
	mrc	p15, 0, r4, cr2, cr0, 1
	mrc	p15, 0, r5, cr2, cr0, 2
	mrc	p15, 0, r6, cr3, cr0, 0
	stmia	r7!, {r2,r3,r4,r5,r6}		// cp15: r1,r2,r3

	mrc	p15, 0, r2, cr5, cr0, 0
	mrc	p15, 0, r3, cr5, cr0, 1
	mrc	p15, 0, r4, cr6, cr0, 0
	mrc	p15, 0, r5, cr6, cr0, 2
	mrc	p15, 0, r6, cr13, cr0, 1	// cp15: r5,r6,r13
	stmia	r7!, {r2,r3,r4,r5,r6}

	ldr	r2, =0xFFFFFFFF
	mcr	p15, 0, r2, cr3, cr0, 0	// cp15:r3(domain) manager

        // set up stack exclusively used for monitor (SVC mode)
	ldr	r2, =__stack_top
	ldr	r3, =__stack_bottom	// stack exclusively used for monitor
	cmp	sp, r2
	cmpcs	r3, sp			// using monitor stack?
	movcc	sp, r3			// switch to monitor stack

        // monitor entry: r0 = interrupt/exception vector number (r1 = cpsr) : SVC mode
	bl	Csym(entMonitor)	// call entMonitor(vec)

        // restore registers
	ldr	r7, =Csym(regStack) + ((39 + 10) * 4)

	ldmdb	r7!, {r2,r3,r4,r5,r6}		// cp15: r5,r6,r13
	mcr	p15, 0, r2, cr5, cr0, 0
	mcr	p15, 0, r3, cr5, cr0, 1
	mcr	p15, 0, r4, cr6, cr0, 0
	mcr	p15, 0, r5, cr6, cr0, 2
	mcr	p15, 0, r6, cr13, cr0, 1

	ldmdb	r7!, {r2,r3,r4,r5,r6}		// cp15: r1,r2,r3
//	mcr	p15, 0, r2, cr1, cr0, 0		// already restored inside entMonitor
//RO	mcr	p15, 0, r3, cr2, cr0, 0		// no need to restore (Read Only)
//RO	mcr	p15, 0, r4, cr2, cr0, 1
//RO	mcr	p15, 0, r5, cr2, cr0, 2
	mcr	p15, 0, r6, cr3, cr0, 0

	ldmdb	r7!, {r3, sp, lr}	// svc: spsr, sp, lr
	msr	spsr_fsxc, r3
	mov	r1, lr			// r1: lr_svc (used for forcible terminal of processes)

	cps	#PSR_UND
	ldmdb	r7!, {r3, sp, lr}	// und: spsr, sp, lr
	msr	spsr_fsxc, r3

	cps	#PSR_ABT
	ldmdb	r7!, {r3, sp, lr}	// abt: spsr, sp, lr
	msr	spsr_fsxc, r3

	cps	#PSR_IRQ
	ldmdb	r7!, {r3, sp, lr}	// irq: spsr, sp, lr
	msr	spsr_fsxc, r3

	cps	#PSR_FIQ
	ldmdb	r7!, {r3, r8-r12,sp,lr}	// fiq: spsr, r8 .. r12,sp,lr
	msr	spsr_fsxc, r3

	sub	r7, r7, #(4*7)
	ldmia	r7, {r8-r12,sp,lr}^	// usr: r8 .. r12,sp,lr

        // restore status on etry
	ldmdb	r7!, {r0, r2, r3, r4}	// r0:Entry cpsr, r2:Entry ip
					// r3:cpsr(spsr), r4:pc(lr)
	msr	cpsr_fsxc, r0		// entry cpsr
	stmfd	sp!, {r3}		// spsr -> stack
	stmfd	sp!, {r4}		// pc(lr) -> stack

        // do we have request for forcible termination of processe(es)?
	ldr	r4, =Csym(killProcReq)
	ldr	r0, [r4]
	tst	r0, #0xff
	beq	no_kill			// no request
	mov	r0, #0
	str	r0, [r4]		// clear forcible termination request

        // restore to the state on entry completely, and then push lr_svc
	stmfd	sp!, {r1, ip}		// sp -> lr_svc, ip
	mov	ip, r2			// restore ip
	ldmdb	r7, {r0-r7}		// r0 .. r7

	swi	#SWI_KILLPROC		// forcible termination of processes request
	nop				// do not return
	nop

  no_kill:
	ldmdb	r7, {r0-r7}		// r0 .. r7

        // return from monitor
	rfefd	sp!

/* ------------------------------------------------------------------------ */

/*
 * calling an external program
 * W callExtProg( FP entry )
 */
	.text
	.balign	4
	.globl	Csym(callExtProg)
	.type	Csym(callExtProg), %function
Csym(callExtProg):
	stmfd	sp!, {r4-r10, fp, lr}	// save registers
	blx	r0			// call entry(void)
	ldmfd	sp!, {r4-r10, fp, pc}	// restore registers
