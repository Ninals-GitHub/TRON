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
#include <sys/svc/fnconsio.h>

	.text
	.balign	4
	.globl	Csym(console_conf)
	.type	Csym(console_conf), %function
Csym(console_conf):
	stmfd	sp!, {r0-r3}
	mov	r0, sp
	stmfd	sp!, {lr}
	ldr	ip, =CONSIO_CONSOLE_CONF_FN
	swi	SWI_SVC
	ldmfd	sp!, {lr}
	add	sp, sp, #4*4
	bx	lr

