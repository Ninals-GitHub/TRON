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
 *	@(#)asm_depend.h (tk/EM1-D512)
 *
 *	Assembler Macro for EM1-D512
 */

#ifndef __TK_ASM_DEPEND_H__
#define __TK_ASM_DEPEND_H__

#define base(n)		( (n) & 0xfffff000 )
#define offs(n)		( (n) & 0x00000fff )

/*
 * Interrupt flag specified to CPS instruction
 */
#define IMASK		ai

/*
 * Memory barrier instruction
 *	.ISB	Instruction Synchronization Barrier
 *	.DSB	Data Synchronization Barrier
 *	.DMB	Data Memory Barrier
 */
 .macro _mov reg, val
   .ifnes "\reg", "\val"
	mov	\reg, \val
   .endif
 .endm
 .macro .ISB reg, val=#0
	_mov	\reg, \val
	mcr	p15, 0, \reg, cr7, c5, 4
 .endm
 .macro .DSB reg, val=#0
	_mov	\reg, \val
	mcr	p15, 0, \reg, cr7, c10, 4
 .endm
 .macro .DMB reg, val=#0
	_mov	\reg, \val
	mcr	p15, 0, \reg, cr7, c10, 5
 .endm

/* ------------------------------------------------------------------------ */
/*
 *	Processing for returning from an exception
 */

/*
 * Processing for return from an interrupt (IRQ) (excluding FIQ)
 */
 .macro INT_RETURN
	.arm
	ldmfd	sp!, {r3, ip}
	rfefd	sp!
 .endm

/*
 * Processing for return from an exception
 */
 .macro EXC_RETURN
	.arm
	ldmfd	sp!, {ip}
	rfefd	sp!
 .endm

/*
 * Return from Exception/Interrupt (excluding FIQ)
 */
 .macro EIT_RETURN
	.arm
	mrs	ip, cpsr
	and	ip, ip, #PSR_M(31)
	cmp	ip, #PSR_IRQ
	ldmeqfd	sp!, {r3, ip}		// for IRQ
	ldmnefd	sp!, {ip}		// for other cases
	rfefd	sp!
 .endm

/* ------------------------------------------------------------------------ */
/*
 *	tk_ret_int()
 */

/*
 * enter SVC mode
 */
 .macro ENTER_SVC_MODE
	.arm
	cps	#PSR_SVC		// enter SVC mode
 .endm

/*
 * returning from handler using tk_ret_int()
 *	mode	handler exception mode (not usable for FIQ)
 *
 *	called from SVC mode
 *
 *	status of exception mode stack of the handler  when the macro is called
 *		+---------------+
 *	sp  ->	|R12=ip		|
 *		|R14=lr		|
 *		|SPSR		|
 *		+---------------+
 */
 .macro TK_RET_INT mode
	.arm
	mov	ip, lr				// ip = lr_svc
	cpsid	IMASK, #\mode			// return to the original exception
	stmfd	sp!, {ip}			// save lr_svc
	svc	SWI_RETINT
 .endm

/*
 * returning from handler using tk_ret_int()
 *	mode	handler exception mode (not usable for FIQ)
 *
 *	called from SVC
 *
 *	status of exception mode stack of the handler  when the macro is called
 *		+---------------+
 *	sp  ->	|R3		|
 *		|R12=ip		|
 *		|R14=lr		|
 *		|SPSR		|
 *		+---------------+
 */
 .macro TK_RET_INT_FIQ mode
	.arm
	mov	r3, lr				// r3 = lr_svc
	cpsid	IMASK, #\mode			// return to the original exception
	swp	r3, r3, [sp]			// save lr_svc, and restore r3
	svc	SWI_RETINT
 .endm

/* ------------------------------------------------------------------------ */
/*
 *	entry processing for a task exception handler
 *
 *		+---------------+
 *	sp  ->	|texcd		| exception code
 *		|PC		| return address from the handler
 *		|CPSR		| CPSR to be restored on return
 *		+---------------+
 */

 .macro TEXHDR_ENTRY texhdr
	.arm
	swp	lr, lr, [sp]		// save lr , lr = texcd
	stmfd	sp!, {r0-r4, ip}	// save other registers

	mov	r4, sp
	bic	sp, sp, #8-1		// align (module 8 bytes) of sp

	ldr	ip, =\texhdr
	mov	r0, lr
	blx	ip			// call texhdr(texcd)

	mov	sp, r4
	ldmfd	sp!, {r0-r4, ip, lr}	// restore registers
	svc	SWI_RETTEX		// return from task exception handler
 .endm

/* ------------------------------------------------------------------------ */
#endif /* __TK_ASM_DEPEND_H__ */
