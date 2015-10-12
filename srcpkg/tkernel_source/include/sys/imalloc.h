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
 *	@(#)imalloc.h (sys)
 *
 *	Kernel memory allocation
 *
 *	A function for allocating memory used in parts of T-Kernel.
 *	Not for general use.
 */

#ifndef __SYS_IMALLOC_H__
#define __SYS_IMALLOC_H__

#include <basic.h>
#include <tk/typedef.h>
#include <tk/sysmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Allocates resident memory which has the lowest protection
 * level (TSCVLimit) in which level where a T-Kernel system call
 * can be issued.
 */
IMPORT void* Imalloc( size_t size );
IMPORT void* Icalloc( size_t nmemb, size_t size );
IMPORT void  Ifree( void *ptr );

/*
 * Allocates memory with the attributes specified in 'attr'.
 *	attr = TA_RNGn | TA_NORESIDENT
 */
IMPORT void* IAmalloc( size_t size, UINT attr );
IMPORT void* IAcalloc( size_t nmemb, size_t size, UINT attr );
IMPORT void  IAfree( void *ptr, UINT attr );

/*
 * In all cases, malloc and calloc cannot be called while interrupt or
 * dispatch are disabled.
 * Only free can always be called while interrupt or dispatch are
 * disabled; however memory release may not complete. For this reason,
 * it is preferable to issue the call when interrupt and dispatch are
 * both enabled.
 * If release has not completed, the area is allocated in accordance
 * with the subsequent
 * malloc and calloc.
*/

/* ------------------------------------------------------------------------ */

/*
 * System memory management in block units
 */
IMPORT void* GetSysMemBlk( INT nblk, UINT attr );
IMPORT ER RelSysMemBlk( CONST void *addr );
IMPORT ER RefSysMemInfo( T_RSMB *rsmb );

#ifdef __cplusplus
}
#endif
#endif /* __SYS_IMALLOC_H__ */
