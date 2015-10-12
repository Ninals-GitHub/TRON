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
 *	power.c (EM1-D512)
 *	Power-Saving Function
 */

#include "sysmgr.h"

/*
 * Switch to power-saving mode
 */
EXPORT void low_pow( void )
{
	Asm("mcr p15, 0, %0, cr7, c0, 4":: "r"(0));
}

/*
 * Move to suspend mode
 */
EXPORT void off_pow( void )
{
	Asm("mcr p15, 0, %0, cr7, c0, 4":: "r"(0));
}
