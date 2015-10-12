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
 *	reset.S
 *
 *       EM1-D512: initial setting after a reset.
 */

#define	_in_asm_source_

#include <machine.h>
#include <tk/sysdef.h>
#include <arm/em1d512.h>
#include <sys/sysinfo.h>

#include "setup_em1d512.h"

/*
 * macro for setting up registers
 */
.macro out_w reg, val
  .ifnes "\val", ""	// when val is empty, do nothing.
	ldr	r0, =\reg
	ldr	r1, =\val
	str	r1, [r0]
  .endif
.endm

.macro setup_param	// r0: address of parameter string.
0:			// * r0, r2, and r3 are going to be clobbered.
	ldmia	r0!, {r2, r3}
	cmp	r2, #0
	strne	r3, [r2]
	bne	0b
.endm

.macro	wait_nsec	// r0: wait time (nsec)
			// * Assume one step is 4 ns @ (500MHz)
	lsr	r0, r0, #2
0:
	subs	r0, r0, #1
	bne	0b
.endm

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

/*----------------------------------------------------------------------
        T-Monitor boot processing
----------------------------------------------------------------------*/
	.section .startup, "ax"
	.balign	4
	.globl	startup_entry
	.type	startup_entry, %function
	.org	0x00000000
startup_entry:
// SVC mode, FIQ/IRQ interrupt disabled
	mov	r0, #(PSR_SVC | PSR_I | PSR_F)
	msr	cpsr_fsxc, r0

// use On-Chip SRAM as stack area
	ldr	sp, =0xa0020000

// not in effect: MMU, cache (D/I), program-flow prediction, High-Vector, VIC
// in effect: Force AP, TEX remap, Subpage AP
	.DSB	r0
	mrc	p15, 0, r0, cr1, cr0, 0
	ldr	r1, =~0x01003f85
	and	r0, r0, r1
	ldr	r1, =0x30800000
	orr	r0, r0, r1
	mcr	p15, 0, r0, cr1, cr0, 0

// Setup clock divider
	mov	r0, #0
	ldr	r2, =CHG_L1_HOLD
	str	r0, [r2]		// release data hold when L1 is off
	mov	r0, #0x30000000
	ldr	r2, =AUTO_FRQ_CHANGE
	str	r0, [r2]		// automatic frequency change function is off

setup_clock_divider:
	adr	r0, param_table0
	setup_param

// Setup PLL1 (PLL3 is operating)
setup_pll1:
        // We assume Power ON mode: In other mode setting, we simply take it for granted that PLL has been configured already
	ldr	r2, =CLK_MODE_SEL
	ldr	r0, [r2]
	ands	r0, r0, #0x00000f00
	bne	setup_power_mode

	mov	r0, #0x79		// (default) PLL1=499.712MHz
	ldr	r2, =PLL1CTRL0
	str	r0, [r2]
	mov	r0, #0
	ldr	r2, =PLL1CTRL1
	str	r0, [r2]		// PLL starts to operate
	ldr	r2, =PLL_STATUS
wait_pll1:
	ldr	r0, [r2]		// Wait for PLL1 operation completion
	ands	r0, r0, #0x00000001
	beq	wait_pll1

// Setup power mode
setup_power_mode:
        // Transition from Power ON to Normal Mode A
	mov	r0, #1
	ldr	r2, =CLK_MODE_SEL
	str	r0, [r2]
wait_power_mode_change:
	ldr	r0, [r2]
	and	r0, r0, #0x00000f00
	cmp	r0, #0x00000100
	bne	wait_power_mode_change

// Setup PLL2 (needs to be configured in Normal Mode)
setup_pll2:
	mov	r0, #0xff		// PLL2 ceases to operate
	ldr	r2, =PLL2CTRL1
	str	r0, [r2]
	ldr	r2, =PLL_STATUS
wait_pll2_0:	
	ldr	r0, [r2]		// Wait for PLL1 operation
	ands	r0, r0, #0x00000100
	bne	wait_pll2_0

	mov	r0, #0x61		// PLL2=401.408MHz
	ldr	r2, =PLL2CTRL0
	str	r0, [r2]
	mov	r0, #0			// PLL2 starts to operate
	ldr	r2, =PLL2CTRL1
	str	r0, [r2]
	ldr	r2, =PLL_STATUS
wait_pll2_1:	
	ldr	r0, [r2]		// wait for PLL2 to stop operation.
	ands	r0, r0, #0x00000100
	beq	wait_pll2_1

// Setup pin multiplexer
setup_pin_mux:
	mov	r1, #0xff000000		// since 'adr' cannot be used, we manually make sure
	ldr	r0, =Csym(GPIOConfig)	// that the code is relocatable at 16MB units boundary.
	bic	r0, r0, r1
	and	r1, pc, r1
	orr	r0, r0, r1
	setup_param
	
// release reset of the internal modules
setup_module:
	adr	r0, param_table1
	setup_param
	ldr	r0, =100000
	wait_nsec

// supplying clock to modules.
setup_clock_distribution:
	adr	r0, param_table2
	setup_param

// Setup Bus controller
setup_bcr:
	adr	r0, param_table3
	setup_param

// initialization of DDR memory
	bl	setup_ddr

// creation of temporary page table
	ldr	r0, =PAGETBL_BASE
	ldr	r1, =0x00000000
	ldr	r2, =0x00000402	// Kernel/RW, Strongly-order
tmptable_loop:
	orr	r3, r1, r2
	str	r3, [r0], #4
	adds	r1, r1, #0x00100000
	bne	tmptable_loop

// Mapping of FlashROM area (0x70000000 - 0x72000000 --> 0x00000000 -)
	ldr	r0, =(PAGETBL_BASE + (0x700 << 2))
	ldr	r1, =0x00000000
	ldr	r2, =0x0000940e	// Kernel/RO, Normal WB/WA
flashtable_loop:
	orr	r3, r1, r2
	str	r3, [r0], #4
	adds	r1, r1, #0x00100000
	cmp	r1, #0x02000000
	bne	flashtable_loop
	
// initialization of CP15
	ldr	r0, =0x00000004
	mcr	p15, 0, r0, cr2, cr0, 2		// TTBCR
	ldr	r0, =(PAGETBL_BASE + 0x09)	// WB/WA, no-shared, cachable
	mcr	p15, 0, r0, cr2, cr0, 1		// TTBR1
	mcr	p15, 0, r0, cr2, cr0, 0		// TTBR0
	ldr	r0, =EITENT_BASE
	mcr	p15, 0, r0, cr12, cr0, 0	// VBAR
	ldr	r0, =0x000a8aa4
	mcr	p15, 0, r0, cr10, cr2, 0 	// PRRR
	ldr	r0, =0x44e048e0
	mcr	p15, 0, r0, cr10, cr2, 1	// NMRR
	ldr	r0, =0x55555555			// All client
	mcr	p15, 0, r0, cr3, cr0, 0		// Domain access

// MMU enable
	.DSB	r0
	mcr	p15, 0, r0, cr8, cr7, 0		// I/D TLB invalidate
	mcr	p15, 0, r0, cr7, cr5, 6		// invalidate BTC
	.DSB	r0
	.ISB	r0
	mrc	p15, 0, r0, cr1, cr0, 0
	orr	r0, r0, #0x00000001
	mcr	p15, 0, r0, cr1, cr0, 0
	.ISB	r0

// perform reset processing
	ldr	pc, =reset_entry

// initialization of DDR memory
setup_ddr:
	ldr	r0, =MEMCCLK270_SEL
	ldr	r1, =0x00000001		// MEMCCLK270 no phase delay
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGT1
	ldr	r1, =0x00000006		// start auto-calibration
	str	r1, [r0]
calibrate_loop:
	ldr	r1, [r0]
	ands	r1, r1, #0x00000002	// wait for complete
	beq	calibrate_loop

	ldr	r0, =MEMC_DDR_CONFIGT3
	ldr	r1, [r0]		// get calibration result
	ldr	r0, =MEMC_DDR_CONFIGT2
	str	r1, [r0]		// apply calibrated value

	ldr	r0, =MEMCCLK270_SEL
	ldr	r1, =0x00000000		// MEMCCLK270 270degree delay
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGT1
	ldr	r1, =0x000d0803
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGF
	ldr	r1, =0x00000015
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGA1
	ldr	r1, =0x53443203
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGA2
	ldr	r1, =0x28da1042
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGC2
	ldr	r1, =0x0000001d
	str	r1, [r0]

	ldr	r0, =200000
	wait_nsec

	ldr	r0, =MEMC_DDR_CONFIGC1
	ldr	r1, =0x80200033
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGC2
	ldr	r1, =0x00000018		// CS0: memory initialize sequence
	str	r1, [r0]
ddr_init_loop:
	ldr	r1, [r0]
	ands	r1, r1, #0x00000100
	beq	ddr_init_loop

	ldr	r0, =MEMC_REQSCH
	ldr	r1, =0x0000001f		// memory request schedule
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGC2
	ldr	r1, =0x00000090		// CS0: CMD_REQ release
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGR1
	ldr	r1, =0x00690069		// refresh counter
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGR2
	ldr	r1, =0x3777011f
	str	r1, [r0]

	ldr	r0, =MEMC_DDR_CONFIGR3
	ldr	r1, =0x00001415
	str	r1, [r0]

	bx	lr

	.pool

param_table0:
	.long	NORMALA_DIV		// ACPU =PLL1/1 (499.712MHz)
	.long	0x00244200		// ADSP =PLL1/1 (499.712MHz)
					// HBUS =PLL1/3 (166.571MHz)
					// LBUS =PLL1/6 ( 83.285MHz)
					// FLASH=PLL1/6 ( 83.285MHz)
					// MEMC =PLL1/3 (166.571MHz)
	.long	DIVU70SCLK
	.long	0x00000000		// U70_SCLK=PLL3/1 (229.376MHz)
	.long	DIVU71SCLK
	.long	0x00000000		// U71_SCLK=PLL3/1 (229.376MHz)
	.long	DIVU72SCLK
	.long	0x00000000		// U72_SCLK=PLL3/1 (229.376MHz)
	.long	DIVLCDLCLK
	.long	0x00000004		// LCD_LCLK=PLL2/16 (25.088MHz)
	.long	DIVIICSCLK
	.long	0x00530053		// IIC_SCLK=PLL3/48 (4.779MHz)
	.long	DIVTIMTIN
	.long	0x00000003		// Txx_TIN=PLL3/8 (28.672MHz)
	.long	DIVSP0SCLK
	.long	0x00000074		// SP0_SCLK=PLL3/128 (1.792MHz)
	.long	TI0TIN_SEL
	.long	0x00000000
	.long	TI1TIN_SEL
	.long	0x00000000
	.long	TI2TIN_SEL
	.long	0x00000000
	.long	TI3TIN_SEL
	.long	0x00000000
	.long	TIGnTIN_SEL
	.long	0x00000000

	.long	0x00000000		// (terminate)
	.long	0x00000000

param_table1:
	.long	RESETREQ0ENA
	.long	0xffffffff
	.long	RESETREQ0
	.long	0xffffffe7		// Reset everything, but DSP
	.long	RESETREQ0ENA
	.long	0x00000000
	.long	RESETREQ1ENA
	.long	0xffffffff
	.long	RESETREQ1
	.long	0xffffffff		// Reset everything
	.long	RESETREQ1ENA
	.long	0x00000000
	.long	RESETREQ2ENA
	.long	0xffffffff
	.long	RESETREQ2
	.long	0xffffffff		// Reset everything
	.long	RESETREQ2ENA
	.long	0x00000000
	.long	RESETREQ3ENA
	.long	0xffffffff
	.long	RESETREQ3
	.long	0xffffffff		// Reset everything
	.long	RESETREQ3ENA
	.long	0x00000000

	.long	0x00000000		// (terminate)
	.long	0x00000000

param_table2:
	.long	GCLKCTRL0ENA
	.long	0xffffffff
	.long	GCLKCTRL0
	.long	0xffffffff		// (default) module clock on
	.long	GCLKCTRL0ENA
	.long	0x00000000
	.long	GCLKCTRL1ENA
	.long	0xffffffff
	.long	GCLKCTRL1
	.long	0xffffffff		// (default) module clock on
	.long	GCLKCTRL1ENA
	.long	0x00000000
	.long	GCLKCTRL2ENA
	.long	0xffffffff
	.long	GCLKCTRL2
	.long	0xffffffff		// (default) module clock on
	.long	GCLKCTRL2ENA
	.long	0x00000000
	.long	GCLKCTRL3ENA
	.long	0xffffffff
	.long	GCLKCTRL3
	.long	0xffffffff		// (default) module clock on
	.long	GCLKCTRL3ENA
	.long	0x00000000
	.long	GCLKCTRL4ENA
	.long	0xffffffff
	.long	GCLKCTRL4
	.long	0xffffffff		// (default) module clock on
	.long	GCLKCTRL4ENA
	.long	0x00000000
	.long	AHBCLKCTRL0
	.long	0x00000000		// (default) prohibit automatic control
	.long	AHBCLKCTRL1
	.long	0x00000000		// (default) prohibit automatic control
	.long	APBCLKCTRL0
	.long	0x00000000		// (default) prohibit automatic control
	.long	APBCLKCTRL1
	.long	0x00000000		// (default) prohibit automatic control
	.long	CLKCTRL
	.long	0x00000000		// (default) prohibit automatic control
	.long	CLKCTRL1
	.long	0x00000000

	.long	0x00000000		// (terminate)
	.long	0x00000000

param_table3:
	.long	AB1_U70WAITCTRL
	.long	0x00010200		// recommended value for 83MHz operation
	.long	AB1_U71WAITCTRL
	.long	0x00010200		// recommended value for 83MHz operation
	.long	AB1_U72WAITCTRL
	.long	0x00010200		// recommended value for 83MHz operation
	.long	AB1_IIC2WAITCTRL
	.long	0x00010300		// recommended value for 83MHz operation
	.long	AB1_IICWAITCTRL
	.long	0x00010300		// recommended value for 83MHz operation
	.long	AB1_SDIAWAITCTRL
	.long	0x00010300
	.long	AB1_SDIBWAITCTRL
	.long	0x00010300
	.long	AB1_SDICWAITCTRL
	.long	0x00010300
	.long	AB1_U70READCTRL
	.long	0x00000000		// (default)
	.long	AB1_U71READCTRL
	.long	0x00000000		// (default)
	.long	AB1_U72READCTRL
	.long	0x00000000		// (default)
	.long	AB1_IIC2READCTRL
	.long	0x00000000		// (default)
	.long	AB1_IICREADCTRL
	.long	0x00000000		// (default)
	.long	AB1_SDIAREADCTRL
	.long	0x00000000		// (default)
	.long	AB1_SDIBREADCTRL
	.long	0x00000000		// (default)
	.long	AB1_SDICREADCTRL
	.long	0x00000000		// (default)

	// memory map setup (CS0-3)
	//
	// 0x00000000 +----------------------------+
	//            | Bank0/CS0 (not used)       |
	// 0x10000000 +----------------------------+
	//            | Bank1/CS1 (not used)       |
	// 0x20000000 +----------------------------+
	//            | Bank2/CS2 (not used)       |
	// 0x28000000 +----------------------------+
	//            | Bank2/CS3 (LAN controller) |
	// 0x30000000 +----------------------------+
	.long	AB0_CSnBASEADD(0)
	.long	0x00000000
	.long	AB0_CSnBITCOMP(0)
	.long	0xf0000000
	.long	AB0_CSnBASEADD(1)
	.long	0x10000000
	.long	AB0_CSnBITCOMP(1)
	.long	0xf0000000
	.long	AB0_CSnBASEADD(2)
	.long	0x20000000
	.long	AB0_CSnBITCOMP(2)
	.long	0xf8000000
	.long	AB0_CSnBASEADD(3)
	.long	0x28000000
	.long	AB0_CSnBITCOMP(3)
	.long	0xf8000000
	.long	AB0_FLASHCLKCTRL
	.long	0x00000001		// AB0:Flash=1:2

        // set up LAN controller
        // Assuming the use of 83.333 MHz (12ns/1clk), we set the following values.
	//	CSint=1clk (Read+1clk=24ns, Write+2clk=36ns)
	//	T0=0clk (0ns), T1=3clk(36ns), T2=2clk(24ns)
	.long	AB0_CSnWAITCTRL(3)
	.long	0x01020300
	.long	AB0_CSnWAITCTRL_W(3)
	.long	0x00020300
	.long	AB0_CSnREADCTRL(3)
	.long	0x00000000		// (default)
	.long	AB0_CSnWAIT_MASK(3)
	.long	0x00000000		// (default)
	.long	AB0_CSnCONTROL(3)
	.long	0x00010100		// (default)
	.long	AB0_FLASHCOMSET
	.long	0x00000008		// CS3 value above is reflected.
	
	.long	0x00000000		// (terminate)
	.long	0x00000000

/*----------------------------------------------------------------------
        Reset processing
----------------------------------------------------------------------*/
	.text
	.balign	4
	.globl	reset_entry
	.type	reset_entry, %function
reset_entry:

// SVC mode, FIQ/IRQ interrupt disabled
	mov	r0, #(PSR_SVC | PSR_I | PSR_F)
	msr	cpsr_fsxc, r0

// Switch to T-Monitor stack
	ldr	sp, =__stack_bottom

// not in effect: cache (D/I), program-flow prediction, High-Vector, VIC
// in effect: Force AP, TEX remap, Subpage AP
	.DSB	r0
	mrc	p15, 0, r0, cr1, cr0, 0
	ldr	r1, =~0x01003f84
	and	r0, r0, r1
	ldr	r1, =0x30800000
	orr	r0, r0, r1
	mcr	p15, 0, r0, cr1, cr0, 0

	.ISB	r0
	mcr	p15, 0, r0, cr8, cr7, 0		// I/D TLB invalidate
	.DSB	r0
	bl	Csym(FlushCache)		// Clean/invalidate I/D cache

// Turn on VFP
	mrc	p15, 0, r0, cr1, cr0, 2
	orr	r0, r0, #0x00f00000	// VFP(CP11,CP10) enable
	bic	r0, r0, #0xc0000000	// Should be Zero
	mcr	p15, 0, r0, cr1, cr0, 2
	.ISB	r0			// Flush Prefetch buffer

// initialize data area
	ldr	r1, =__data_org
	ldr	r2, =__data_start
	ldr	r3, =_edata
data_loop:
	ldmia	r1!, {r4-r7}	// copy in unit of 16 bytes
	stmia	r2!, {r4-r7}
	cmp	r2, r3
	blo	data_loop

// clear bss and stack area
	ldr	r2, =__bss_start
	ldr	r3, =__stack_bottom
	ldr	r4, =0
	ldr	r5, =0
	ldr	r6, =0
	ldr	r7, =0
bss_loop:
	stmia	r2!, {r4-r7}	// clear in units of 16 bytes
	cmp	r2, r3
	blo	bss_loop

// reset processing
	bl	procReset

// clear registers & initialize stack pointer
	ldr	r7, =__stack_top	// since it is 0-cleared, why not use it.

	ldmia	r7, {r8-r12,sp,lr}^	// usr: r8-r12, sp, lr

	mov	r0, #(PSR_FIQ | PSR_I | PSR_F)
	msr	cpsr_fsxc, r0
	ldmia	r7, {r8-r12,sp,lr}	// fiq: r8-r12, sp, lr, spsr
	msr	spsr_fsxc, lr
	ldr	sp, =__stack_top + 32

	mov	r0, #(PSR_IRQ | PSR_I | PSR_F)
	msr	cpsr_fsxc, r0
	ldr	sp, =__stack_top + 16	// irq: sp, lr, spsr
	mov	lr, #0
	msr	spsr_fsxc, lr

	mov	r0, #(PSR_ABT | PSR_I | PSR_F)
	msr	cpsr_fsxc, r0
	ldr	sp, =__stack_top + 64	// abt: sp, lr, spsr
	mov	lr, #0
	msr	spsr_fsxc, lr

	mov	r0, #(PSR_UND | PSR_I | PSR_F)
	msr	cpsr_fsxc, R0
	ldr	sp, =__stack_top + 48	// und: sp, lr, spsr
	mov	lr, #0
	msr	spsr_fsxc, lr

        // clear VFP
	mov	r0, #0x40000000		// EX=0,EN=1,SBZ/other flags = 0
	fmxr	fpexc, r0

	mov	r1, #0x00000000		// SBZ/other flags = 0
	fmxr	fpscr, r1

	fldmiad	r7, {d0-d15}		// zero clear

        // return to SVC mode
	mov	r0, #(PSR_SVC | PSR_I | PSR_F)
	msr	cpsr_fsxc, r0

	ldmia	r7, {r0-r7}		// r0-r7

// enter monitor by means of SVC #0 instruction (SVC mode)
  resetLoop:
	svc	#0
	b	resetLoop		// return will cause another reset

	.pool
