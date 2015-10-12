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
 *	@(#)tm_extsvc.S (libtm)
 *
 *	Extended service call
 */
#define	_in_asm_source_

#include "tmon.h"

/*
 * INT tm_extsvc(INT fno, INT par1, INT par2, INT par3 )
 */
TMCALL(tm_extsvc, TM_EXTSVC)
