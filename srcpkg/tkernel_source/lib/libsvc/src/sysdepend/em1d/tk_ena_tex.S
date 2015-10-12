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
#include <sys/svc/tkfncd.h>

	.text
	.balign	4
	.globl	Csym(tk_ena_tex)
	.type	Csym(tk_ena_tex), %function
Csym(tk_ena_tex):
	stmfd	sp!, {r4}
	add	r4, sp, #4
	stmfd	sp!, {lr}
	ldr	ip, =TFN_ENA_TEX
	swi	SWI_SVC
	ldmfd	sp!, {lr}
	ldmfd	sp!, {r4}
	bx	lr

