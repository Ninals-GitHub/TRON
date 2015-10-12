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
 *	patch.c (EM1-D512)
 *	System-dependent initialize process
 */

#include <basic.h>
#include <tk/tkernel.h>
#include "patch.h"

#if USE_SYSDEPEND_PATCH1
/*
 * Empty handler to ignore stray interrupts
 */
LOCAL void ignore_interrupt( UINT dintno )
{
static	INT	ignore_interrupt_counter;

	ignore_interrupt_counter++;
}

/*
 * System-dependent processes (before start_system)
 */
EXPORT void sysdepend_patch1( void )
{
	T_DINT	dint;

	/* register an empty handler to ignore stray interrupts */
	dint.intatr = TA_HLNG;
	dint.inthdr = ignore_interrupt;
	tk_def_int(DINTNO(IV_IRQ(95)), &dint);
}
#endif

#if USE_SYSDEPEND_PATCH2
/*
 * System-dependent processes (after start_system)
 */
EXPORT void sysdepend_patch2( void )
{
	/* Do nothing */
}
#endif
