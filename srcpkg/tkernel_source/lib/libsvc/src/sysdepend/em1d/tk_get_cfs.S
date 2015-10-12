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
 *	T-Kernel extended SVC interface library (em1d)
 *
 */

#include <machine.h>
#include <tk/sysdef.h>
#include <sys/svc/fnsysmgr.h>

	.text
	.balign	4
	.globl	Csym(tk_get_cfs)
	.type	Csym(tk_get_cfs), %function
Csym(tk_get_cfs):
	stmfd	sp!, {r0-r3}
	mov	r0, sp
	stmfd	sp!, {lr}
	ldr	ip, =SYSTEM_TK_GET_CFS_FN
	swi	SWI_SVC
	ldmfd	sp!, {lr}
	add	sp, sp, #4*4
	bx	lr

