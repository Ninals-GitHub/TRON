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
 *	crti.S
 *
 *	C startup
 */

.macro FUNC_START
	.arm
	mov	ip, sp			// r3 is used to align stack
					// module 8 bytes
	stmdb	sp!, {r3-r9, sl, fp, ip, lr, pc}
	sub	fp, ip, #4
.endm

	.section	.init
	.balign		4
	.global		_init
_init:
	FUNC_START
	ldr	sl, I_GOT
.L4:
	add	sl, pc, sl
	b	I_TOP
I_GOT:	.word	_GLOBAL_OFFSET_TABLE_-(.L4+8)
I_TOP:

	.section	.fini
	.balign		4
	.global		_fini
_fini:
	FUNC_START
	ldr	sl, F_GOT
.L5:
	add	sl, pc ,sl
	b	F_TOP
F_GOT:	.word	_GLOBAL_OFFSET_TABLE_-(.L5+8)
F_TOP:
