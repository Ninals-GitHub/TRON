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
 *	@(#)tm_putstring.S (libtm)
 *
 *	Input single line from console
 */
#define	_in_asm_source_

#include "tmon.h"

/*
 * INT tm_putstring( UB *buff )
 */
TMCALL(tm_putstring, TM_PUTSTRING)
