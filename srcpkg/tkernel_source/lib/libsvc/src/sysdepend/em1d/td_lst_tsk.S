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
 *	T-Kernel SVC interface library (em1d)
 *
 */

#include <machine.h>
#include <tk/sysdef.h>
#include <sys/svc/tdfncd.h>

	.text
	.balign	4
	.globl	Csym(td_lst_tsk)
	.type	Csym(td_lst_tsk), %function
Csym(td_lst_tsk):
	stmfd	sp!, {lr}
	ldr	ip, =TDFN_LST_TSK
	swi	SWI_DEBUG
	ldmfd	sp!, {lr}
	bx	lr

