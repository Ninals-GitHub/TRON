#! /usr/local/bin/perl
#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2011 by Ken Sakamura.
#     This software is distributed under the T-License 2.0.
# ----------------------------------------------------------------------
#
#     Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#
# ----------------------------------------------------------------------
#

#
#	makeiftk.pl
#
#	generate interface library for EM1-D512
#

sub makelib
{
	print LIB <<EndOfIfLibBody;
#include <machine.h>
#include <tk/sysdef.h>
#include <sys/svc/$fn_h>

	.text
	.balign	4
	.globl	Csym(${func})
	.type	Csym(${func}), %function
Csym(${func}):
	stmfd	sp!, {r4}
	add	r4, sp, #4
	stmfd	sp!, {lr}
	ldr	ip, =TFN_${Func}
	swi	SWI_SVC
	ldmfd	sp!, {lr}
	ldmfd	sp!, {r4}
	bx	lr

EndOfIfLibBody
}

1;
