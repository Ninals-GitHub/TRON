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
 *	crti.S
 *
 *	C startup
 */

.macro FUNC_START

.endm

	.section	.init
	.balign		4
	.global		_init
_init:
	FUNC_START

.L4:
	jmp	I_TOP
I_GOT:	.word	_GLOBAL_OFFSET_TABLE_-(.L4+8)
I_TOP:

	.section	.fini
	.balign		4
	.global		_fini
_fini:
	FUNC_START
.L5:
	jmp	F_TOP
F_GOT:	.word	_GLOBAL_OFFSET_TABLE_-(.L5+8)
F_TOP:
