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
 *	@(#)tm_writedisk.S (libtm)
 *
 *	Write to disk
 */
#define	_in_asm_source_

#include "tmon.h"

/*
 * INT tm_writedisk( UB *dev, INT sblk, INT nblks, void *addr )
 */
TMCALL(tm_writedisk, TM_WRITEDISK)
