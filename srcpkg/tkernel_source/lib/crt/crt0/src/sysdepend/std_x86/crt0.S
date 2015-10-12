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
 *	crt0.S
 *
 *	C startup
 *
 *      This is used commonly by process startup, manager startup, and driver startup.
 *      Hence common startup processing is done here, and control is passed to  _C_startup.
 *
 *      individual startup processing is done by individual _C_startup.
 */

#include <machine.h>

#if _Csym == 0
#define	START	_start
#else
#define	START	start
#endif

	.text
	.global	START
	.type	START, function
	.balign	4
START:
	jmp	_C_startup
