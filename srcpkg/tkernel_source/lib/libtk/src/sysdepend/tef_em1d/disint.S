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
 *	@(#)disint.S (libtk/EM1-D512)
 *
 *	Enable/disable external interrupt
 */

#include <machine.h>
#include <tk/sysdef.h>

/*
 * Disable external interrupt
 * UINT disint( void )
 *	CPSR interrupt flag is in disabled status.
 *	Return value is CPSR prior to change.
 */
	.text
	.balign	4
	.globl	Csym(disint)
	.type	Csym(disint), %function
Csym(disint):
	mrs	r0, cpsr
	orr	ip, r0, #PSR_DI
	msr	cpsr_xc, ip
	bx	lr


/*
 * Enable external interrupt (restore to original state)
 * UINT enaint( UINT intsts )
 *	Updates CPSR interrupt disabled flag with the intsts interrupt
 *	disabled flag.
 *	CPSR remains unchanged apart from the interrupt disabled flag.
 *	The return value is CPSR prior to change.
 */
	.text
	.balign	4
	.globl	Csym(enaint)
	.type	Csym(enaint), %function
Csym(enaint):
	and	r3, r0, #PSR_DI
	mrs	r0, cpsr
	bic	r2, r0, #PSR_DI
	orr	r2, r2, r3
	msr	cpsr_xc, r2
	bx	lr
