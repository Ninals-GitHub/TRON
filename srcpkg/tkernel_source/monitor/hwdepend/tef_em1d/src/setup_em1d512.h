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
 *	setup_em1d512.h
 *
 *       EM1D-512 configuration information
 *
 *       this file included from assembly source files, too.
 */

#define	EITENT_BASE	0x70000000	// address for exception branch processing
#define	PAGETBL_BASE	0x30000000	// address of the first level table page.

/*
 * clock value
 */
#define	PLL1_CLK	499712		// 499.712MHz
#define	PLL2_CLK	401418		// 401.408MHz
#define	PLL3_CLK	229376		// 229.376MHz

#define	ACPU_CLK	(PLL1_CLK / 1)
#define	Txx_CLK		(PLL3_CLK / 8)

/*
 * assignment to DipSw (switches)
 */
#define	SW_ABT		0x0100		// Abort SW
#define	SW_MON		0x0020		// Monitor Boot
#define	SW_BHI		0x0000		// fix HI_BAUD_RATE

/*
 * LED display (two bits, 2 bits)
 */
#define	LED_POWERUP	0x01		// Power-on
#define	LED_MEMCLR	0xff		// Memory clear
