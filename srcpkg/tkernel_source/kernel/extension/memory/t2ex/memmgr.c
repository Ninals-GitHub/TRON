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
 *    Modified by Nina Petipa at 2015/07/28
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
 *	memmgr.c (T2EX)
 *	T2EX: memory management
 */

#include <typedef.h>
#include "sysmgr.h"
#include "segmgr.h"
#include "pagedef.h"
#include "cache_info.h"
#include "mmu.h"
#include <sys/rominfo.h>
#include <sys/sysinfo.h>
#include <sys/imalloc.h>
#include <tk/cpu_conf.h>

#define GET_SMB_MIN_USER_LEVEL	MMU_MIN_USER_LEVEL

/*
 * Page
 */
typedef struct {
	VB	mem[PAGESIZE];
} PAGE;

typedef UINT	PN;	/* Page number (1 - maxpage) */

/*
 * Number of pages
 */
EXPORT UW smPageCount( UW byte )
{
	return PageCount(byte);
}

/*
 * Page management queue
 */
typedef struct {
	BOOL	cont:1;		/* 1 if multiple pages in succession */
	BOOL	use:1;		/* 1 if page in use */
	PN	next:30;
	UINT	rsv:2;
	PN	prev:30;
} PAGEQUE;

#define USE	TRUE	/* In use */
#define FREE	FALSE	/* Free */
#define CONT	TRUE	/* Continuation (multiple blocks in succession) */
#define ONE	FALSE	/* Independent */

LOCAL CONST PAGEQUE		_clrPageQue = { ONE, FREE, 0, 0, 0 };

#define clrPageQue(q)	( (q) = _clrPageQue )

/* Number of successive pages */
#define NumOfPages(q)	( ( (q)->cont )? ((q)+1)->next: 1 )

/*
 * Page management table
 *	Top of pageque is used for freeque.
 *	freeque is sorted in order from the smallest number of
 *	successive free pages.
 */
typedef struct {
	INT	maxpage;	/* Total number of pages */
	INT	freepage;	/* Number of free pages */
	PAGEQUE	*pageque;	/* Array of management information for
				   all pages */
	PAGE	*top_page;	/* Top page address */
} PAGETBL;

#define freeque			pageque[0]	/* Free page queue */

#define _PageAdr(pn, pt)	( (void*)((pt)->top_page + ((pn) - 1)) )
#define _PageNo(adr, pt)	( (PN)(((PAGE*)(adr) - (pt)->top_page) + 1) )

/* ------------------------------------------------------------------------ */

/*
 * Memory management table
 */
LOCAL PAGETBL	SysMemTbl;	/* System memory management table */

/*
 * Memory management exclusion control lock
 *	During OS startup, Lock() is called before CreateLock().
 *	The initialization value is set to prevent it switching to wait mode.
*/
LOCAL FastLock	MemLock = { -1, -1 };
#define LockMEM()	Lock(&MemLock)
#define UnlockMEM()	Unlock(&MemLock)

/* Set Object Name in .exinf for DEBUG */
#define OBJNAME_MMLOCK	"Mem"		/* Multi-lock for Memory Manager */

/* ------------------------------------------------------------------------ */

Inline void* PageAdr( PN pn, PAGETBL *pt )
{
	return _PageAdr(pn, pt);
}
Inline PN PageNo( CONST void *adr, PAGETBL *pt )
{
	adr = CachingAddr(adr);
	return _PageNo(adr, pt);
}

/*
 * Address check
 *	Returns TRUE if address is OK.
 */
LOCAL BOOL chkadr( CONST void *adr, PAGETBL *pt )
{
	adr = CachingAddr(adr);
	if ( adr >= (void*)pt->top_page && adr < (void*)(pt->top_page + pt->maxpage) ) {
		if ( ((UINT)adr % PAGESIZE) == 0 ) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Set page queue value
 */
Inline PAGEQUE setPageQue( BOOL u, BOOL c, PN n, PN p )
{
	PAGEQUE	q;
	q.use  = u;
	q.cont = c;
	q.next = n;
	q.prev = p;
	return q;
}

/*
 * Page queue initialization
 */
Inline void initPageQue( PN pn, PAGETBL *pt )
{
	pt->pageque[pn].next = pt->pageque[pn].prev = pn;
}

/*
 * Insert page queue
 *	Inserts ent directly prior to que.
 */
Inline void insertPageQue( PN que, PN ent, PAGETBL *pt )
{
	PAGEQUE	*qp = &pt->pageque[que];
	PAGEQUE *ep = &pt->pageque[ent];

	ep->prev = qp->prev;
	ep->next = que;
	pt->pageque[qp->prev].next = ent;
	qp->prev = ent;
}

/*
 * Isolate page queue
 *	Removes ent from queue.
 */
LOCAL void removePageQue( PN ent, PAGETBL *pt )
{
	PAGEQUE	*ep = &pt->pageque[ent];

	if ( ep->next != ent ) {
		pt->pageque[ep->prev].next = ep->next;
		pt->pageque[ep->next].prev = ep->prev;
	}
}

/*
 * TRUE if page queue is free
 */
Inline BOOL isEmptyPageQue( PN que, PAGETBL *pt )
{
	return ( pt->pageque[que].next == que );
}

/*
 * Free page queue search
 *	Searches for a queue with n free pages (or the closest
 *	number of free pages greater than n).
 *	If such a queue cannot be found, returns 0 (i.e., freeque).
 */
LOCAL PN searchFreeQue( INT n, PAGETBL *pt )
{
	PAGEQUE	*pageque = pt->pageque;
	PN	pn = 0;

	while ( (pn = pageque[pn].next) > 0 ) {
		if ( NumOfPages(&pageque[pn]) >= n ) {
			return pn;
		}
	}
	return 0;
}

/*
 * Append free page
 *	Registers as free pages n consecutive pages starting
 *	from the pn page.
 */
LOCAL void appendFreePages( PN pn, INT n, PAGETBL *pt )
{
	PN	ins;
	PAGEQUE	*pq = &pt->pageque[pn];

	/* Queue setting */
	pq->use  = FREE;
	pq->cont = ( n > 1 )? CONT: ONE;
	if ( n > 1 ) {
		pq[1]   = setPageQue(FREE, CONT, n, 0);
	}
	if ( n > 2 ) {
		pq[n-1] = setPageQue(FREE, CONT, n, 0);
	}

	/* Search for position where free pages added */
	ins = searchFreeQue(n, pt);

	/* Register free pages */
	insertPageQue(ins, pn, pt);
}

/*
 * Set queue for using page
 */
Inline void setUsePages( PN pn, INT n, UINT atr, PAGETBL *pt )
{
	PAGEQUE	*pq = &pt->pageque[pn];

	/* Queue setting */
	pq->use  = USE;
	pq->cont = ( n > 1 )? CONT: ONE;
	if ( n > 1 ) {
		pq[1]   = setPageQue(USE, CONT, n, 0);
	}
	if ( n > 2 ) {
		pq[n-1] = setPageQue(USE, CONT, n, 0);
	}

	initPageQue(pn, pt);
}

/*
 * Allocate page
 */
LOCAL void* getPage( INT nblk, UINT atr, PAGETBL *pt )
{
	PN	pn;
	INT	free;

	/* Free page search */
	pn = searchFreeQue(nblk, pt);
	if ( pn == 0 ) {
		return NULL;
	}
	free = NumOfPages(&pt->pageque[pn]);

	/* Remove from the free queue all consecutive free pages
	   starting from the pn page */
	removePageQue(pn, pt);

	/* Extract required pages only */
	setUsePages(pn, nblk, atr, pt);
	free -= nblk;

	if ( free > 0 ) {
		/* Return remaining pages to the free queue */
		appendFreePages(pn + (UINT)nblk, free, pt);
	}

	pt->freepage -= nblk;

	return PageAdr(pn, pt);
}

/*
 * Page release
 *	Returns the total number of pages released
 */
LOCAL INT relPage( CONST void *adr, PAGETBL *pt )
{
	PN	pn;
	PAGEQUE	*pq;
	INT	nblk, free;

	pn = PageNo(adr, pt);
	pq = &pt->pageque[pn];

	if ( pq->use == FREE ) {
		return E_PAR;
	}

	/* Number of pages to be released */
	free = nblk = NumOfPages(pq);

	/* Are the pages next to the released pages free? */
	if ( pn + (UINT)nblk <= pt->maxpage && (pq+(UINT)nblk)->use == FREE ) {

		/* Remove free pages next to the free queue */
		removePageQue(pn+(PN)nblk, pt);

		/* Merge free pages with released pages */
		nblk += NumOfPages(pq+nblk);
	}

	/* Are there free pages previous to the released pages? */
	if ( pn > 1 && (pq-1)->use == FREE ) {

		/* Number of free previous pages  */
		INT n = ( (pq-1)->cont )? (pq-1)->next: 1;

		/* Remove free pages previous to the free queue */
		removePageQue(pn-(PN)n, pt);

		/* Merge free pages and released pages */
		pn -= (UINT)n;
		nblk += n;

		/* Although essentially unnecessary, set to FREE in
		   case of erroneous calls trying to release the
		   same address more than once. */
		pq->use = FREE;
	}

	/* Register release page in free queue */
	appendFreePages(pn, nblk, pt);

	pt->freepage += free;

	return free;
}

/*
 * Memory management table initialization
 */
LOCAL ER initPageTbl( void *top, void *end, PAGETBL *pt )
{
	ER	ercd;
	INT	memsz, npage, tblpage;

	/* Align top with 8 byte unit alignment */
	top = (void*)(((unsigned long)top + 7) & ~0x00000007U);
	memsz = (long)((unsigned long)end - (unsigned long)top);

	/* Allocate page management table */
	pt->pageque = (PAGEQUE*)toLogicalAddress(top);

	/* The number of pages (excluding page management table) */
	npage = (long)(((unsigned long)memsz - sizeof(PAGEQUE))
					/ (PAGESIZE + sizeof(PAGEQUE)));

	/* The number of pages required for page management table */
	tblpage = PageCount(sizeof(PAGETBL) + sizeof(PAGEQUE) * npage);

	/* Allocate logical space for the page management table */
	ercd = _MakeSpace(toLogicalAddress(top), tblpage, 0, PTE_SYS_RW);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Adjust to match address in page size units
	   and determine top page address */
	pt->top_page = (PAGE*)(((unsigned long)(pt->pageque + npage + 1)
					+ (PAGESIZE-1)) / PAGESIZE * PAGESIZE);

	/* Recalculate number of pages */
	npage = (long)(((unsigned long)toLogicalAddress(end)
		- (unsigned long)pt->top_page) / (unsigned long)PAGESIZE);
	pt->maxpage  = npage;
	pt->freepage = npage;

	/* Page management table initialization */
	clrPageQue(pt->freeque);
	appendFreePages(1, npage, pt);

	return E_OK;

err_ret:
	vd_printf("err:initPageTbl\n");
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Allocate system memory
 *	attr = TA_RNGn | TA_NORESIDENT | TA_NOCACHE
 */
EXPORT void* GetSysMemBlk( INT nblk, UINT attr )
{
	ER	ercd;
	PAGETBL	*pt = &SysMemTbl;
	void	*adr;
	UW	pte;

	LockMEM();

	/* Get memory block */
	adr = getPage(nblk, 0, pt);
	if ( adr == NULL ) {
		goto err_ret0;
	}

	/* Make logical space */
	pte = ( ((attr & TA_RNG3) >> 8) < GET_SMB_MIN_USER_LEVEL )? PTE_SYS_RW: PTE_USR_RW;
	if ( (attr & TA_NOCACHE) != 0 ) pte = PTE_CacheOff(pte);
	ercd = _MakeSpace(adr, nblk, 0, pte);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}

	UnlockMEM();
	return adr;

err_ret1:
	relPage(adr, pt);

err_ret0:
	UnlockMEM();
	//TM_DEBUG_PRINT(("GetSysMemBlk E_NOMEM\n"));
	return NULL;
}

/*
 * System memory release
 */
EXPORT ER RelSysMemBlk( CONST void *addr )
{
	PAGETBL	*pt;
	INT	free;
	ER	ercd;

	pt = ( chkadr(addr, &SysMemTbl) )? &SysMemTbl: NULL;
	if ( pt == NULL ) {
		ercd = E_PAR;
		goto err_ret0;
	}

	LockMEM();

	/* Release memory block */
	free = relPage(addr, pt);
	if ( free < E_OK ) {
		ercd = free;
		goto err_ret1;
	}

	/* Invalidate released memory cache */
	FlushCache(addr, free * (W)PAGESIZE);

	/* Unmake logical space */
	ercd = _UnmakeSpace((void *)addr, free, 0);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}

	UnlockMEM();

	return E_OK;

err_ret1:
	UnlockMEM();
err_ret0:
#ifdef DEBUG
	TM_DEBUG_PRINT(("RelSysMemBlk ercd = %d\n", ercd));
#endif
	return ercd;
}

/*
 * Acquire system memory information
 */
EXPORT ER RefSysMemInfo( T_RSMB *pk_rsmb )
{
	LockMEM();
	pk_rsmb->blksz = PAGESIZE;
	pk_rsmb->total = SysMemTbl.maxpage;
	pk_rsmb->free  = SysMemTbl.freepage;
	UnlockMEM();

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * Memory management initialization sequence
 *	Initialization prior to T-Kernel/OS startup
 */
EXPORT ER init_memmgr( void )
{
	struct boot_info *info = getBootInfo();
	unsigned long lowmem_top = info->lowmem_top;
	//unsigned long lowmem_limit = info->lowmem_limit;
	unsigned long lowmem_end = info->lowmem_base + info->lowmem_limit;
	unsigned long memend;
	ER	ercd;
	

	/* Acquire system configuration definition information */
	ercd = _tk_get_cfn(SCTAG_REALMEMEND, (INT*)&memend, 1);
	if ( ercd < 1 || memend > lowmem_end ) {
		memend = lowmem_end;
	} else if (memend == ~(-1UL)) {
		/* use boot information				*/
		memend = lowmem_end;
	}

	/* System memory management table initialization */
	ercd = initPageTbl((void*)lowmem_top, (void*)memend, &SysMemTbl);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	info->lowmem_top = memend;  /* Update memory free space */

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("init_memmgr ercd = %d\n", ercd));
#endif
	return ercd;
}

/*
 * Memory management start sequence
 *	Initialization directly after T-Kernel/OS startup
 */
EXPORT ER start_memmgr( void )
{
	ER	ercd;

	/* Generate exclusion control lock */
	ercd = CreateLock(&MemLock, OBJNAME_MMLOCK);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("start_memmgr ercd = %d\n", ercd));
#endif
	return ercd;
}

/*
 * Memory management start sequence
 */
EXPORT ER finish_memmgr( void )
{
	return E_OK;
}
