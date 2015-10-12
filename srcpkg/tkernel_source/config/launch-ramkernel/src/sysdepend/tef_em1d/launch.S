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
 *	launch.S
 *
 *	Start RAM-kernel by BD command of T-Monitor.
 *	Locate this program at ROM-kernel start address,
 *	then jump to the RAM-kernel.
 */

#include <machine.h>
#include <tk/asm.h>

	.text
	.balign	4
	.globl	START
	.type	START, %function
START:
	ldr	pc, =0x30006074		// RAM-kernel address
