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
 *	reset.c
 *
 *       Reset and reboot after Flash ROM write
 */

#include "flash.h"
#include <tk/sysdef.h>

IMPORT void _start( void );	/* start address after reset */

/*
 * reset and reboot
 */
EXPORT void flashwr_reset( void )
{
#define	PAGETBL_BASE	(_UW *)0x30000000

	void (* volatile reset_p)( void ) = 0;

        /* Remap the NOR FlashROM area to its original space, and jump */
	*PAGETBL_BASE = 0x9402;	// Strongly-order, Kernel/RO
	DSB();
	Asm("mcr p15, 0, %0, cr8, cr7, 0":: "r"(0));	// I/D TLB invalidate
	Asm("mcr p15, 0, %0, cr7, cr5, 6":: "r"(0));	// invalidate BTC
	DSB();
	ISB();
	(*reset_p)();		/* call reset entry (does not return) */
}
