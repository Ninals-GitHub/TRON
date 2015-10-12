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
 *	crtn.S
 *
 *	C startup
 */

.macro FUNC_END
	.arm
	ldmdb	fp, {r4-r9, sl, fp, ip, lr}
	mov	sp, ip
	bx	lr
.endm

	.section	.init
	FUNC_END

	.section	.fini
	FUNC_END
