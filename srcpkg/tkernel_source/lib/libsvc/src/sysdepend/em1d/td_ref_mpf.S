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
	.globl	Csym(td_ref_mpf)
	.type	Csym(td_ref_mpf), %function
Csym(td_ref_mpf):
	stmfd	sp!, {lr}
	ldr	ip, =TDFN_REF_MPF
	swi	SWI_DEBUG
	ldmfd	sp!, {lr}
	bx	lr

