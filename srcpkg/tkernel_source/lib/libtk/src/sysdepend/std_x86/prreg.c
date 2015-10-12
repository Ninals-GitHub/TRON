/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)prreg.c (libtk/EM1-D512)
 *
 *	Display task register value
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <sys/misc.h>

/*
 * Uses prfn to display the contents of gr, er, and cr.
 * prfn must be a printf compatible function.
 */
EXPORT W PrintTaskRegister( int (*prfn)( const char *format, ... ),
				T_REGS *gr, T_EIT *er, T_CREGS *cr )
{
	return 6;  /* Number of display rows */
}
