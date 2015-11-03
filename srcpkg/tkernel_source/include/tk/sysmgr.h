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
 *	@(#)sysmgr.h (T-Kernel)
 *
 *	T-Kernel/SM
 */

#ifndef __TK_SYSMGR_H__
#define __TK_SYSMGR_H__

#include <basic.h>
#include <tk/util.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Memory attribute		tk_get_smb
 */
#define TA_NORESIDENT	0x00000010U	/* Non-resident */
#define TA_RNG0		0x00000000U	/* Protection level 0 */
#define TA_RNG1		0x00000100U	/* Protection level 1 */
#define TA_RNG2		0x00000200U	/* Protection level 2 */
#define TA_RNG3		0x00000300U	/* Protection level 3 */
#define TA_NOCACHE	0x00010000U	/* Non-cache area specify */

/*
 * System memory state information
 */
typedef struct t_rsmb {
	INT	blksz;		/* Block size (byte) */
	INT	total;		/* Number of all blocks */
	INT	free;		/* Number of remaining blocks */
} T_RSMB;

/*
 * Subsystem event
 */
#define TSEVT_SUSPEND_BEGIN	1	/* Before device suspend starts */
#define TSEVT_SUSPEND_DONE	2	/* After device suspend completes */
#define TSEVT_RESUME_BEGIN	3	/* Before device resume starts */
#define TSEVT_RESUME_DONE	4	/* After device resume completes */
#define TSEVT_DEVICE_REGIST	5	/* Device registration notification */
#define TSEVT_DEVICE_DELETE	6	/* Device unregistration notification */

/* ------------------------------------------------------------------------ */

/*
 * Non-resident system memory allocation
 */
IMPORT void* Vmalloc( size_t size );
IMPORT void* Vcalloc( size_t nmemb, size_t size );
IMPORT void* Vrealloc( void *ptr, size_t size );
IMPORT void  Vfree( void *ptr );

/*
 * Resident system memory allocation
 */
IMPORT void* Kmalloc( size_t size );
IMPORT void* Kcalloc( size_t nmemb, size_t size );
IMPORT void* Krealloc( void *ptr, size_t size );
IMPORT void  Kfree( void *ptr );

/* ------------------------------------------------------------------------ */

#ifndef __commarea__
#define __commarea__
typedef struct CommArea	CommArea;	/* sys/commarea.h */
#endif

/*
 * Definition for interface library automatic generation (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<tk/sysmgr.h>

[PREFIX]
SYSTEM
***/

/* [BEGIN SYSCALLS] */
/* System information management function */
/* ALIGN_NO 1 */
IMPORT INT tk_get_cfn( CONST UB *name, INT *val, INT max );
IMPORT INT tk_get_cfs( CONST UB *name, UB *buf, INT max );

/* System memory management function */
/* ALIGN_NO 0x100 */
IMPORT ER tk_get_smb( void **addr, INT nblk, UINT attr );
IMPORT ER tk_rel_smb( void *addr );
IMPORT ER tk_ref_smb( T_RSMB *pk_rsmb );

/* System internal use */
/* ALIGN_NO 0x1000 */
IMPORT ER  _GetKernelCommonArea( CommArea **area );
IMPORT int _syslog_send( const char *string, int len );
/* [END SYSCALLS] */

IMPORT INT _tk_get_cfn( CONST UB *name, INT *val, INT max );
IMPORT void FlushCache( CONST void *laddr, INT len );
IMPORT void FlushCacheM( CONST void *laddr, INT len, UINT mode );
IMPORT ER ControlCacheM( void *laddr, INT len, UINT mode );
IMPORT INT GetCacheLineSize( void );
IMPORT UW smPageCount( UW byte );
/* chkplv.c */
IMPORT ER ChkCallPLevel( void );

#ifdef __cplusplus
}
#endif
#endif /* __TK_SYSMGR_H__ */
