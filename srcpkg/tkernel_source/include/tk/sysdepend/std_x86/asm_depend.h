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

 .endm
 .macro .ISB reg, val=#0

 .endm
 .macro .DSB reg, val=#0

 .endm
 .macro .DMB reg, val=#0

 .endm

/* ------------------------------------------------------------------------ */
/*
 *	Processing for returning from an exception
 */

/*
 * Processing for return from an interrupt (IRQ) (excluding FIQ)
 */
 .macro INT_RETURN

 .endm

/*
 * Processing for return from an exception
 */
 .macro EXC_RETURN

 .endm

/*
 * Return from Exception/Interrupt (excluding FIQ)
 */
 .macro EIT_RETURN

 .endm

/* ------------------------------------------------------------------------ */
/*
 *	tk_ret_int()
 */

/*
 * enter SVC mode
 */
 .macro ENTER_SVC_MODE

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

 .endm

/* ------------------------------------------------------------------------ */
#endif /* __TK_ASM_DEPEND_H__ */
