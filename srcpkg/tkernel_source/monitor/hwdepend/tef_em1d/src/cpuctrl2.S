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
 *	cpuctrl2.S
 *
 *       ARM CPU control
 *
 *       Assume that system control processor (CP15) exists.
 */
#define	_in_asm_source_

#include <machine.h>
#include <tk/sysdef.h>

#define	DCACHE_NWAY		4	/* N as in the number of N-way data cache */
#define	DCACHE_NWAY_SHIFT	30
#define	DCACHE_NSEG		256	/* number of segments of data cache */
#define	DCACHE_NSEG_SHIFT	5

/*
 * flush the entire cache (write back and then invalidate)
 *	void FlushCache( void )
 */
	.text
	.balign	4
	.globl	Csym(FlushCache)
	.type	Csym(FlushCache), %function
Csym(FlushCache):
	ldr	r2, =DCACHE_NWAY-1
  l_flush_dcache1:
	ldr	r3, =DCACHE_NSEG-1
  l_flush_dcache2:
	mov	ip, r2, lsl #DCACHE_NWAY_SHIFT
	orr	ip, ip, r3, lsl #DCACHE_NSEG_SHIFT
	mcr	p15, 0, ip, cr7, c14, 2		// data cache is written back,
	subs	r3, r3, #1			// and is invalidated
	bpl	l_flush_dcache2
	subs	r2, r2, #1
	bpl	l_flush_dcache1

	ldr	ip, =0
	mcr	p15, 0, ip, cr7, c7, 0		// Invalidate I/D-Cache
	mcr	p15, 0, ip, cr7, c10, 4		// Drain Write Buffer

	bx	lr

/*
 * cache and MMU control
 *	void setCacheMMU( UW cp15r1 )
 */
	.text
	.balign	4
	.globl	Csym(setCacheMMU)
	.type	Csym(setCacheMMU), %function
Csym(setCacheMMU):
	stmfd	sp!, {r4, lr}			// save registers
	mov	r4, r0				// save argument

        /* flush cache */
	bl	Csym(FlushCache)

        /* TLB flush */
	ldr	ip, =0
	mcr	p15, 0, ip, cr8, c7, 0		// Invalidate I/D-TLB

        /* set new r1 for CP15 */
	mrc	p15, 0, r2, cr1, cr0, 0
	ldr	r3, =0x3307			// V,I,R,S,C,A,M (B = 0)
	and	r0, r4, r3
	mvn	r3, r3				// clear old V,I,R,S,C,A,M
	and	r2, r2, r3
	orr	r0, r0, r2
	mcr	p15, 0, r0, cr1, cr0, 0
	nop
	nop

	ldmfd	sp!, {r4, lr}			// restore registers
	bx	lr
