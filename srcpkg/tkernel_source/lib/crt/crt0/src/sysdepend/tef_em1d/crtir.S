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
 *	crtir.S
 *
 *	C startup
 *
 *	Relocatable module version
 *	_GLOBAL_OFFSET_TABLE_ is not used
 *	this means it is not Position Independent
 */

.macro FUNC_START
	.arm
	mov	ip, sp			// r3 is used to align stack
					// modulo 8 bytes
	stmdb	sp!, {r3-r9, sl, fp, ip, lr, pc}
	sub	fp, ip, #4
.endm

	.section	.init
	.balign		4
	.global		_init
_init:
	FUNC_START

	.section	.fini
	.balign		4
	.global		_fini
_fini:
	FUNC_START
