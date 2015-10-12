/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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
 *	@(#)memalloc.c (libtk)
 *
 *	Memory allocation library
 *
 *	Redivides memory obtained in block units for allocation.
 *	Must be reentrant
 */

#include "mem.h"

/*
 * Memory allocation check function
 */
EXPORT BOOL (*_mem_chkalloc)( void *ptr, int mode, MACB *macb );

#define chkalloc	(*_mem_chkalloc)

/*
 * Byte size -->  number of pages
 */
Inline size_t toPageCount( size_t size, MACB *macb )
{
	return (size + (macb->pagesz-1)) / macb->pagesz;
}

/*
 * Free queue search
 *	Searches for a free space with the same size as 'size' or
 *	the next largest.
 *      If none is found, returns &freeque.
 */
LOCAL QUEUE* searchFreeArea( size_t size, MACB *macb )
{
	QUEUE	*q = &macb->freeque;

	/*
	 * Areas up to 1/4 of page size are searched starting
	 * from the smallest;
         * others are searched starting from the largest.
	 */
	if ( size > macb->pagesz / 4 ) {
		/* Searches in order of increasing size */
		size_t fsz = 0;
		while ( (q = q->prev) != &macb->freeque ) {
			fsz = (size_t)FreeSize(q);
			if ( fsz <= size ) {
				return ( fsz < size )? q->next: q;
			}
		}
		return ( fsz >= size )? q->next: q;
	} else {
		/* Searches in order of decreasing size */
		while ( (q = q->next) != &macb->freeque ) {
			if ( (size_t)FreeSize(q) >= size ) {
				break;
			}
		}
		return q;
	}
}

/*
 * Registration in free space free queue
 *	Free queue comprises a two-tier structure: a queue linking
 *	areas of differing size in order of size, and a queue
 *	linking areas that are the same size.
 *
 *     macb->freeque
 *      |
 *	|   +-----------------------+		+-----------------------+
 *	|   | AreaQue		    |		| AreaQue		|
 *	|   +-----------------------+		+-----------------------+
 *	+----> FreeQue size order   |	 +--------> FreeQue same size ----->
 *	|   |  FreeQue same size --------+      |   EmptyQue		|
 *	|   |			    |		|			|
 *	|   |			    |		|			|
 *	|   +-----------------------+		+-----------------------+
 *	|   | AreaQue		    |		| AreaQue		|
 *	|   +-----------------------+		+-----------------------+
 */
LOCAL void appendFreeArea( QUEUE *aq, MACB *macb )
{
	QUEUE	*fq;
	size_t	size = (size_t)AreaSize(aq);

	/* Search registration position */
	/*  Searches for a free space with the same size as 'size' or
	 *  the next largest.
	 *  If none is found, returns &freeque.
	 */
	fq = searchFreeArea(size, macb);

	/* Registration */
	clrAreaFlag(aq, AREA_USE);
	if ( fq != &macb->freeque && (size_t)FreeSize(fq) == size ) {
		QueInsert(aq + 1, fq + 1);
	} else {
		QueInsert(aq + 1, fq);
	}
	QueInit(aq + 2);
}

/*
 * Delete from free queue
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
 * Area registration
 *	Insert 'ent' directly after 'que'
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
 * Allocate new page capable of allocating a contiguous area at least
 * as big as 'size' byte
 */
Inline QUEUE* newPage( size_t size, MACB *macb )
{
	QUEUE	*top, *end;
	size_t	nblk;

	if ( macb->pagesz == 0 ) {
		return NULL;
	}

	/* Allocate page */
	nblk = toPageCount(size + sizeof(QUEUE)*2, macb);
	top = (QUEUE*)(*macb->getblk)(nblk, macb->mematr);
	if ( top == NULL ) {
		return NULL;
	}

	/* Register in area queue */
	end = (QUEUE*)((VB*)top + nblk * macb->pagesz) - 1;
	insertAreaQue(&macb->areaque, end);
	insertAreaQue(&macb->areaque, top);
	setAreaFlag(top, AREA_TOP);
	setAreaFlag(end, AREA_END);

	return top;
}

/*
 * Fragment and allocate
 */
Inline void* allocate( QUEUE *aq, size_t size, MACB *macb )
{
	QUEUE	*q;

	/* Any fragments smaller than the minimum fragment size
	   will also be allocated together */
	if ( (size_t)AreaSize(aq) - size >= MIN_FRAGMENT + sizeof(QUEUE) ) {

		/* Divide area in half */
		q = (QUEUE*)((VB*)(aq + 1) + size);
		insertAreaQue(aq, q);

		/* Register surplus area in free queue */
		appendFreeArea(q, macb);
	}
	setAreaFlag(aq, AREA_USE);

	return (void*)(aq + 1);
}

/* ------------------------------------------------------------------------ */

/*
 * Memory allocate
 */
EXPORT void* _mem_malloc( size_t size, MACB *_macb )
{
	MACB	*macb = AlignMACB(_macb);
	QUEUE	*q;

	if ( macb->testmode > 0 ) {
		chkalloc(NULL, 0, macb);
	}

	/* If smaller than the minimum fragment size,
	   allocate the minimum fragment size */
	if ( size > 0 && size < MIN_FRAGMENT ) {
		size = MIN_FRAGMENT;
	}

	size = ROUND(size);
	if ( size <= 0 ) {
		return NULL;
	}

	/* Search free queue */
	q = searchFreeArea(size, macb);

	if ( q != &macb->freeque ) {
		/* Free space available: first, isolate from free queue */
		removeFreeQue(q);

		q = q - 1;
	} else {
		/* No free space, then allocate new page */
		q = newPage(size, macb);
		if ( q == NULL ) {
			return NULL;  /* Insufficient memory */
		}
	}

	/* Allocate memory */
	return allocate(q, size, macb);
}

/*
 * Memory allocate  and clear
 */
EXPORT void* _mem_calloc( size_t nmemb, size_t size, MACB *macb )
{
	size_t	sz = nmemb * size;
	void	*p;

	/* Allocate memory */
	p = _mem_malloc(sz, macb);
	if ( p == NULL ) {
		return NULL;
	}

	/* Memory clear */
	return memset(p, 0, sz);
}

/*
 * Memory allocation size change
 */
EXPORT void* _mem_realloc( void *ptr, size_t size, MACB *_macb )
{
	MACB	*macb = AlignMACB(_macb);
	QUEUE	*aq;
	size_t	oldsz, sz;

	if ( macb->testmode > 0 ) {
		if ( !chkalloc(ptr, 0, macb) ) {
			return NULL;
		}
	}

	/* If smaller than minimum fragment size,
	   allocate minimum fragment size */
	if ( size > 0 && size < MIN_FRAGMENT ) {
		size = MIN_FRAGMENT;
	}

	size = ROUND(size);

	aq = (QUEUE*)ptr - 1;

	if ( ptr != NULL ) {
		/* Current allocation size */
		oldsz = (size_t)AreaSize(aq);

		/* Merge if next space is free space */
		if ( !chkAreaFlag(aq->next, AREA_END|AREA_USE) ) {
			removeFreeQue(aq->next + 1);
			removeAreaQue(aq->next);
		}

		sz = (size_t)AreaSize(aq);
	} else {
		sz = oldsz = 0;
	}

	if ( size <= sz ) {
		if ( size > 0 ) {
			/* Fragment current area and allocate */
			allocate(aq, size, macb);
		} else {
			/* Release area */
			_mem_free(ptr, macb);
			ptr = NULL;
		}
	} else {
		/* Allocate new area */
		void *newptr = _mem_malloc(size, macb);
		if ( newptr == NULL ) {
			/* Reallocate original area at original size */
			if ( ptr != NULL ) {
				allocate(aq, oldsz, macb);
			}
			return NULL;
		}

		if ( ptr != NULL ) {
			/* Copy contents */
			memcpy(newptr, ptr, oldsz);

			/* Release old area */
			_mem_free(ptr, macb);
		}
		ptr = newptr;
	}

	return ptr;
}

/*
 * Free memory
 */
EXPORT void  _mem_free( void *ptr, MACB *_macb )
{
	MACB	*macb = AlignMACB(_macb);
	QUEUE	*aq;

	if ( ptr == NULL ) {
		return;
	}

	if ( macb->testmode > 0 ) {
		if ( !chkalloc(ptr, 0, macb) ) {
			return;
		}
	}

	aq = (QUEUE*)ptr - 1;
	clrAreaFlag(aq, AREA_USE);

	if ( !chkAreaFlag(aq->next, AREA_END|AREA_USE) ) {
		/* Merge with just next free area */
		removeFreeQue(aq->next + 1);
		removeAreaQue(aq->next);
	}

	if ( !chkAreaFlag(aq, AREA_TOP) && !chkAreaFlag(aq->prev, AREA_USE) ) {
		/* Merge with just previous free area */
		aq = aq->prev;
		removeFreeQue(aq + 1);
		removeAreaQue(aq->next);
	}

	/* If whole page is empty, then release the page itself */
	if ( chkAreaFlag(aq, AREA_TOP) && chkAreaFlag(aq->next, AREA_END) ) {
		/* Page release */
		removeAreaQue(aq->next);
		removeAreaQue(aq);
		(*macb->relblk)(aq);
	} else {
		/* Register free area in free queue */
		appendFreeArea(aq, macb);
	}
}
