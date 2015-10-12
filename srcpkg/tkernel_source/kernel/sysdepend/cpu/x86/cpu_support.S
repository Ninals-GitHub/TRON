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
#include <cpu/x86/cpu_conf.h>

#include <tk/config.h>
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
	ret

Csym(dispatch_entry):
_ret_int_dispatch:
    l_dispatch0:
#if USE_DBGSPT

  ret_hook_stop:
#endif
  l_dispatch1:

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
	ret

Csym(exchdr_startup):
	ret

Csym(inthdr_startup):
	
  l_inthdr1:

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
 * Setting and releasing hook routine of an interrupt handler
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
	

  l_retint_svc:

  l_retint1:

  l_nodispatch:
	ret

/* ------------------------------------------------------------------------ */

/*
 * Unsupported system call
 */
	.text
	.balign	4
	.globl	Csym(no_support)
	.type	Csym(no_support), %function
Csym(no_support):
	ret

/*
----------------------------------------------------------------------------------
	T-Kernel standard svc
----------------------------------------------------------------------------------
*/
	.text
	.balign	4
	.globl _svctbl
_svctbl:
	.long	no_support
#define	_tk_ret_int	no_support
#include <sys/svc/tksvctbl.h>
#undef	_tk_ret_int

/*
----------------------------------------------------------------------------------
	T-Kernel/DS svc
----------------------------------------------------------------------------------
*/
#if USE_DBGSPT
	.text
	.balign	4
	.globl _tdsvctbl
_tdsvctbl:
	.long	Csym(no_support)
#include <sys/svc/tdsvctbl.h>
#endif /* USE_DBGSPT */
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

#if USE_DBGSPT

  ret_hook_enter:
#endif

  l_nochklevel:

  l_param_copy:

  l_nocopy:

  l_retsvc:
#if USE_DBGSPT
  ret_hook_leave:
#endif


  l_nodct:


  l_esvc_function:

  l_illegal_svc:

  l_oacv_err:
	ret

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

  he_param_copy:
  he_param4:
  he_param2:

hook_leave:
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

			.data
			.balign	4
  hook_enter_jmp:	.long	ret_hook_enter
  hook_leave_jmp:	.long	ret_hook_leave

#endif /* USE_DBGSPT */



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
 *	if an illegal call is made, call default hander.
 */
	.text
	.balign	4
	.globl	Csym(rettex_entry)
	.type	Csym(rettex_entry), %function
Csym(rettex_entry):


  l_illegal_rettex:
	ret
/* ------------------------------------------------------------------------ */
