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
 *	@(#)asmstartup.S (crttk/EM1-D512)
 *
 *	Driver/manager startup
 */

#include <machine.h>
#include <tk/asm.h>

	.text
	.balign	4
	.globl	Csym(_C_startup)
	.type	Csym(_C_startup), %function
Csym(_C_startup):

  copy_data:
  nocopy_data:

  clrbss:
  nobss:

  l_noinit:
	jmp	Csym(_P_startup)
