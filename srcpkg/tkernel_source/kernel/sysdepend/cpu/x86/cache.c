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
 *	cache.c (EM1-D512)
 *	Cache Operation
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/sysdef.h>

#define	CacheLineSZ	32

/*
 * Obtain cache line size
 */
EXPORT INT GetCacheLineSize( void )
{
	return CacheLineSZ;
}

/*
 * Flush cache
 *      Flush cache for an area that starts at laddr for len bytes.
 *      cache is written back and invalidated.
 *
 *      mode := [TCM_ICACHE] | [TCM_DCACHE]
 */
EXPORT void FlushCacheM( CONST void *laddr, INT len, UINT mode )
{
	ASM ("wbinvd	\n\t");
	DSB();
}

EXPORT void FlushCache( CONST void *laddr, INT len )
{
	FlushCacheM(laddr, len, TCM_ICACHE|TCM_DCACHE);
}

/*
 * Control cache
 *	mode := [CC_FLUSH] | [CC_INVALIDATE]
 */
EXPORT ER ControlCacheM( void *laddr, INT len, UINT mode )
{
	FlushCacheM(laddr, len, TCM_ICACHE|TCM_DCACHE);
	return E_OK;
}
