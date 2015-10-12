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
 *	@(#)libs.h (sys)
 *
 *	Internal library functions
 */

#ifndef __SYS_LIBS_H__
#define __SYS_LIBS_H__

#include <basic.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Internal library error during process startup sequence
 */
IMPORT ER _StartupError;

/*
 * TRUE for multitasking library
 */
IMPORT BOOL _isUseMT( void );

/*
 * Common exclusion control lock in library
 */
IMPORT ER  _lib_lock( W lockno, BOOL ignore_mintr );
IMPORT ER  _lib_locktmo( W lockno, W tmo, BOOL ignore_mintr );
IMPORT void _lib_unlock( W lockno );

/* lockno (0 - 15) */
#define _LL_MEMALLOC	0	/* Allocate memory */
#define _LL_DLL		1	/* Dynamic loader */
#define _LL_GCC		2	/* gcc run-time support */
#define _LL_TF		3	/* TRON Code Framework (libtf) */
#define _LL_LOOKUP	4	/* character search library (liblookup) */

#ifdef __cplusplus
}
#endif
#endif /* __SYS_LIBS_H__ */
