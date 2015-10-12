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
 *	@(#)memalloc.h (sys)
 *
 *	Memory allocation library
 */

#ifndef __SYS_MEMALLOC_H__
#define __SYS_MEMALLOC_H__

#include <basic.h>
#include <tk/typedef.h>
#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Memory allocation control information
 *	In the address, the &areaque position must be aligned in
 *	8 byte units. (The lower three bits must be 0)
 *	The 'nouse' area is used to adjust the address.
 *	It is not possible to write to 'nouse'.
 */
typedef struct MemoryAllocateControlBlock {
	const QUEUE	nouse;		/* Area used for alignment */

	/* Area queue connects the various partitioned areas of the
	 * allocated page.
	 * Addresses are arranged in ascending order within each page,
	 * and in no particular order between pages. */
	QUEUE		areaque;
	/* Free queue connects unused areas within the allocated
	 * page. Arranged in order of free area size, starting with
	 * the smallest. */
	QUEUE		freeque;

	UINT		pagesz;		/* Page size (number of bytes) */
	UINT		mematr;		/* Memory attributes */
	INT		testmode;	/* Test mode */

	/* Memory allocate/release function */
	void* (*getblk)( INT nblk, UINT mematr );
	void  (*relblk)( void *ptr );
} MACB;

/*
 * Correction to align the &areaque position with the 8 byte boundary
 */
#define AlignMACB(macb)		( (MACB*)((UW)macb & ~0x00000007U) )

IMPORT ER    _tkm_init( UINT mematr, MACB *macb );  /* for T-Kernel use */
IMPORT ER    _mem_init( UINT mematr, MACB *macb );  /* for extension use */

IMPORT void* _mem_malloc( size_t size, MACB *macb );
IMPORT void* _mem_calloc( size_t nmemb, size_t size, MACB *macb );
IMPORT void* _mem_realloc( void *ptr, size_t size, MACB *macb );
IMPORT void  _mem_free( void *ptr, MACB *macb );
IMPORT void  _mem_malloctest( int mode, MACB *macb );
IMPORT BOOL  _mem_malloccheck( void *ptr, MACB *macb );

/*
 * Option setting: minimum fragment size
 *	must be size 'sizeof(QUEUE) * 2' or more.
 */
IMPORT size_t _mem_minfragment;

#ifdef __cplusplus
}
#endif
#endif /* __SYS_MEMALLOC_H__ */
