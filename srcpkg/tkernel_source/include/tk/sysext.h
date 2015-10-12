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
 *	sysext.h
 *
 *	Definitions related to Extension
 */

#ifndef __TK_SYSEXT_H__
#define	__TK_SYSEXT_H__

#include <basic.h>
#include "typedef.h"

#include <sys/sysdepend/segment_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Memory Cache Control
 */
IMPORT ER  FlushMemCache( void *laddr, INT len, UINT mode );
IMPORT INT SetCacheMode( void *addr, INT len, UINT mode );
IMPORT INT ControlCache( void *addr, INT len, UINT mode );

/*
 * mode of FlushMemCache()
 */
#define	TCM_ICACHE	0x0001	/* Invalidate instruction cache */
#define	TCM_DCACHE	0x0002	/* Flush data cache and then invalidate it */

/*
 * mode of SetCacheMode()
 */
#define CM_OFF		0x01	/* cache off */
#define CM_WB		0x02	/* cache on: writeback */
#define CM_WT		0x03	/* cache on: writethrough */
#define CM_CONT		0x10	/* Set up cache only for consecutive physical address range */
 
/*
 * mode of ControlCache()
 */
#define CC_FLUSH	0x01	/* flush cache */
#define CC_INVALIDATE	0x02	/* invaliate cache */
 
/*
 * Memory Map Function
 */
IMPORT ER MapMemory( CONST void *paddr, INT len, UINT attr, void **laddr );
IMPORT ER UnmapMemory( CONST void *laddr );

/*
 * Obtain the information about the address space
 */
typedef struct {
	void	*paddr;	/* physical address that is mapped to addr */
	void	*page;	/* the physical start address of the page in which addr resides */
	INT	pagesz;	/* page size (in bytes)) */
	INT	cachesz; /* cache line size (in bytes) */
	INT	cont;	/* the size of the area covered by the consecutive physical addresses (in bytes) */
} T_SPINFO;
IMPORT ER GetSpaceInfo( CONST void *addr, INT len, T_SPINFO *pk_spinfo );

/*
 * Operation on memory access right
 */
IMPORT INT SetMemoryAccess( CONST void *addr, INT len, UINT mode );

#ifdef __cplusplus
}
#endif
#endif /* __TK_SYSEXT_H__ */
