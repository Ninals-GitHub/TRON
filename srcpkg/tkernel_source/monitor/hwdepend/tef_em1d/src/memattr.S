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
 *	memattr.S
 *
 *       manipulation of page table
 */
#define _in_asm_source_

#include <machine.h>
#include <tk/sysdef.h>

#include "setup_em1d512.h"

/*
 * memory barrier macros
 */
.macro _mov reg, val
  .ifnes "\reg", "\val"
	mov	\reg, \val
  .endif
.endm
.macro .ISB reg, val=#0
	_mov	\reg, \val
	mcr	p15, 0, \reg, cr7, c5, 4
.endm
.macro .DSB reg, val=#0
	_mov	\reg, \val
	mcr	p15, 0, \reg, cr7, c10, 4
.endm
.macro .DMB reg, val=#0
	_mov	\reg, \val
	mcr	p15, 0, \reg, cr7, c10, 5
.endm

/*
 * change memory attribute
 *	void ChangeMemAttr(UW top, UW end, UW attr)
 *               change the memory attribute of memory area from `top' to `end' - 1 into `attr'
 *               The physical address where T-Monitor resides is assumed to start at 0x00000000.
 *               It must be called with disabled cache.
 */
	.text
	.balign	4
	.globl	Csym(ChangeMemAttr)
	.type	Csym(ChangeMemAttr), %function
Csym(ChangeMemAttr):
changememattr_start:
	stmfd	sp!, {r4, r5, r7, r9, r10}

        /* Map T-Monitor to 0x00000000 and up, the information before the mapping is saved. */
	ldr	ip, =PAGETBL_BASE
	ldr	r4, [ip]
	ldr	r10, =attr_prev
	str	r4, [r10]
	ldr	r4, =0x00009402		// Strongly-order, Kernel/RO
	str	r4, [ip]

	.DSB	r4
	mcr	p15, 0, r4, cr8, cr7, 0		// I/D TLB invalidate
	mcr	p15, 0, r4, cr7, cr5, 6		// invalidate BTC
	.DSB	r4
	.ISB	r4

        /* Jump to the address into the remapped area */
	ldr	r4, =changememattr_main
	ldr	r9, =0xfff00000
	bic	r4, r4, r9		// r4 &= 0x000fffff
	mov	pc, r4

	.pool

changememattr_main:	
	bic	r2, r2, r9		// r2 &= 0x000fffff
	mov	r4, r0, lsr #20
	add	r4, ip, r4, lsl #2	// r4 = (r2 >> 20) * 4 + PAGETBL_BASE

changememattr_loop:
	ldr	r5, [r4]		// *r4 = (*r4 & 0xfff00000) | r2
	and	r5, r5, r9
	orr	r5, r5, r2
	str	r5, [r4], #4
	add	r0, r0, #0x00100000
	cmp	r0, r1
	bne	changememattr_loop

        /* Jump to the address in the originally mapped area */
	.DSB	r4
	mcr	p15, 0, r4, cr8, cr7, 0		// I/D TLB invalidate
	mcr	p15, 0, r4, cr7, cr5, 6		// invalidate BTC
	.DSB	r4
	.ISB	r4

	ldr	pc, =changememattr_finish

	.pool

changememattr_finish:
        /* Unmap the T-Monitor in the area from 0x00000000 and up. */
	ldr	r4, [r10]			// attr_prev
	str	r4, [ip]			// PAGETBL_BASE

	.DSB	r4
	mcr	p15, 0, r4, cr8, cr7, 0		// I/D TLB invalidate
	mcr	p15, 0, r4, cr7, cr5, 6		// invalidate BTC
	.DSB	r4
	.ISB	r4

	ldmfd	sp!, {r4, r5, r7, r9, r10}
	bx	lr

	.pool


	.bss
attr_prev:
	.long	0
