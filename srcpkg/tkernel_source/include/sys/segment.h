/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/11/29
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)segment.h (sys)
 *
 *	Segment management
 */

#ifndef __SYS_SEGMENT_H__
#define __SYS_SEGMENT_H__

#include <cpu.h>
#include <basic.h>
#include <tk/sysext.h>
#include <tk/syslib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define INVADR			((void*)-1)	/* Invalid address */

#ifndef __pinfo__
typedef struct pinfo		PINFO;		/* Defined in sys/pinfo.h */
#define __pinfo__
#endif
#ifndef __diskinfo__
typedef struct diskinfo		DiskInfo;	/* Defined in device/disk.h */
#define __diskinfo__
#endif

struct phyblk {
	UW	blk;		/* Physical block number */
	UW	len;		/* Number of physical blocks */
};
#ifndef __phyblk__
typedef struct phyblk		PhyBlk;
#define __phyblk__
#endif

/*
 * CheckSpace(),CheckStrSpace() mode
 */
#define MA_READ		0x04U	/* Read */
#define MA_WRITE	0x02U	/* Write */
#define MA_EXECUTE	0x01U	/* Execute */


/*
==================================================================================

	Management 

==================================================================================
*/

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
 * Definitions for interface library auto generate (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<sys/segment.h>

[PREFIX]
SEG
***/

/* [BEGIN SYSCALLS] */

/* ALIGN_NO 0x100 */
#ifndef LockSpace
IMPORT ER  LockSpace( CONST void *laddr, INT len );
IMPORT ER  UnlockSpace( CONST void *laddr, INT len );
#endif
IMPORT INT CnvPhysicalAddr( CONST void *laddr, INT len, void **paddr );
IMPORT ER  ChkSpace( CONST void *laddr, INT len, UINT mode, UINT env );
IMPORT INT ChkSpaceTstr( CONST TC *str, INT max, UINT mode, UINT env );
IMPORT INT ChkSpaceBstr( CONST UB *str, INT max, UINT mode, UINT env );
IMPORT INT ChkSpaceLen( CONST void *laddr, INT len, UINT mode, UINT env, INT lsid );
IMPORT INT ReadMemSpace( void *laddr, void *buf, INT len, INT lsid );
IMPORT INT WriteMemSpace( void *laddr, void *buf, INT len, INT lsid );
IMPORT INT SetMemSpaceB( void *laddr, INT len, UB data, INT lsid );
IMPORT ER  FlushMemCache( void *laddr, INT len, UINT mode );
IMPORT INT SetCacheMode( void *addr, INT len, UINT mode );
IMPORT INT ControlCache( void *addr, INT len, UINT mode );
IMPORT ER  GetSpaceInfo( CONST void *addr, INT len, T_SPINFO *pk_spinfo );
IMPORT INT SetMemoryAccess( CONST void *addr, INT len, UINT mode );

/* ALIGN_NO 0x100 */
IMPORT ER MapMemory( CONST void *paddr, INT len, UINT attr, void **laddr );
IMPORT ER UnmapMemory( CONST void *laddr );

/* ALIGN_NO 0x1000 */
IMPORT ER MakeSpace( void *laddr, INT npage, INT lsid, UINT pte );
IMPORT ER UnmakeSpace( void *laddr, INT npage, INT lsid );
IMPORT ER ChangeSpace( void *laddr, INT npage, INT lsid, UINT pte );
/* [END SYSCALLS] */

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:ChkSpaceR
 Input		:CONST void *addr
 		 < address to check >
 		 INT len
 		 < check length >
 Output		:void
 Return		:ER
 		 < result >
 Description	:check read permission of address space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER ChkSpaceR( CONST void *addr, INT len );

#ifdef __cplusplus
}
#endif
#endif /* __SYS_SEGMENT_H__ */
