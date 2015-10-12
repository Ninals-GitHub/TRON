/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/07/14.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *    Modified by Nina Petipa at 2015/07/28
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

#if USE_DBGSPT
#endif
	ret
Csym(dispatch_entry):
	/* SVC mode / interrupt-disabled state CPSR.I=1 A=1 */
_ret_int_dispatch:
	/* SVC mode / interrupt-disabled state  CPSR.I=1 A=1 */

  l_dispatch0:

	/* interrupt-enabled state   CPSR.I=0 A=0 */
#if USE_DBGSPT
  ret_hook_stop:
#endif

#	ldr	r5, =Csym(schedtsk)		// R5 = &schedtsk
#	ldr	r6, =Csym(lowpow_discnt)	// R6 = &lowpow_discnt

  l_dispatch1:
	ret
  l_dispatch2:					// dispatch to schedtsk

#if USE_DBGSPT
  ret_hook_exec:
#endif

	ret


#if USE_DBGSPT
/*
 * Hook routine invocation in task dispatcher
 *	void stop( ID tskid, INT lsid, UINT tskstat )
 *	void exec( ID tskid, INT lsid )
 */
	.text
	.balign	4
hook_stop:
	ret

  l_notask:
  	ret

hook_exec:
	ret

/*
 * Setting and releasing of task dispatcher hook routine
 */
	.text
	.balign	4
	.globl	Csym(hook_dsp)
	.type	Csym(hook_dsp), %function
Csym(hook_dsp):
	ret

	.globl	Csym(unhook_dsp)
	.type	Csym(unhook_dsp), %function
Csym(unhook_dsp):
	ret
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
	ret

Csym(exchdr_startup):
	/* unknown mode / interrupt-disabled state CPSR.I=1 A=? F=? */
	ret
Csym(inthdr_startup):
	/* unknown mode / interrupt-disabled state CPSR.I=1 A=1 F=? */
	ret
  l_inthdr1:
	ret

  l_inthdr2:
#if USE_DBGSPT
  ret_hook_ienter:
#endif
	
#if USE_DBGSPT
  ret_hook_ileave:
#endif

	ret


#if USE_DBGSPT
/*
 * Invoking a hook routine of an interrupt handler
 */
	.text
	.balign	4
hook_ienter:
	ret

hook_ileave:
	ret
/*
 * Setting and relaseing hook routine of an interrupt handler
 */
	.text
	.balign	4
	.globl	Csym(hook_int)
	.type	Csym(hook_int), %function
Csym(hook_int):
	ret


	.globl	Csym(unhook_int)
	.type	Csym(unhook_int), %function
Csym(unhook_int):
	ret

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
	ret

  l_retint_svc:

  l_retint1:
	ret

  l_nodispatch:
	ret

/*
 * Setting and releasing of hook routine for system call and extended SVC
 */
	.text
	.balign	4
	.globl	Csym(hook_svc)
	.type	Csym(hook_svc), %function
Csym(hook_svc):
	ret

	.globl	Csym(unhook_svc)
	.type	Csym(unhook_svc), %function
Csym(unhook_svc):
	ret

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

  l_nodct_startup:
	ret

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
 *	if an illegal call is made, call default handler.
 */
	.text
	.balign	4
	.globl	Csym(rettex_entry)
	.type	Csym(rettex_entry), %function
Csym(rettex_entry):
	/* SVC mode / interrupt-disabled state CPSR.I=1 A=? F=? */

	/* we access user stack, and so we must not be in
	   task-independent portion, and interrupt-disabled state */
	ret

  l_illegal_rettex:
	ret

/* ------------------------------------------------------------------------ */
