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
 *	cpu_support.S
 *
 *	CPU operation specific to EM1-D512, etc.
 */
#define	_in_asm_source_

#include <machine.h>
#include <tk/errno.h>
#include <tk/sysdef.h>
#include <tk/asm.h>
#include <sys/sysinfo.h>

#include <tk/config.h>
#include <tk/cpu_conf.h>
#include <tk/isysconf.h>
#include <tk/tkdev_conf.h>
#include "offset.h"

/* ------------------------------------------------------------------------ */
/*
 * Dispatcher
 *	dispatch_to_schedtsk:
 *		Discard current context and, dispatch to schedtsk forcibly.
 *		Jumping to it directly using (bx) and never returns.
 *		Called when stack is not defined (ssp is undefined).
 *		Called when interrupt is disabled.
 *	dispatch_entry:
 *		Ordinary dispatch processing. Called by svc,  SWI_DISPATCH.
 *	_ret_int_dispatch:
 *		Called if tk_ret_int() requires dispatching.
 *
 *	Saved context
 *	Save registers except for ssp(R13_svc) to stack. ssp is saved in TCB.
 *
 *		+---------------+
 *	ssp ->	| R0--R11	|
 *		| taskmode	|
 *		| R13_usr = usp	| valid only in RNG1-3
 *		| R14_usr	|
 *		+---------------+
 *		| R14_svc	| R14_svc before interrupt
 *		|		| (valid only in case of tk_ret_int)
 *		+---------------+
 *		| R12     = ip	|			^ saved by interrupt entry
 *		| R14_svc = lr	| return address(pc)	| routine
 *		| SPSR_svc	|			V
 *		+---------------+
 */

	/* temporary stack used when dispatch_to_schedtsk is called
	   */
#define	TMP_STACK_SZ	(4*1024)
#define	TMP_STACK_TOP	(tmp_stack + TMP_STACK_SZ)
	.lcomm	tmp_stack, TMP_STACK_SZ

	.text
	.balign	4
	.globl	Csym(dispatch_to_schedtsk)
	.type	Csym(dispatch_to_schedtsk), %function
	.globl	Csym(dispatch_entry)
	.type	Csym(dispatch_entry), %function

Csym(dispatch_to_schedtsk):
	/* SVC mode / interrupt-disabled state CPSR.I=1 A=1 */
	ldr	sp, =TMP_STACK_TOP		// set up a temporary stack

	ldr	ip, =Csym(dispatch_disabled)
	ldr	r0, =1
	str	r0, [ip]			// disable dispatch

	ldr	r4, =Csym(ctxtsk)		// R4 = &ctxtsk
	ldr	ip, =TASKMODE
	ldr	r0, =0
#if USE_DBGSPT
	ldr	r8, [r4]
#endif
	str	r0, [r4]			// ctxtsk = NULL
	str	r0, [ip]			// taskmode = 0

	cpsie	aif				// enable interrupt
	b	l_dispatch0

Csym(dispatch_entry):
	/* SVC mode / interrupt-disabled state CPSR.I=1 A=1 */
	ldr	ip, [sp, #2*4]
	bic	ip, ip, #PSR_DI
	str	ip, [sp, #2*4]			// adjust SPSR_svc

	stmfd	sp!, {lr}			// save context (R14_svc)
						// meaningless place holder for proper stack alignment
_ret_int_dispatch:
	/* SVC mode / interrupt-disabled state  CPSR.I=1 A=1 */
	ldr	ip, =Csym(dispatch_disabled)
	ldr	lr, =1
	str	lr, [ip]			// disable dispatch

	cpsie	aif				// enable interrupt

	ldr	ip, =TASKMODE
	ldr	ip, [ip]
	sub	sp, sp, #15*4
	stmia	sp, {r0-r11, ip, sp, lr}^	// save context

	ldr	r4, =Csym(ctxtsk)		// R4 = &ctxtsk
	ldr	ip, =TASKMODE
	ldr	r0, =0
	ldr	r8, [r4]
	str	sp, [r8, #TCB_tskctxb + CTXB_ssp] // save ssp to TCB
	str	r0, [r4]			// ctxtsk = NULL
	str	r0, [ip]			// taskmode = 0

  l_dispatch0:
	bic	sp, sp, #7			// align stack module 8 bytes

	/* interrupt-enabled state   CPSR.I=0 A=0 */
#if USE_DBGSPT
	ldr	ip, =hook_stop_jmp		// hook processing
	ldr	pc, [ip]
  ret_hook_stop:
#endif

	ldr	r5, =Csym(schedtsk)		// R5 = &schedtsk
	ldr	r6, =Csym(lowpow_discnt)	// R6 = &lowpow_discnt

  l_dispatch1:
	cpsid	IMASK				// disable interrupt

	ldr	r8, [r5]			// R8 = schedtsk
	cmp	r8, #0				// is there schedtsk ?
	bne	l_dispatch2

	/* enter low-power mode since there is no task to execute */
	ldr	ip, [r6]			// is low_pow disabled?
	cmp	ip, #0
	bleq	Csym(low_pow)			// call low_pow()

	cpsie	aif				// enable interrupt
	b	l_dispatch1

  l_dispatch2:					// dispatch to schedtsk
	/* interrupt-disabled state CPSR.I=1 A=1 */
	str	r8, [r4]			// ctxtsk = schedtsk
	ldr	sp, [r8, #TCB_tskctxb + CTXB_ssp] // restore ssp from TCB

	/* switch task spaces */
	ldr	r0, =0
	ldr	r1, [r8, #TCB_tskctxb + CTXB_uatb]
	ldr	r2, [r8, #TCB_tskctxb + CTXB_lsid]
	mrc	p15, 0, ip, cr2, c0, 1		// TTBR1
	cmp	r1, #0
	andne	ip, ip, #0x7f
	orrne	ip, ip, r1
	mcr	p15, 0, r0, cr13, c0, 1		// set ASID to a meaningless value
	.ISB	r0, r0				// for synchronizing ASID and TTBR
	mcr	p15, 0, ip, cr2,  c0, 0		// TTBR0
	mcr	p15, 0, r2, cr13, c0, 1		// CONTEXTIDR
	.ISB	r0, r0

#if USE_DBGSPT
	ldr	ip, =hook_exec_jmp		// hook processing
	ldr	pc, [ip]
  ret_hook_exec:
#endif

	ldr	ip, =Csym(dispatch_disabled)
	ldr	lr, =0
	str	lr, [ip]			// enable dispatch

	ldr	ip, [r8, #TCB_reqdct]		// request DCT
	cmp	ip, #1

	ldmia	sp, {r0-r11, ip, sp, lr}^	// restore context
	nop
	add	sp, sp, #15*4
	ldr	lr, =TASKMODE
	str	ip, [lr]
	movne	ip, #0				// ip = 0 when no DCT request

	ldmfd	sp!, {lr}			// restore R14_svc

	ands	ip, ip, #TMF_CPL(3)		// DCT is ineffective in protection level 0
	bne	dct_startup			// jump to DCT processing

	EXC_RETURN


#if USE_DBGSPT
/*
 * Hook routine invocation in task dispatcher
 *	void stop( ID tskid, INT lsid, UINT tskstat )
 *	void exec( ID tskid, INT lsid )
 */
	.text
	.balign	4
hook_stop:
	cmp	r8, #0			// r8 = ctxtsk
	beq	l_notask

	ldrb	r2, [r8, #TCB_state]			// tskstat
	mov	r2, r2, lsl #1
	ldr	r1, [r8, #TCB_tskctxb + CTXB_lsid]	// lsid
	ldr	r0, [r8, #TCB_tskid]			// tskid

	ldr	ip, =Csym(hook_stopfn)
	ldr	ip, [ip]
	blx	ip			// call stop(tskid, lsid, tskstat)

  l_notask:
	b	ret_hook_stop

hook_exec:
	mov	r9, sp			// save sp
	bic	sp, sp, #7		// align stack module 8 bytes

					// r8 = ctxtsk
	ldr	r1, [r8, #TCB_tskctxb + CTXB_lsid]	// lsid
	ldr	r0, [r8, #TCB_tskid]			// tskid

	ldr	ip, =Csym(hook_execfn)
	ldr	ip, [ip]
	blx	ip			// call exec(tskid, lsid)

	mov	sp, r9			// restore sp
	b	ret_hook_exec

/*
 * Setting and releasing of task dispatcher hook routine
 */
	.text
	.balign	4
	.globl	Csym(hook_dsp)
	.type	Csym(hook_dsp), %function
Csym(hook_dsp):
	ldr	r0, =hook_exec_jmp
	ldr	r1, =hook_stop_jmp
	ldr	r2, =hook_exec
	ldr	r3, =hook_stop
	str	r2, [r0]
	str	r3, [r1]
	bx	lr

	.globl	Csym(unhook_dsp)
	.type	Csym(unhook_dsp), %function
Csym(unhook_dsp):
	ldr	r0, =hook_exec_jmp
	ldr	r1, =hook_stop_jmp
	ldr	r2, =ret_hook_exec
	ldr	r3, =ret_hook_stop
	str	r2, [r0]
	str	r3, [r1]
	bx	lr

			.data
			.balign	4
  hook_exec_jmp:	.long	ret_hook_exec
  hook_stop_jmp:	.long	ret_hook_stop

#endif /* USE_DBGSPT */

/* ------------------------------------------------------------------------ */
/*
 * High-level language support routine for interrupt handler
 *	this is called from interrupt entry routine with information
 * 	in the stack as follows.
 *		+---------------+	SVC stack
 *	ssp ->	| R13_svc = ssp	|
 *		| R12_usr = ip	|
 *		| (padding)	| stack alignment (if necessary)
 *		| CPSR_xxx	|
 *		| R14_svc = lr	|
 *		+---------------+
 *
 *		+---------------+	exception stack
 *		| R0 - R2	|
 *		+===============+
 *	isp ->	| R3		| only in the case of FIQ and IRQ
 *		+---------------+
 *	isp ->	| R12 = ip	| R12_usr or R12_fiq
 *		| R14_xxx = lr	|
 *		| SPSR		|
 *		+---------------+
 *
 *	ip holds the vector table address
 *	(ip - EIT_VECTBL) / 4 = vector number
 */
	.text
	.balign	4
	.globl	Csym(defaulthdr_startup)
	.type	Csym(defaulthdr_startup), %function
	.globl	Csym(inthdr_startup)
	.type	Csym(inthdr_startup), %function
	.globl	Csym(exchdr_startup)
	.type	Csym(exchdr_startup), %function
Csym(defaulthdr_startup):
	/* unknown mode / interrupt-disabled state CPSR.I=1 A=? F=? */
	mrs	lr, cpsr
	and	lr, lr, #PSR_M(31)
	cmp	lr, #PSR_FIQ
	cmpne	lr, #PSR_IRQ
	stmnefd	sp!, {r3}		// save registers
	stmfd	sp!, {r0-r2}

	ldr	r3, =EIT_VECTBL
	sub	r3, ip, r3
					// argument to the handler
	mov	r0, r3, lsr #2		// r0 = dintno
	add	r1, sp, #4*4		// r1 = sp

	ldr	r3, =EIT_DEFAULT * 4	// r3 = vector table offset
	b	l_inthdr2

Csym(exchdr_startup):
	/* unknown mode / interrupt-disabled state CPSR.I=1 A=? F=? */
	stmfd	sp!, {r3}		// save registers
	b	l_inthdr1

Csym(inthdr_startup):
	/* unknown mode / interrupt-disabled state CPSR.I=1 A=1 F=? */
	mrs	lr, cpsr
	and	lr, lr, #PSR_M(31)
	cmp	lr, #PSR_SVC
	stmeqfd	sp!, {r3}		// save r3 in the case of SVC
  l_inthdr1:
	stmfd	sp!, {r0-r2}		// save registers

	ldr	r3, =EIT_VECTBL
	sub	r3, ip, r3		// r3 = vector table offset

					// argument to the handler
	mov	r0, r3, lsr #2		// r0 = dintno
	add	r1, sp, #4*4		// r1 = sp

  l_inthdr2:
	mrs	r2, cpsr		// save CPSR to r2
	cpsid	IMASK, #PSR_SVC		// enter SVC mode, interrupt is disabled

	stmfd	sp!, {r2, lr}		// save CPSR and lr_svc

	mov	r2, sp
	bic	sp, sp, #7		// align stack module 8 bytes
	stmfd	sp!, {r2, ip}		// save sp and ip

	ldr	ip, =TASKINDP		// enter task independent portion
	ldr	lr, [ip]
	add	lr, lr, #1
	str	lr, [ip]

#if USE_DBGSPT
	ldr	ip, =hook_ienter_jmp
	ldr	pc, [ip]
  ret_hook_ienter:
#endif

	ldr	ip, =Csym(hll_inthdr)
	ldr	ip, [ip, r3]
	blx	ip		// call hll_inthdr[n](dintno, sp)

#if USE_DBGSPT
	ldr	ip, =hook_ileave_jmp
	ldr	pc, [ip]
  ret_hook_ileave:
#endif

	ldmfd	sp!, {r2, ip}		// restore sp and ip
	mov	sp, r2
	ldmfd	sp!, {r2, r3}		// r2 contains the original mode
					// r3 = R14_svc
	orr	r2, r2, #PSR_DI
	msr	cpsr_xc, r2		// restore original mode, interrupt is disabled

	ldr	ip, =TASKINDP
	ldr	lr, [ip]
	sub	lr, lr, #1
	str	lr, [ip]

	ldmfd	sp!, {r0-r2}		// restore registers
	swp	r3, r3, [sp]		// restore r3, R14_svc is saved
	swi	SWI_RETINT		// tk_ret_int()


#if USE_DBGSPT
/*
 * Invoking a hook routine of an interrupt handler
 */
	.text
	.balign	4
hook_ienter:
	stmfd	sp!, {r0-r1}		// save registers
	stmfd	sp!, {r2-r3}		// align stack module 8

	ldr	ip, =Csym(hook_ienterfn)
	ldr	ip, [ip]
	blx	ip			// call enter(dintno, sp)

	ldmfd	sp!, {r2-r3}		// restore registers
	ldmfd	sp,  {r0-r1}		// leave dintno and sp on the stack
	b	ret_hook_ienter

hook_ileave:
	ldmfd	sp!, {r0-r1}		// restore dintno and sp

	ldr	ip, =Csym(hook_ileavefn)
	ldr	ip, [ip]
	blx	ip			// call leave(dintno, sp)

	b	ret_hook_ileave

/*
 * Setting and releasing hook routine of an interrupt handler
 */
	.text
	.balign	4
	.globl	Csym(hook_int)
	.type	Csym(hook_int), %function
Csym(hook_int):
	ldr	r0, =hook_ienter_jmp
	ldr	r1, =hook_ileave_jmp
	ldr	r2, =hook_ienter
	ldr	r3, =hook_ileave
	str	r2, [r0]
	str	r3, [r1]
	bx	lr

	.globl	Csym(unhook_int)
	.type	Csym(unhook_int), %function
Csym(unhook_int):
	ldr	r0, =hook_ienter_jmp
	ldr	r1, =hook_ileave_jmp
	ldr	r2, =ret_hook_ienter
	ldr	r3, =ret_hook_ileave
	str	r2, [r0]
	str	r3, [r1]
	bx	lr

			.data
			.balign	4
  hook_ienter_jmp:	.long	ret_hook_ienter
  hook_ileave_jmp:	.long	ret_hook_ileave

#endif /* USE_DBGSPT */

/*
 * Processing tk_ret_int()
 *	interrupt stack looks as follows when this is called.
 *		+---------------+
 *	ssp ->	| R12_usr	| saved by svc SWI_RETINT
 *		| R14_svc	|
 *		| SPSR_svc	|
 *		+---------------+
 *
 *		+---------------+
 *	isp ->	| R14_svc	| saved when tk_ret_int was called
 *		+---------------+
 *		| R12_xxx	| saved at the time of interrupt
 *		| R14_xxx	| <- return address
 *		| SPSR_xxx	|
 *		+---------------+
 */
	.text
	.balign	4
	.globl	Csym(_tk_ret_int)
	.type	Csym(_tk_ret_int), %function
Csym(_tk_ret_int):
	ldr	ip, [sp, #2*4]		// ip = SPSR

	and	lr, ip, #PSR_M(31)
	cmp	lr, #PSR_SVC
	beq	l_retint_svc		// is this tk_ret_int invoked from an SVC?

	stmfd	sp!, {r2, r3}		// r2 is used as temporary work register (r3 is saved as place holder)
	add	r2, sp, #4

	orr	ip, ip, #PSR_DI
	bic	ip, ip, #PSR_T
	cmp	lr, #PSR_FIQ
	msr	cpsr_xc, ip		// enter interrupted mode, interrupt is disabled

	ldr	ip, [sp, #0*4]		// copy isp to ssp
	ldr	lr, [sp, #3*4]
	str	ip, [r2, #0*4]		// R14_svc
	str	lr, [r2, #3*4]		// SPSR_xxx
	ldr	ip, [sp, #1*4]
	ldr	lr, [sp, #2*4]
	strne	ip, [r2, #1*4]		// R12_xxx (in non-FIQ cases only)
	str	lr, [r2, #2*4]		// R14_xxx (return address)
	add	sp, sp, #4*4		// pop isp

	cpsid	IMASK, #PSR_SVC		// enter SVC mode, interrupt is disabled

	ldmfd	sp!, {r2}		// restore r2
	b	l_retint1

  l_retint_svc:
	add	sp, sp, #3*4		// discard the value saved by svc SWI_RETINT

	cpsid	IMASK, #PSR_SVC		// disable interrupt

  l_retint1:
	ldr	ip, =TASKINDP		// is there additional  multi-interrupt(s)?
	ldr	ip, [ip]
	cmp	ip, #0
	bne	l_nodispatch

	ldr	ip, =Csym(dispatch_disabled)	// is dispatch disabled?
	ldr	ip, [ip]
	cmp	ip, #0
	bne	l_nodispatch

	ldr	ip, [sp, #3*4]		// SPSR
	tst	ip, #PSR_A|PSR_I|PSR_F	// is this exception during interrupt-disabled state?
	bne	l_nodispatch

	ldr	ip, =Csym(ctxtsk)	// is dispatch necessary?
	ldr	lr, =Csym(schedtsk)
	ldr	ip, [ip]
	ldr	lr, [lr]
	cmp	ip, lr
	bne	_ret_int_dispatch	// goto dispatch processing

  l_nodispatch:
	ldmfd	sp!, {lr}		// restore lr
	EXC_RETURN


/* ------------------------------------------------------------------------ */

/*
 * Unsupported system call
 */
	.text
	.balign	4
	.globl	Csym(no_support)
	.type	Csym(no_support), %function
Csym(no_support):
	ldr	r0, =E_RSFN
	bx	lr

/*
 * System call entry table
 */
	.text
	.balign	4
_svctbl:
	.int	Csym(no_support)
#define	_tk_ret_int	no_support
#include <sys/svc/tksvctbl.h>
#undef	_tk_ret_int

/*
 * System call entry
 *	no need to save temporary registers
 *	permanent registers are saved by the code generated by the compiler
 *		+---------------+
 *		| exinf		| save for hook
 *		+---------------+
 *		| svc_ssp	| save within call_entry
 *		| taskmode	|
 *		| r9		|
 *		| r10		|
 *		| fp		|
 *		+===============+
 *	ssp ->	| ip		| function code
 *		| lr		| return address
 *		| SPSR		|
 *		+---------------+
 *
 *	Save ssp to CTXB.svc_ssp when a T-Kernel system call (SVC) is invoked.
 *	However, if this is a nested SVC, svc_ssp is not updated.
 *	Save the original svc_ssp to the system stack.
 *	svc_ssp is restored when the system call returns.
 *	svc_ssp = NULL means that SVC is not being callled.
 *	With extended SVC, svc_ssp = NULL holds.
 */
	.text
	.balign	4
	.globl	Csym(call_entry)
	.type	Csym(call_entry), %function
Csym(call_entry):
	/* SVC mode / interrupt-disabled state CPSR.I=1 A=? F=? */
	ldr	ip, [sp, #2*4]
	and	ip, ip, #PSR_I|PSR_F	// cpsr_c doesn't include PSR_A
	orr	ip, ip, #PSR_SVC
	msr	cpsr_c, ip		// restore interrupt disabled status to the original.

	stmfd	sp!, {r9-r10, fp}	// save working registers
	add	fp, sp, #3*4		// fp is the sp when call_entry was entered.

	ldr	ip, =TASKMODE		// obtain taskmode flag
	ldr	r10, [ip]		// r10 = taskmode
	stmfd	sp!, {r10}		// save taskmode
	mov	lr, r10, lsl #16	// update taskmode flag
	str	lr, [ip]

	ldr	r9, =Csym(ctxtsk)	// r9 = ctxtsk
	ldr	r9, [r9]		//   if no ctxtsk (NULL) then
	cmp	r9, #0			//   assign a dummy value
	ldreq	r9, =dummy_svc_ssp - (TCB_tskctxb + CTXB_svc_ssp)
	ldr	ip, [r9, #TCB_tskctxb + CTXB_svc_ssp]
	cmp	ip, #0
	streq	fp, [r9, #TCB_tskctxb + CTXB_svc_ssp]	// update svc_ssp
	stmfd	sp!, {ip}				// save   svc_ssp

#if USE_DBGSPT
	mov	ip, #0
	stmfd	sp!, {ip}		// exinf = 0
	ldr	ip, =hook_enter_jmp	// hook processing
	ldr	pc, [ip]
  ret_hook_enter:
#endif

	ldr	lr, [fp, #0*4]		// lr = function code
	cmp	lr, #0			//	<  0: system call
	bge	l_esvc_function		//	>= 0: extended system call
 
	/* T-Kernel system call */
	ldr	ip, =TASKINDP		// if called from task-independent portion
	ldr	ip, [ip]		// protection-level check is unnecessary
	cmp	ip, #0
	bhi	l_nochklevel

	ldr	ip, =Csym(svc_call_limit)
	ldr	ip, [ip]		// limit value of the ring for call protection
	and	r10, r10, #TMF_CPL(3)	// r10 is taskmode upon entry
	cmp	r10, ip
	bhi	l_oacv_err
  l_nochklevel:

	mov	r10, lr, asr #16	// r10 = function number
	ldr	ip, =N_TFN + 0xffff8000
	cmp	r10, ip
	bgt	l_illegal_svc

	mov	lr, lr, lsr #8
	and	lr, lr, #0xff		// lr = number of arguments
	subs	lr, lr, #4
	bicle	sp, sp, #7		// align stack module 8 bytes
	ble	l_nocopy
	sub	sp, sp, lr, lsl #2
	bic	sp, sp, #7		// align stack module 8 bytes
  l_param_copy:
	subs	lr, lr, #1
	ldr	ip, [r4, lr, lsl #2]	// copy the arguments (from the fifth and later)
	str	ip, [sp, lr, lsl #2]
	bgt	l_param_copy
  l_nocopy:

	ldr	ip, =_svctbl - (0xffff8000 << 2)
	mov	lr, pc
	ldr	pc, [ip, r10, lsl #2]	// T-Kernel system call

  l_retsvc:
#if USE_DBGSPT
	ldr	ip, =hook_leave_jmp	// hook processing
	ldr	pc, [ip]
  ret_hook_leave:
#endif

	sub	sp, fp, #5*4
	ldmfd	sp!, {r2-r3}		// svc_ssp, taskmode

	str	r2, [r9, #TCB_tskctxb + CTXB_svc_ssp]	// restore svc_ssp
	mov	r2, r9			// r2 = ctxtsk

	ldr	ip, =TASKMODE		// restore taskmode
	str	r3, [ip]

	ldmfd	sp!, {r9-r10, fp}	// restore working registers

	ands	r3, r3, #TMF_CPL(3)	// DCT is not processed if called from
	beq	l_nodct			// protection level 0

	cpsid	IMASK			// disable interrupt

	ldr	ip, =TASKINDP		// DCT is not processed if called from
	ldr	ip, [ip]		// task independent portion
	cmp	ip, #0
	bne	l_nodct

	ldr	ip, [r2, #TCB_reqdct]	// is there DCT request?
	cmp	ip, #1
	bne	l_nodct

	ldr	ip, =Csym(dispatch_disabled)
	ldr	ip, [ip]		// if dispatch is disabled,
	cmp	ip, #0			// DCT is not processed
	bne	l_nodct

	b	dct_startup		// goto DCT processing

  l_nodct:
	EXC_RETURN


  l_esvc_function:
	/* extended SVC */
	mov	ip, #0
	str	ip, [r9, #TCB_tskctxb + CTXB_svc_ssp]	// svc_ssp = NULL

	bic	sp, sp, #7		// align stack module 8 bytes
	mov	r1, lr			// r1 = function code
	bl	Csym(svc_ientry)	// svc_ientry(pk_para, fncd)

	b	l_retsvc


  l_illegal_svc:
	ldr	r0, =E_RSFN
	b	l_retsvc

  l_oacv_err:
	ldr	r0, =E_OACV
	b	l_retsvc

			.data
			.balign 4
  dummy_svc_ssp:	.long	-1


#if USE_DBGSPT
/*
 * Invoking hook routine for system call, and extended SVC
 *	void* enter( FN fncd, TD_CALINF *calinf, ... )
 *	void leave( FN fncd, INT ret, void *exinf )
 *
 *	typedef struct td_calinf {
 *		void	*ssp;	system stack pointer
 *		void	*r11;	frame pointer when it is called
 *	} TD_CALINF;
 */
	.text
	.balign	4
hook_enter:
	stmfd	sp!, {r0-r3, r8}	// save argument and work registers
	mov	r8, sp			// r8 = keep the stack position

	ldr	ip, [fp, #-1*4]		// frame pointer when it is called
	stmfd	sp!, {fp, ip}		// create TD_CALINF

	bic	sp, sp, #7		// align stack module 8 bytes

	ldr	lr, [fp, #0*4]		// lr = function code
	cmp	lr, #0			//	<  0: system call
	bge	he_param2		//	>= 0: extended system call

	mov	lr, lr, lsr #8
	and	lr, lr, #0xff		// number of arguments
	subs	lr, lr, #2
	ble	he_param2
	subs	lr, lr, #2
	ble	he_param4
	sub	sp, sp, lr, lsl #2
	bic	sp, sp, #7		// align stack module 8 bytes
  he_param_copy:
	subs	lr, lr, #1
	ldr	ip, [r4, lr, lsl #2]	// Fifth argument (and later)
	str	ip, [sp, lr, lsl #2]
	bgt	he_param_copy
  he_param4:
	str	r3, [sp, #-1*4]!	// forth argument
	str	r2, [sp, #-1*4]!	// third argument
  he_param2:
	mov	r3, r1			// second argument
	mov	r2, r0			// first argument
	sub	r1, r8, #2*4		// calinf
	ldr	r0, [fp, #0*4]		// fncd
	ldr	ip, =Csym(hook_enterfn)
	ldr	ip, [ip]
	blx	ip			// exinf = enter(fncd, ...)
	str	r0, [fp, #-6*4]		// save exinf

	mov	sp, r8			// restore stack position
	ldmfd	sp!, {r0-r3, r8}	// restore argument and work registers
	b	ret_hook_enter

hook_leave:
	sub	sp, fp, #6*4		// sp = saved position of exinf
	swp	r2, r0, [sp]		// save ret and , restore exinf
	mov	r1, r0			// ret
	ldr	r0, [fp, #0*4]		// fncd

	bic	sp, sp, #7		// align stack module 8 bytes
	ldr	ip, =Csym(hook_leavefn)
	ldr	ip, [ip]
	blx	ip			// call leave(fncd, ret, exinf)

	ldr	r0, [fp, #-6*4]		// restore ret
	b	ret_hook_leave

/*
 * Setting and releasing of hook routine for system call and extended SVC
 */
	.text
	.balign	4
	.globl	Csym(hook_svc)
	.type	Csym(hook_svc), %function
Csym(hook_svc):
	ldr	r0, =hook_enter_jmp
	ldr	r1, =hook_leave_jmp
	ldr	r2, =hook_enter
	ldr	r3, =hook_leave
	str	r2, [r0]
	str	r3, [r1]
	bx	lr

	.globl	Csym(unhook_svc)
	.type	Csym(unhook_svc), %function
Csym(unhook_svc):
	ldr	r0, =hook_enter_jmp
	ldr	r1, =hook_leave_jmp
	ldr	r2, =ret_hook_enter
	ldr	r3, =ret_hook_leave
	str	r2, [r0]
	str	r3, [r1]
	bx	lr

			.data
			.balign	4
  hook_enter_jmp:	.long	ret_hook_enter
  hook_leave_jmp:	.long	ret_hook_leave

#endif /* USE_DBGSPT */

/* ------------------------------------------------------------------------ */

#if USE_DBGSPT
/*
 * Call entry table for debugger support functions
 */
	.text
	.balign	4
_tdsvctbl:
	.int	Csym(no_support)
#include <sys/svc/tdsvctbl.h>

/*
 * Entry routine for debugger support functions
 *		+---------------+
 *	ssp ->	| ip		| function code
 *		| lr		| return address
 *		| SPSR		|
 *		+---------------+
 */
	.text
	.balign	4
	.globl	Csym(call_dbgspt)
	.type	Csym(call_dbgspt), %function
Csym(call_dbgspt):
	/* SVC mode / interrupt-disabled CPSR.I=1 A=? F=? */
	ldr	ip, [sp, #2*4]
	and	ip, ip, #PSR_I|PSR_F	// cpsr_c doesn't include PSR_A
	orr	ip, ip, #PSR_SVC
	msr	cpsr_c, ip		// interrupt disabled status is restored to the original state in the caller

	stmfd	sp!, {r10, fp}		// save work registers
	add	fp, sp, #2*4

	ldr	ip, =TASKINDP		// if called from task-independent portion
	ldr	ip, [ip]		// no need to check protection levels.
	cmp	ip, #0
	bhi	b_nochklevel

	ldr	ip, =TASKMODE
	ldr	lr, =Csym(svc_call_limit)
	ldr	ip, [ip]
	ldr	lr, [lr]		// limit value of the ring for call protection
	and	ip, ip, #TMF_CPL(3)	// protection level upon call
	cmp	ip, lr
	bhi	b_oacv_err
  b_nochklevel:

	ldr	lr, [fp, #0*4]		// lr = function code
	mov	r10, lr, asr #16
	ldr	ip, =N_TDFN + 0xffff8000
	cmp	r10, ip
	bgt	b_illegal_svc

	bic	sp, sp, #7		// align stack module 8 bytes

	ldr	ip, =_tdsvctbl - (0xffff8000 << 2)
	mov	lr, pc
	ldr	pc, [ip, r10, lsl #2]	// T-Kernel/DS service call

	sub	sp, fp, #2*4		// restore sp

  b_retsvc:
	cpsid	IMASK			// disable interrupt
	ldmfd	sp!, {r10, fp}		// restore work registers
	EXC_RETURN


  b_illegal_svc:
	ldr	r0, =E_RSFN
	b	b_retsvc

  b_oacv_err:
	ldr	r0, =E_OACV
	b	b_retsvc

#endif /* USE_DBGSPT */

/* ------------------------------------------------------------------------ */
/*
 * High-level language support routine for timer handler
 */
	.text
	.balign	4
	.globl	Csym(timer_handler_startup)
	.type	Csym(timer_handler_startup), %function
Csym(timer_handler_startup):
	/* IRQ mode / interrupt-disabled state CPSR.I=1 A=? */
	cpsid	IMASK, #PSR_SVC		// enter SVC mode, interrupt is disabled

	stmfd	sp!, {r0-r2, r4-r5, fp, lr}	// save registers
	mov	fp, sp
	bic	sp, sp, #7		// align stack module 8 bytes

	ldr	r4, =TASKINDP		// enter task-independent portion
	ldr	r5, [r4]
	add	r0, r5, #1
	str	r0, [r4]

	bl	Csym(timer_handler)	// call timer_handler()
	/* return, interrupt disabled, CPSR.I=1 A=1 */

	str	r5, [r4]		// leave task-independent portion

	mov	sp, fp
	ldmfd	sp!, {r0-r2, r4-r5, fp, lr}	// restore registers

	TK_RET_INT_FIQ PSR_IRQ		// tk_ret_int()

/* ------------------------------------------------------------------------ */
/*
 * delayed context trap(DCT)
 *	invoke task exception handler
 *	the stack looks as follows when this is called.
 *		+---------------+
 *	ssp ->	| R12     = ip	|
 *		| R14_svc = lr	| return address(pc)
 *		| SPSR_svc	|
 *		+---------------+
 */
	.text
	.balign	4
dct_startup:
	/* SVC mode / interrupt-disabled-state CPSR.I=1 A=1 */
	stmfd	sp!, {r0-r3, fp, lr}	// save registers
	add	fp, sp, #6*4
	bic	sp, sp, #7		// align stack module 8 bytes

	ldr	ip, [fp, #2*4]		// spsr
	and	ip, ip, #PSR_M(31)	// processor mode of caller
	cmp	ip, #PSR_USR		// was it user mode?
	cmpne	ip, #PSR_SYS		// If not system mode,
	bne	l_nodct_startup		// DCT is not invoked

	mov	r0, fp
	bl	Csym(setup_texhdr)	// call setup_texhdr(ssp)
	/* return with interrupt-enabled state  */
	cpsid	IMASK			// Inhibit interrupt

  l_nodct_startup:
	sub	sp, fp, #6*4
	ldmfd	sp!, {r0-r3, fp, lr}	// Restore registers
	EXC_RETURN

/*
 * return from task exception handler
 *		+---------------+
 *	usp ->	|PC		| return address from the handler
 *		|CPSR		| CPSR to be restored on return from the handler
 *		+---------------+
 *
 *		+---------------+
 *	ssp ->	| R12     = ip	|
 *		| R14_svc = lr	|  <- copy PC
 *		| SPSR_svc	|  <- copy CPSR
 *		+---------------+
 *
 *	if an illegal call is made, call default hander.
 */
	.text
	.balign	4
	.globl	Csym(rettex_entry)
	.type	Csym(rettex_entry), %function
Csym(rettex_entry):
	/* SVC mode / interrupt-disabled state CPSR.I=1 A=? F=? */
	ldr	ip, [sp, #2*4]		// spsr
	tst	ip, #PSR_A|PSR_I|PSR_F	// call during an interrupt-disabled state is illegal
	bne	l_illegal_rettex
	and	ip, ip, #PSR_M(31)
	cmp	ip, #PSR_USR		// call neither from user mode nor system mode
	cmpne	ip, #PSR_SYS		// is illegal
	bne	l_illegal_rettex

	cpsid	IMASK, #PSR_SYS		// interrupt-disabled state, SYS mode
	mov	ip, sp			// ip = usp

	/* we access user stack, and so we must not be in
	   task-independent portion, and interrupt-disabled state */
	cpsie	IMASK, #PSR_SVC		// enable interrupt, SVC mode

	ldr	lr, [ip, #0*4]		// copy usp to ssp
	str	lr, [sp, #1*4]
	ldr	ip, [ip, #1*4]		// usp->CPSR
	ldr	lr, [sp, #2*4]		// ssp->SPSR
	bic	ip, ip, #PSR_M(31)	// make sure mode is legal
	bic	ip, ip, #PSR_A|PSR_I|PSR_F
	and	lr, lr, #PSR_M(31)
	orr	ip, ip, lr
	str	ip, [sp, #2*4]

	cpsid	IMASK, #PSR_SYS		// interrupt-disabled, SYS mode
	add	sp, sp, #2*4		// pop user stack

	cpsid	IMASK, #PSR_SVC		// intterupt-disabled, SVC mode
	EXC_RETURN

  l_illegal_rettex:
	ldr	ip, =EITVEC(SWI_RETTEX)
	ldr	lr, =base(EITVEC(EIT_DEFAULT))
	ldr	lr, [lr, #offs(EITVEC(EIT_DEFAULT))]
	bx	lr

/* ------------------------------------------------------------------------ */
