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
 *	@(#)syslib.h (T-Kernel)
 *
 *	System Library
 */

#ifndef __TK_SYSLIB_H__
#define __TK_SYSLIB_H__

#include <basic.h>
#include <tk/typedef.h>

#include <tk/sysdepend/syslib_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Task address space setting
 */
IMPORT ER SetTaskSpace( ID tskid );

/*
 * Address space check
 */
IMPORT ER ChkSpaceR( CONST void *addr, INT len );
IMPORT ER ChkSpaceRW( CONST void *addr, INT len );
IMPORT ER ChkSpaceRE( CONST void *addr, INT len );
IMPORT INT ChkSpaceBstrR( CONST UB *str, INT max );
IMPORT INT ChkSpaceBstrRW( CONST UB *str, INT max );
IMPORT INT ChkSpaceTstrR( CONST TC *str, INT max );
IMPORT INT ChkSpaceTstrRW( CONST TC *str, INT max );

/*
 * Address space lock
 */
IMPORT ER LockSpace( CONST void *addr, INT len );
IMPORT ER UnlockSpace( CONST void *addr, INT len );

/*
 * Get physical address
 */
IMPORT INT CnvPhysicalAddr( CONST void *vaddr, INT len, void **paddr );

/*
 * Wait
 */
IMPORT void WaitUsec( UINT usec );	/* micro second wait */
IMPORT void WaitNsec( UINT nsec );	/* nano second wait */

#ifdef __cplusplus
}
#endif
#endif /* __TK_SYSLIB_H__ */
