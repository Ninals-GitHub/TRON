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
 *	@(#)misc.h (sys)
 *
 *	Various functions useful for such as debug
 */

#ifndef __SYS_MISC_H__
#define __SYS_MISC_H__

#include <basic.h>
#include <tk/tkernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Task register display (libtk)
 *	Display the contents of 'gr,' 'er,' and 'cr' using 'prfn.'
 *	'prfn' needs to be a printf compatible function.
 *	Return the number of rows displayed in the return value.
 */
IMPORT W PrintTaskRegister( int (*prfn)( const char *format, ... ),
				T_REGS *gr, T_EIT *er, T_CREGS *cr );

#ifdef __cplusplus
}
#endif
#endif /* __SYS_MISC_H__ */
