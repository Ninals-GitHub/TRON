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
 *	imalloc.c (T-Kernel/SM)
 *	Kernel Memory Allocation (Imalloc)
 */

#include "sysmgr.h"
#include <sys/imalloc.h>
#include <sys/queue.h>

/*
 * Memory allocation management information
 */
typedef struct {
	const QUEUE	nouse;		/* Area to match alignment */

	/* AreaQue for connecting each area where reserved pages are
	   divided Sort in ascending order of addresses in a page.
	   Do not sort between pages. */
	QUEUE		areaque;
	/* FreeQue for connecting unused area in reserved pages
	   Sort from small to large free spaces. */
	QUEUE		freeque;

	UINT		mematr;		/* Memory attribute */
} IMACB;

/*
 * Compensation for aligning "&areaque" position to 8 bytes border
 */
#define AlignIMACB(imacb)	( (IMACB*)((UW)(imacb) & ~0x00000007U) )

LOCAL	UINT		pagesz;		/* Page size (byte) */

/*
 * Minimum unit of subdivision
 *	The lower 3 bits of address is always 0
 *	because memory is allocated by ROUNDSZ.
 *	AreaQue uses the lower 3 bits for flag.
 */
#define ROUNDSZ		( sizeof(QUEUE) )	/* 8 bytes */
#define ROUND(sz)	( ((sz) + (ROUNDSZ-1)) & ~(ROUNDSZ-1) )

/* Minimum fragment size */
#define MIN_FRAGMENT	( sizeof(QUEUE) * 2 )

/*
 * Flag that uses the lower bits of AreaQue's 'prev'.
 */
#define AREA_USE	0x00000001U	/* In-use */
#define AREA_TOP	0x00000002U	/* Top of page */
#define AREA_END	0x00000004U	/* End of page */
#define AREA_MASK	0x00000007U

#define setAreaFlag(q, f)	( (q)->prev = (QUEUE*)((UW)(q)->prev |  (f)) )
#define clrAreaFlag(q, f)	( (q)->prev = (QUEUE*)((UW)(q)->prev & ~(f)) )
#define chkAreaFlag(q, f)	( ((UW)(q)->prev & (f)) != 0 )

#define Mask(x)		( (QUEUE*)((UW)(x) & ~AREA_MASK) )
#define Assign(x, y)	( (x) = (QUEUE*)(((UW)(x) & AREA_MASK) | (UW)(y)) )

/*
 * Area size
 */
#define AreaSize(aq)	((size_t)( (VB*)(aq)->next - (VB*)((aq) + 1) ))
#define FreeSize(fq)	((size_t)( (VB*)((fq) - 1)->next - (VB*)(fq) ))

/*
 * Byte size -> Page number
 */
Inline size_t PageCount( size_t size )
{
	return (size + (pagesz-1)) / pagesz;
}

/*
 * FreeQue search
 *	Search free area whose size is equal to 'blksz', or closest and
 *	larger than 'blksz'.
 *	If it can not be found, return '&imacb->freeque'.
 */
LOCAL QUEUE* searchFreeArea( size_t blksz, IMACB *imacb )
{
	QUEUE	*q = &imacb->freeque;

	/* For area that is less than 1/4 of the page size, search from
	   smaller size. Otherwise, search from larger size. */
	if ( blksz > pagesz / 4 ) {
		/* Search from larger size */
		size_t fsz = 0;
		while ( (q = q->prev) != &imacb->freeque ) {
			fsz = FreeSize(q);
			if ( fsz <= blksz ) {
				return ( fsz < blksz )? q->next: q;
			}
		}
		return ( fsz >= blksz )? q->next: q;
	} else {
		/* Search from smaller size */
		while ( (q = q->next) != &imacb->freeque ) {
			if ( FreeSize(q) >= blksz ) {
				break;
			}
		}
		return q;
	}
}

/*
 * Registration of free area on FreeQue
 *	FreeQue is composed of 2 types: Queue that links the different
 *	size of areas by size
 *	and queue that links the same size areas.
 *
 *	imacb->freeque
 *	|
 *	|  +-----------------------+	  +-----------------------+
 *	|  | AreaQue		   |	  | AreaQue		  |
 *	|  +-----------------------+	  +-----------------------+
 *	*---> FreeQue by size	   |  *----> FreeQue same size   ---->
 *	|  | FreeQue same size    ----*   | EmptyQue		  |
 *	|  |			   |	  |			  |
 *	|  |			   |	  |			  |
 *	|  +-----------------------+	  +-----------------------+
 *	|  | AreaQue		   |	  | AreaQue		  |
 *	v  +-----------------------+	  +-----------------------+
 */
LOCAL void appendFreeArea( QUEUE *aq, IMACB *imacb )
{
	QUEUE	*fq;
	size_t	size = AreaSize(aq);

	/* Registration position search */
	/*  Search free area whose size is equal to 'blksz',
	 *  or closest and larger than 'blksz'.
	 *  If it can not be found, return '&imacb->freeque'.
	 */
	fq = searchFreeArea(size, imacb);

	/* Registration */
	clrAreaFlag(aq, AREA_USE);
	if ( fq != &imacb->freeque && FreeSize(fq) == size ) {
		QueInsert(aq + 1, fq + 1);
	} else {
		QueInsert(aq + 1, fq);
	}
	QueInit(aq + 2);
}

/*
 * Delete from FreeQue
 */
LOCAL void removeFreeQue( QUEUE *fq )
{
	if ( !isQueEmpty(fq + 1) ) {
		QUEUE *nq = (fq + 1)->next;

		QueRemove(fq + 1);
		QueInsert(nq + 1, nq);
		QueRemove(nq);
		QueInsert(nq, fq);
	}

	QueRemove(fq);
}

/*
 * Register area
 *	Insert 'ent' just after 'que'
 */
LOCAL void insertAreaQue( QUEUE *que, QUEUE *ent )
{
	ent->prev = que;
	ent->next = que->next;
	Assign(que->next->prev, ent);
	que->next = ent;
}

/*
 * Delete area
 */
LOCAL void removeAreaQue( QUEUE *aq )
{
	Mask(aq->prev)->next = aq->next;
	Assign(aq->next->prev, Mask(aq->prev));
}

/*
 * Subdivide and allocate
 */
Inline void* mem_alloc( QUEUE *aq, size_t blksz, IMACB *imacb )
{
	QUEUE	*q;

	/* If there are fragments smaller than the minimum fragment size,
	   allocate them also */
	if ( AreaSize(aq) - blksz >= MIN_FRAGMENT + sizeof(QUEUE) ) {

		/* Divide area into 2 */
		q = (QUEUE*)((VB*)(aq + 1) + blksz);
		insertAreaQue(aq, q);

		/* Register remaining area to FreeQue */
		appendFreeArea(q, imacb);
	}
	setAreaFlag(aq, AREA_USE);

	return (void*)(aq + 1);
}

/*
 * Get memory
 */
LOCAL void* imalloc( size_t size, IMACB *imacb )
{
	QUEUE	*q;
	void	*mem;
	UW	imask;

	/* If it is smaller than the minimum fragment size,
	   allocate the minimum size to it. */
	if ( size < MIN_FRAGMENT ) {
		size = MIN_FRAGMENT;
	}
	size = ROUND(size);

	DI(imask);  /* Exclusive control by interrupt disable */

	/* Search FreeQue */
	q = searchFreeArea(size, imacb);
	if ( q != &imacb->freeque ) {
		/* There is free area: Split from FreeQue once */
		removeFreeQue(q);

		q = q - 1;
	} else {
		/* Reserve new pages because there is no free space */
		QUEUE	*e;
		size_t	n;

		/* Reserve pages */
		EI(imask);
		n = PageCount(size + sizeof(QUEUE) * 2);
		q = GetSysMemBlk(n, imacb->mematr);
		if ( q == NULL ) {
			goto err_ret;  /* Insufficient memory */
		}
		DI(imask);

		/* Register on AreaQue */
		e = (QUEUE*)((VB*)q + n * pagesz) - 1;
		insertAreaQue(&imacb->areaque, e);
		insertAreaQue(&imacb->areaque, q);
		setAreaFlag(q, AREA_TOP);
		setAreaFlag(e, AREA_END);
	}

	/* Allocate memory */
	mem = mem_alloc(q, size, imacb);

	EI(imask);
	return mem;

err_ret:
	BMS_DEBUG_PRINT(("imalloc error\n"));
	return NULL;
}

/*
 * Get memory
 */
LOCAL void* icalloc( size_t nmemb, size_t size, IMACB *imacb )
{
	size_t	sz = nmemb * size;
	void	*mem;

	mem = imalloc(sz, imacb);
	if ( mem == NULL ) {
		return NULL;
	}

	memset(mem, 0, sz);

	return mem;
}

/*
 * Free memory
 *	It may be called during interrupt disable. In this case, need to wait
 *	 until interrupt is enabled and until free.
 */
LOCAL void ifree( void *ptr, IMACB *imacb )
{
	QUEUE	*aq;
	UW	imask;

	DI(imask);  /* Exclusive control by interrupt disable */

	aq = (QUEUE*)ptr - 1;
	clrAreaFlag(aq, AREA_USE);

	if ( !chkAreaFlag(aq->next, AREA_END|AREA_USE) ) {
		/* Merge with free area in after location */
		removeFreeQue(aq->next + 1);
		removeAreaQue(aq->next);
	}

	if ( !chkAreaFlag(aq, AREA_TOP) && !chkAreaFlag(aq->prev, AREA_USE) ) {
		/* Merge with free area in front location */
		aq = aq->prev;
		removeFreeQue(aq + 1);
		removeAreaQue(aq->next);
	}

	/* If the whole page is free, then free the page.
	 * However, do not free the page if it is called during
	 * interrupt disabled.
	 */
	if ( !isDI(imask) && chkAreaFlag(aq, AREA_TOP) && chkAreaFlag(aq->next, AREA_END) ) {
		/* Free pages */
		removeAreaQue(aq->next);
		removeAreaQue(aq);
		EI(imask);
		RelSysMemBlk(aq);
		DI(imask);
	} else {
		/* Register free area to FreeQue */
		appendFreeArea(aq, imacb);
	}

	EI(imask);
}

/* ------------------------------------------------------------------------ */
/*
 * Allocate memory whose attributes are specified by 'attr.'
 *	attr = TA_RNGn | TA_NORESIDENT
 */

LOCAL IMACB	Imacb[2][2];

#define RING(attr)	( ( ((attr) & TA_RNG3) == TA_RNG3 )? 1: 0 )
#define RESIDENT(attr)	( ( ((attr) & TA_NORESIDENT) == 0 )? 1: 0 )

#define SelIMACB(attr)	( AlignIMACB(&Imacb[RING(attr)][RESIDENT(attr)]) )

EXPORT void* IAmalloc( size_t size, UINT attr )
{
	return imalloc(size, SelIMACB(attr));
}

EXPORT void* IAcalloc( size_t nmemb, size_t size, UINT attr )
{
	return icalloc(nmemb, size, SelIMACB(attr));
}

EXPORT void  IAfree( void *ptr, UINT attr )
{
	ifree(ptr, SelIMACB(attr));
}

/* ------------------------------------------------------------------------ */
/*
 * Allocate resident memory where T-Kernel system call is enabled
 * and the protection level (TSVCLimit) is lowest.
 */

/* SVC control protection level (T-Kernel/OS) */
IMPORT INT	svc_call_limit;

#define TA_RNG	( (UINT)svc_call_limit << 8 )

EXPORT void* Imalloc( size_t size )
{
	return IAmalloc(size, TA_RNG);
}

EXPORT void* Icalloc( size_t nmemb, size_t size )
{
	return IAcalloc(nmemb, size, TA_RNG);
}

EXPORT void  Ifree( void *ptr )
{
	IAfree(ptr, TA_RNG);
}

/* ------------------------------------------------------------------------ */

/*
 * IMACB Initialization
 */
LOCAL void initIMACB( UINT attr )
{
	IMACB	*imacb = SelIMACB(attr);

	QueInit(&imacb->areaque);
	QueInit(&imacb->freeque);
	imacb->mematr = attr;
}

/*
 * Imalloc initial setting
 */
EXPORT ER init_Imalloc( void )
{
	T_RSMB	rsmb;
	ER	ercd;

	ercd = RefSysMemInfo(&rsmb);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	pagesz = (UINT)rsmb.blksz;

	initIMACB(TA_RNG0);
	initIMACB(TA_RNG0|TA_NORESIDENT);
	initIMACB(TA_RNG3);
	initIMACB(TA_RNG3|TA_NORESIDENT);

	return E_OK;

err_ret:
	BMS_DEBUG_PRINT(("init_Imalloc ercd = %d\n", ercd));
	return ercd;
}
