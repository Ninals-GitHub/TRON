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
 *	eitentry.S
 *
 *       EM1D512 (ARM1176JZF-S) exception branch handling
 */

#define	_in_asm_source_

#include <machine.h>
#include <tk/sysdef.h>
#include <arm/em1d512.h>
#include <sys/sysinfo.h>

#define	base(n)		( (n) & 0xfffff000 )
#define	offs(n)		( (n) & 0x00000fff )

// see <sys/sysdepend/tef_em1d/sysinfo_depend.h>
#define	N_INTVEC	256

/*
 *       EIT entry
 */

	.section EITBRA, "ax"
	.arm
	.org	0x00000000
	b	startup_entry		// 00 : reset
	b	undef_entry		// 04 : undefined instruction exception
	b	svc_entry		// 08 : supervisor call (SVC)
	b	iabort_entry		// 0C : prefetch abort
	b	dabort_entry		// 10 : data abort
	nop				// 14 : (reserved)
	b	irq_entry		// 18 : interrupt
	.org	0x0000001c		// 1C : fast interrupt

/*
 * fast interrupt
 *       calls the handler defined at FIQ interrupt vector unconditionally.
 *       no saving of registers to stack is performed.
 *       the content of R12_fiq(ip) register is overwritten.
 */
fiq_entry:
	ldr	ip, =base(EIT_VECTBL)
	ldr	ip, [ip, #offs(EITVEC(EIT_FIQ))]
	bx	ip

/*
 * interrupt
 *       ACPU interrupt mask status register of Interrupt controller (AINT)
 *      Judge the priority of interrupts using (IT0_MST0,1,2),
 *       the highest interrupt's handler is called by jumping into it.
 *       Interrupt priority is descending order of interrupt factor (INT 0-95) , and INT 95 (IRQ 95) is highest.
 *       INT 0 (IRQ 0) has the lowest priority.
 *       If there is no cause of the interrupt, the handler of INT 95 (IRQ95) is called.
 *		+---------------+
 *	sp  ->	|R3		|
 *		|R12=ip		|
 *              |R14=lr         | <- return address from interrupt
 *		|SPSR		|
 *		+---------------+
 *      registers upon handler invocation
 *       ip = vector table address
 *       lr = indeterminate
 */
irq_entry:
	sub	lr, lr, #4		// return address adjustment
	srsdb	sp!, #PSR_IRQ		// save registers
	stmfd	sp!, {r3, ip}

	ldr	lr, =base(AINTBase)
	ldr	ip, =EITVEC(EIT_IRQ(95))

	ldr	r3, [lr, #offs(IT0_MST2)]
	cmp	r3, #0
	bne	l_irq_br

	sub	ip, ip, #32*4
	ldr	r3, [lr, #offs(IT0_MST1)]
	cmp	r3, #0
	bne	l_irq_br

	sub	ip, ip, #32*4
	ldr	r3, [lr, #offs(IT0_MST0)]
	cmp	r3, #0
	bne	l_irq_br

	ldr	ip, =EITVEC(EIT_IRQ(95))

  l_irq_br:
	clzne	r3, r3
	ldr	lr, [ip, -r3, lsl #2]!
	cmp	lr, #0
	bxne	lr
	b	default_entry

/*
 * GPIO interrupt
 *      Interrupt obtained by means of input port interrupt maskable status register (GIO_MST)
 *       is analyzed to check the interrupt priority, and if appropriate, the highest priority interrupt handler is entered.
 *       branch and call handler.
 *       interrupt priority is descending order of input port NUMBER (port 0 - port 127). port 127 has the highest priority, and
 *       port 0 has the lowest priority GPIO interrupts are grouped : each group has 16 interrupts, and
 *       their priorities are considered only within the context of each group.
 *       if there is no cause of interupt, the handler of IRQ95 is called.
 *		+---------------+
 *	sp  ->	|R3		|
 *		|R12=ip		|
 *              |R14=lr         | <- return address from interrupt
 *		|SPSR		|
 *		+---------------+
 *      registers upon handler invocation
 *       ip = vector table address
 *       lr = indeterminate
 */
 .macro gio_e reg, vec
	ldr	lr, =base(\reg)
	ldr	r3, [lr, #offs(\reg)]
	lsls	r3, r3, #16
	beq	l_gio_spurious

	ldr	ip, =\vec
	clz	r3, r3
	ldr	lr, [ip, -r3, lsl #2]!
	cmp	lr, #0
	bxne	lr
	b	default_entry
 .endm
 .macro gio_o reg, vec
	ldr	lr, =base(\reg)
	ldr	r3, [lr, #offs(\reg)]
	lsrs	ip, r3, #16
	beq	l_gio_spurious

	ldr	ip, =\vec
	clz	r3, r3
	ldr	lr, [ip, -r3, lsl #2]!
	cmp	lr, #0
	bxne	lr
	b	default_entry
 .endm

	.globl	Csym(_gio0Hdr)
	.type	Csym(_gio0Hdr), %function
	.globl	Csym(_gio1Hdr)
	.type	Csym(_gio1Hdr), %function
	.globl	Csym(_gio2Hdr)
	.type	Csym(_gio2Hdr), %function
	.globl	Csym(_gio3Hdr)
	.type	Csym(_gio3Hdr), %function
	.globl	Csym(_gio4Hdr)
	.type	Csym(_gio4Hdr), %function
	.globl	Csym(_gio5Hdr)
	.type	Csym(_gio5Hdr), %function
	.globl	Csym(_gio6Hdr)
	.type	Csym(_gio6Hdr), %function
	.globl	Csym(_gio7Hdr)
	.type	Csym(_gio7Hdr), %function
Csym(_gio0Hdr):	gio_e	GIO_MST(GIO_L),   EITVEC(EIT_GPIO( 15))
Csym(_gio1Hdr):	gio_o	GIO_MST(GIO_L),   EITVEC(EIT_GPIO( 31))
Csym(_gio2Hdr):	gio_e	GIO_MST(GIO_H),   EITVEC(EIT_GPIO( 47))
Csym(_gio3Hdr):	gio_o	GIO_MST(GIO_H),   EITVEC(EIT_GPIO( 63))
Csym(_gio4Hdr):	gio_e	GIO_MST(GIO_HH),  EITVEC(EIT_GPIO( 79))
Csym(_gio5Hdr):	gio_o	GIO_MST(GIO_HH),  EITVEC(EIT_GPIO( 95))
Csym(_gio6Hdr):	gio_e	GIO_MST(GIO_HHH), EITVEC(EIT_GPIO(111))
Csym(_gio7Hdr):	gio_o	GIO_MST(GIO_HHH), EITVEC(EIT_GPIO(127))

  l_gio_spurious:
	ldr	ip, =base(EITVEC(EIT_IRQ(95)))
	ldr	lr, [ip, #offs(EITVEC(EIT_IRQ(95)))]!
	cmp	lr, #0
	bxne	lr
	b	default_entry

/*
 * undefined instruction
 *		+---------------+
 *	sp  ->	|R12=ip		|
 *              |R14=lr         | <- the return address, i.e., the next address that follows the undefined instruction
 *		|SPSR		|
 *		+---------------+
 *      registers upon handler invocation
 *       ip = vector table address
 *       lr = indeterminate
 */
undef_entry:
	srsdb	sp!, #PSR_UND		// save registers
	stmfd	sp!, {ip}

	ldr	ip, =base(EITVEC(EIT_UNDEF))
	ldr	lr, [ip, #offs(EITVEC(EIT_UNDEF))]!
	cmp	lr, #0
	bxne	lr
	b	default_entry

/*
 * supervisor call(SVC)
 *      the valid range of supervisor call number is 0-255 (N_INTVEC - 1).
 *       if an out of range value is given, treat it as SVC 0, and invokes the default handler.
 *		+---------------+
 *	sp  ->	|R12=ip		|
 *              |R14=lr         | <- return address: the address that follows the SVC instruction
 *		|SPSR		|
 *		+---------------+
 *      registers upon handler invocation
 *       ip = vector table address
 *       lr = indeterminate
 */
svc_entry:
	srsdb	sp!, #PSR_SVC		// save registers
	stmfd	sp!, {ip}

	mrs	ip, spsr
	tst	ip, #PSR_T
	ldrneh	ip, [lr, #-2]		// Thumb instruction
	ldreq	ip, [lr, #-4]		// ARM instruction
	bicne	lr, ip, #0xff00
	biceq	lr, ip, #0xff000000
	cmp	lr, #N_INTVEC		// lr = software interrupt number
	movge	lr, #0

	ldr	ip, =EIT_VECTBL
	ldr	lr, [ip, lr, lsl #2]!
	cmp	lr, #0
	bxne	lr
	b	default_entry

/*
 * prefetch abort
 *       in the case of debug event, debug abort (instruction) handler is called.
 *       Otherwise, prefetch abort handler is called.
 *		+---------------+
 *	sp  ->	|R12=ip		|
 *              |R14=lr         | <- return address: the address of aborted instruction
 *		|SPSR		|
 *		+---------------+
 *      registers upon handler invocation
 *       ip = vector table address
 *       lr = indeterminate
 */
iabort_entry:
	sub	lr, lr, #4		// return address adjustment
	srsdb	sp!, #PSR_ABT		// save registers
	stmfd	sp!, {ip}

	mrc	p15, 0, ip, c5, c0, 1	// IFSR
	tst	ip, #0x400		// FS[4]
	and	ip, ip, #0x00f		// FS[3:0]
	cmpeq	ip, #FSR_DebugEvent

	ldr	ip, =base(EITVEC(EIT_IABORT))
	ldrne	lr, [ip, #offs(EITVEC(EIT_IABORT))]!
	ldreq	lr, [ip, #offs(EITVEC(EIT_IDEBUG))]!
	cmp	lr, #0
	bxne	lr
	b	default_entry

/*
 * data abort
 *       in the case of debug event, debug abort (data) handler is called.
 *       Otherwise, data abort handler is called.
 *		+---------------+
 *	sp  ->	|R12=ip		|
 *              |R14=lr         | <- return address: the address of aborted instruction
 *		|SPSR		|
 *		+---------------+
 *      registers upon handler invocation
 *       ip = vector table address
 *       lr = indeterminate
 */
dabort_entry:
	sub	lr, lr, #8		// return address adjustment
	srsdb	sp!, #PSR_ABT		// save registers
	stmfd	sp!, {ip}

	mrc	p15, 0, ip, c5, c0, 0	// DFSR
	tst	ip, #0x400		// FS[4]
	and	ip, ip, #0x00f		// FS[3:0]
	cmpeq	ip, #FSR_DebugEvent

	ldr	ip, =base(EITVEC(EIT_DABORT))
	ldrne	lr, [ip, #offs(EITVEC(EIT_DABORT))]!
	ldreq	lr, [ip, #offs(EITVEC(EIT_DDEBUG))]!
	cmp	lr, #0
	bxne	lr
	b	default_entry

/*
 * default handler
 *       stack contains the details of the generated exception.
 *      registers upon handler invocation
 *       ip = address of the vector table for the raised exception
 *       lr = indeterminate
 */
default_entry:
	ldr	lr, =base(EITVEC(EIT_DEFAULT))
	ldr	lr, [lr, #offs(EITVEC(EIT_DEFAULT))]
	bx	lr

	.pool
