/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/08.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *    Modified by Nina Petipa at 2015/11/01
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	segmgr.c (T2EX)
 *	T2EX: segment manager
 */

#include <typedef.h>
/*
#include "sysmgr.h"
#include "segmgr.h"
#include "excmgr.h"
#include "cache_info.h"
#include "pagedef.h"*/
#include <device/cache_info.h>
#include <bk/memory/page.h>
#include <bk/memory/segmgr.h>
#include <sys/segment.h>
#include <sys/svc/ifsegment.h>

/*
 * Segment manager lock
 *	LockSEG() is called while the operating system is being started. 
 *	To prevent it from transiting to wait status, initial value is set.
 */
EXPORT	FastLock	SegLock = { -1, -1 };

/*
 * Get address space information
 */
LOCAL ER _GetSpaceInfo( CONST void *addr, INT len, T_SPINFO *pk_spinfo )
{
	T_RSMB	rsmb;
	ER	ercd = E_OK;

	if ( len <= 0 ) {
		ercd = E_PAR;
		goto err_ret;
	}
	ercd = ChkSpaceR(addr, len);
	if ( ercd < E_OK ){
		goto err_ret;
	}

	ercd = RefSysMemInfo(&rsmb);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	pk_spinfo->paddr   = toPhysicalAddress(addr);
	pk_spinfo->page	   = (void*)((UW)(pk_spinfo->paddr) & ~(rsmb.blksz-1));
	pk_spinfo->pagesz  = rsmb.blksz;
	pk_spinfo->cachesz = GetCacheLineSize();

	/* Assumes here that logical and physical addresses are mapped linear.
	   This assures that physical page addresses are also contiguous 
	   if logical page addresses are contiguous. */
	pk_spinfo->cont = len;

	return ercd;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_GetSpaceInfo ercd = %d\n", ercd));
#endif
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Memory map
 *	When paddr is NULL, allocate len bytes of memory having contiguous physical address,
 *	and returns the logical address to *laddr.
 *	When paddr is not NULL, map len bytes of physical memory from paddr and returns 
 *	the logical address to *laddr.
 *
 *	N.B. Access to *laddr may cause page fault.
 */
LOCAL ER _MapMemory( CONST void *paddr, INT len, UINT attr, void **laddr )
{
	ER	ercd;
	UINT	a;

	if ( len <= 0 ) {
		ercd = E_PAR;
		goto err_ret;
	}

	a = ( (attr & MM_USER) != 0 )? TA_RNG3: TA_RNG0;
	if ( (attr & MM_CDIS) != 0 ) {
		a |= TA_NOCACHE;
	}

	if ( paddr == NULL ) {
		/* Allocate memory automatically */
		*laddr = GetSysMemBlk((INT)smPageCount((UW)len), a);
		if ( *laddr == NULL ) {
			ercd = E_NOMEM;
			goto err_ret;
		}

		/* Set memory access privilege */
		ercd = _SetMemoryAccess(*laddr, len, (attr & (MM_READ|MM_WRITE|MM_EXECUTE)));
		if ( ercd < E_OK ) {
			RelSysMemBlk(*laddr);
			*laddr = NULL;
			goto err_ret;
		}

	} else {
		/* Logical address conversion */
		*laddr = toLogicalAddress(paddr);

		/* Flush cache */
		FlushCache(*laddr, len);

		if ( (attr & MM_CDIS) != 0 ) {
			/* Allocate logical addresses for cache off area */
			*laddr = toNoCacheLogicalAddress(*laddr);
		}
	}

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_MapMemory ercd = %d\n", ercd));
#endif
	return ercd;
}

/*
 * Memory unmap
 */
LOCAL ER _UnmapMemory( CONST void *laddr )
{
	ER	ercd;

	/* Memory release when memory is allocated automatically by MapMemory()
	 *	If the memory is not automatically allocated, RelSysMemBlk()
	 *	returns E_PAR.
	 */
	ercd = RelSysMemBlk(laddr);
	if ( ercd < E_OK && ercd != E_PAR ) {
		goto err_ret;
	}

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_UnmapMemory ercd = %d\n", ercd));
#endif
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Memory Cache Control
 */
LOCAL ER _FlushMemCache( void *laddr, INT len, UINT mode )
{
	ER	ercd;

	if ( (mode & ~(TCM_ICACHE|TCM_DCACHE)) != 0 ) {
		ercd = E_PAR;
		goto err_ret;
	}
	ercd = ChkSpaceR(laddr, len);
	if ( ercd < E_OK ){
		goto err_ret;
	}

	FlushCacheM(laddr, len, mode);

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_FlushMemCache ercd = %d\n", ercd));
#endif
	return ercd;
}

/*
 * Memory Cache Control
 */
LOCAL INT _ControlCache( void *addr, INT len, UINT mode )
{
	ER	ercd;

	if ( len <= 0 ) {
		ercd = E_PAR;
		goto err_ret;
	}
	if ( mode == 0 ) {
		ercd = E_PAR;
		goto err_ret;
	}
	ercd = ChkSpaceR(addr, len);
	if ( ercd < E_OK ){
		goto err_ret;
	}

	ercd = ControlCacheM(addr, len, mode);
	if( ercd != E_OK ){
		goto err_ret;
	}

	return len;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_ControlCache ercd = %d\n", ercd));
#endif
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Extended SVC entry
 */
LOCAL INT SegSVCentry( void *para, W fn )
{
	INT	ercd;

	switch ( fn ) {
		/* You can call it from all protection levels */
	  case SEG_FLUSHMEMCACHE_FN:
		break;

	  default:
		/* Caller protection level check */
		ercd = ChkCallPLevel();
		if ( ercd < E_OK ) {
			goto err_ret;
		}
	}

	switch ( fn ) {
	  case SEG_LOCKSPACE_FN:
	  case SEG_UNLOCKSPACE_FN:
		ercd = E_OK;
		break;

	  case SEG_CNVPHYSICALADDR_FN:
		{ SEG_CNVPHYSICALADDR_PARA *p = para;
		ercd = _CnvPhysicalAddr(p->laddr, p->len, p->paddr); }
		break;

	  case SEG_CHKSPACE_FN:
		{ SEG_CHKSPACE_PARA *p = para;
		ercd = _ChkSpace(p->laddr, p->len, p->mode, p->env); }
		break;
	  case SEG_CHKSPACETSTR_FN:
		{ SEG_CHKSPACETSTR_PARA *p = para;
		ercd = _ChkSpaceTstr(p->str, p->max, p->mode, p->env); }
		break;
	  case SEG_CHKSPACEBSTR_FN:
		{ SEG_CHKSPACEBSTR_PARA *p = para;
		ercd = _ChkSpaceBstr(p->str, p->max, p->mode, p->env); }
		break;

	  case SEG_MAPMEMORY_FN:
		{ SEG_MAPMEMORY_PARA *p = para;
		ercd = _MapMemory(p->paddr, p->len, p->attr, p->laddr); }
		break;
	  case SEG_UNMAPMEMORY_FN:
		{ SEG_UNMAPMEMORY_PARA *p = para;
		ercd = _UnmapMemory(p->laddr); }
		break;

	  case SEG_FLUSHMEMCACHE_FN:
		{ SEG_FLUSHMEMCACHE_PARA *p = para;
		ercd = _FlushMemCache(p->laddr, p->len, p->mode); }
		break;

	  case SEG_CHKSPACELEN_FN:
		{ SEG_CHKSPACELEN_PARA *p = para;
		ercd = _ChkSpaceLen(p->laddr, p->len, p->mode, p->env, p->lsid); }
		break;
	  case SEG_READMEMSPACE_FN:
	  case SEG_WRITEMEMSPACE_FN:
	  case SEG_SETMEMSPACEB_FN:
		ercd = E_NOSPT;
		break;
	  case SEG_MAKESPACE_FN:
		{ SEG_MAKESPACE_PARA *p = para;
		ercd = _MakeSpace(p->laddr, p->npage, p->lsid, p->pte); }
		break;
	  case SEG_UNMAKESPACE_FN:
		{ SEG_UNMAKESPACE_PARA *p = para;
		ercd = _UnmakeSpace(p->laddr, p->npage, p->lsid); }
		break;
	  case SEG_CHANGESPACE_FN:
		{ SEG_CHANGESPACE_PARA *p = para;
		ercd = _ChangeSpace(p->laddr, p->npage, p->lsid, p->pte); }
		break;

	  /* T-Kernel 2.0 */
	  case SEG_CONTROLCACHE_FN:
		{ SEG_CONTROLCACHE_PARA *p = para;
		ercd = _ControlCache(p->addr, p->len, p->mode); }
		break;
	  case SEG_GETSPACEINFO_FN:
		{ SEG_GETSPACEINFO_PARA *p = para;
		ercd = _GetSpaceInfo(p->addr, p->len, p->pk_spinfo); }
		break;
	  case SEG_SETCACHEMODE_FN:
		ercd = E_NOSPT;
		break;
	  case SEG_SETMEMORYACCESS_FN:
		{ SEG_SETMEMORYACCESS_PARA *p = para;
		ercd = _SetMemoryAccess(p->addr, p->len, p->mode); }
		break;

	  default:
		ercd = E_RSFN;
	}

err_ret:
	return ercd;
}

/*
 * Initialize segment management
 */
EXPORT ER init_segmgr( void )
{
	ER ercd;

	/* Initialize system exception manager */
	//ercd = init_excmgr();
	//if (ercd < E_OK ) {
	//	goto err_ret;
	//}

	/* Initialize logical space */
	ercd = InitLogicalSpace();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

err_ret:
	return ercd;
}

/*
 * Start segment management
 */
EXPORT ER start_segmgr( void )
{
	T_DSSY	dssy;
	ER	ercd;

	/* Create segment manager lock */
	ercd = CreateLock(&SegLock, "Seg");
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Start system exception manager */
	//ercd = start_excmgr();
	//if ( ercd < E_OK ) {
	//	goto err_ret;
	//}

	/* Register manager */
	dssy.ssyatr    = TA_NULL;
	dssy.ssypri    = SEG_PRI;
	dssy.svchdr    = (FP)SegSVCentry;
	dssy.breakfn   = NULL;
	dssy.startupfn = NULL;
	dssy.cleanupfn = NULL;
	dssy.eventfn   = NULL;
	dssy.resblksz  = 0;
	ercd = tk_def_ssy(SEG_SVC, &dssy);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("start_segmgr ercd = %d\n", ercd));
#endif
	return ercd;
}

/*
 * End segment manager
 */
EXPORT ER finish_segmgr( void )
{
	ER	ercd, ret = E_OK;

	/* Finish system exception manager */
	//ercd = finish_excmgr();
	//if ( ercd < E_OK ) {
	//	ret = ercd;
	//}

	/* Delete subsystem registration */
	ercd = tk_def_ssy(SEG_SVC, NULL);
	if ( ercd < E_OK ) {
		ret = ercd;
	}

	/* Remove segment manager lock */
	DeleteLock(&SegLock);

#ifdef DEBUG
	if ( ret < E_OK ) {
		TM_DEBUG_PRINT(("finish_segmgr ercd = %d\n", ret));
	}
#endif
	return ret;
}
