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
 *	@(#)libtk.h (libtk)
 *
 *	T-Kernel library common definitions
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/sysmgr.h>
#include <sys/libs.h>
#include <sys/memalloc.h>

/*
 * Initialization and deletion of library internal shared exclusive
 * control lock
 */
IMPORT ER  _init_liblock( void );
IMPORT void _delete_liblock( void );

/* ------------------------------------------------------------------------ */

IMPORT	MACB	_Kmacb;		/* Kmalloc control block */
IMPORT	MACB	_Vmacb;		/* Vmalloc control block */
IMPORT	MACB	_Smacb;		/* Smalloc control block */

/*
 * Memory allocation exclusion control
 */
#define MEMLOCK(ERR_RET) {						\
		if ( _lib_lock(_LL_MEMALLOC, TRUE) < E_OK ) {		\
			ERR_RET;					\
		}							\
	}

#define MEMUNLOCK() {							\
		_lib_unlock(_LL_MEMALLOC);				\
	}

/* ------------------------------------------------------------------------ */
