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
 *	crt1s.S
 *
 *	C startup
 *
 *	Dynamic link version
 *	link in shared lybrary loader
 */

#include <machine.h>

	.data
	.globl	Csym(__initloading)
	.globl	Csym(__callinit)
	.globl	Csym(__callfini)

			.balign	4
Csym(__initloading):	.int	Csym(__so_initloading)
Csym(__callinit):	.int	Csym(__so_init)
Csym(__callfini):	.int	Csym(__so_fini)
