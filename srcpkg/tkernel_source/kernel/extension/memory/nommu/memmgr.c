/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by T-Engine Forum at 2012/10/24.
 *
 *----------------------------------------------------------------------
 */

/*
 *	memmgr.c (T-Kernel/SM)
 *	No MMU version: memory management
 */

#include "sysmgr.h"
#include "cache_info.h"
#include <sys/rominfo.h>
#include <sys/imalloc.h>

/*
 * Page
 */
typedef struct {
	VB	mem[2*1024];
} PAGE;
#define PAGESZ		( sizeof(PAGE) )

typedef UINT	PN;	/* Page number (1 - maxpage) */

/*
 * Number of pages
 */
EXPORT UW smPageCount( UW byte )
{
	return (byte + (PAGESZ-1)) / PAGESZ;
}

/*
 * Page management queue
 */
typedef struct {
	BOOL	cont:1;		/* 1 if multiple pages in succession */
	BOOL	use:1;		/* 1 if page in use */
	PN	next:30;
	UINT	atr:2;		/* Memory block attributes */
	PN	prev:30;
} PAGEQUE;

#define USE	TRUE	/* In use */
#define FREE	FALSE	/* Free */
#define CONT	TRUE	/* Continuation (multiple blocks in succession) */
#define ONE	FALSE	/* Independent */

CONST LOCAL PAGEQUE	_clrPageQue = { ONE, FREE, 0, 0, 0 };

#define clrPageQue(q)	( (q) = _clrPageQue )

/* Number of successive pages */
#define NumOfPages(q)	( ( (q)->cont )? ((q)+1)->next: 1 )

/*
 * Memory block attributes
 */
#define MB_NOCACHE	1	/* Cache off */

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
#if USE_NOCACHE_MEMTBL
LOCAL PAGETBL	NoCacheMemTbl;	/* Non-cache area memory management table */
#endif

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
	void* adr = _PageAdr(pn, pt);

	if ( (pt->pageque[pn].atr & MB_NOCACHE) != 0 ) {
		adr = NoCachingAddr(adr);
	}
	return adr;
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
		if ( ((UINT)adr % PAGESZ) == 0 ) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Set page queue value
 */
Inline PAGEQUE setPageQue( BOOL u, BOOL c, PN n, PN p, UINT a )
{
	PAGEQUE	q;
	q.use  = u;
	q.cont = c;
	q.next = n;
	q.prev = p;
	q.atr  = a;
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
	pq->atr  = 0;
	if ( n > 1 ) {
		pq[1]   = setPageQue(FREE, CONT, n, 0, 0);
	}
	if ( n > 2 ) {
		pq[n-1] = setPageQue(FREE, CONT, n, 0, 0);
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
	pq->atr  = atr;
	if ( n > 1 ) {
		pq[1]   = setPageQue(USE, CONT, n, 0, 0);
	}
	if ( n > 2 ) {
		pq[n-1] = setPageQue(USE, CONT, n, 0, 0);
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
	INT	memsz, npage;

	/* Align top with 8 byte unit alignment */
	top = (void*)(((UINT)top + 7) & ~0x00000007U);
	memsz = (INT)((UINT)end - (UINT)top);

	/* Allocate page management table */
	pt->pageque = (PAGEQUE*)top;

	/* Determine provisional number of pages */
	npage = (INT)(((UINT)memsz - sizeof(PAGEQUE)) / (PAGESZ + sizeof(PAGEQUE)));

	/* Adjust to match address in page size units
	and determine top page address */
	pt->top_page = (PAGE*)(((UINT)(pt->pageque + npage + 1)
					+ (PAGESZ-1)) / PAGESZ * PAGESZ);

	/* Recalculate number of pages */
	npage = (INT)(((UINT)end - (UINT)pt->top_page) / (UINT)PAGESZ);
	pt->maxpage  = npage;
	pt->freepage = npage;

	/* Page management table initialization */
	clrPageQue(pt->freeque);
	appendFreePages(1, npage, pt);

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * Allocate system memory
 *	attr = TA_RNGn | TA_NORESIDENT | TA_NOCACHE
 */
EXPORT void* GetSysMemBlk( INT nblk, UINT attr )
{
	PAGETBL	*pt;
	void	*adr;
	UINT	mb_atr;

	/* Memory management table */
#if USE_NOCACHE_MEMTBL
	pt = ( (attr & TA_NOCACHE) != 0 )? &NoCacheMemTbl: &SysMemTbl;
#else
	pt = &SysMemTbl;
#endif

	/* Memory block attributes */
	mb_atr = 0;
	if ( (attr & TA_NOCACHE) != 0 ) {
		mb_atr |= MB_NOCACHE;
	}

	/* Get memory block */
	LockMEM();
	adr = getPage(nblk, mb_atr, pt);
	UnlockMEM();

#ifdef DEBUG
	if ( adr == NULL ) {
		BMS_DEBUG_PRINT(("GetSysMemBlk E_NOMEM\n"));
	}
#endif
	return adr;
}

/*
 * System memory release
 */
EXPORT ER RelSysMemBlk( CONST void *addr )
{
	PAGETBL	*pt;
	INT	free;
	ER	ercd;

	pt = ( chkadr(addr, &SysMemTbl) )?	&SysMemTbl:
#if USE_NOCACHE_MEMTBL
	     ( chkadr(addr, &NoCacheMemTbl) )?	&NoCacheMemTbl:
#endif
						NULL;
	if ( pt == NULL ) {
		ercd = E_PAR;
		goto err_ret;
	}

	LockMEM();
	free = relPage(addr, pt);
	UnlockMEM();
	if ( free < E_OK ) {
		ercd = free;
		goto err_ret;
	}

	/* Invalidate released memory cache */
	FlushCache(addr, free * (W)PAGESZ);

	return E_OK;

err_ret:
	BMS_DEBUG_PRINT(("RelSysMemBlk ercd = %d\n", ercd));
	return ercd;
}

/*
 * Acquire system memory information
 */
EXPORT ER RefSysMemInfo( T_RSMB *pk_rsmb )
{
	LockMEM();
	pk_rsmb->blksz = PAGESZ;
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
/* Low-level memory management information */
IMPORT	void	*lowmem_top, *lowmem_limit;
	void	*memend;
	ER	ercd;

	/* Acquire system configuration definition information */
	ercd = _tk_get_cfn(SCTAG_REALMEMEND, (INT*)&memend, 1);
	if ( ercd < 1 || (UINT)memend > (UINT)lowmem_limit ) {
		memend = lowmem_limit;
	}

	/* System memory management table initialization */
	ercd = initPageTbl(lowmem_top, memend, &SysMemTbl);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	lowmem_top = memend;  /* Update memory free space */

#if USE_NOCACHE_MEMTBL
	/* Non-cache area memory management table initialization */
	ercd = initPageTbl(NoCacheMemoryTop, NoCacheMemoryEnd, &NoCacheMemTbl);
	if ( ercd < E_OK ) {
		goto err_ret;
	}
#endif

	return E_OK;

err_ret:
	BMS_DEBUG_PRINT(("init_memmgr ercd = %d\n", ercd));
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
	BMS_DEBUG_PRINT(("start_memmgr ercd = %d\n", ercd));
	return ercd;
}

/*
 * Memory management start sequence
 */
EXPORT ER finish_memmgr( void )
{
	return E_OK;
}
