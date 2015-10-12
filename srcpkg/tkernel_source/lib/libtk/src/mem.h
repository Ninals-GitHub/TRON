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
 *	@(#)mem.h (libtk)
 *
 *	Memory local allocation library
 */

#include <basic.h>
#include <sys/memalloc.h>
#include <libstr.h>

/*
 * Minimum fragmentation unit
 *	Since memory is allocated in ROUNDSZ units,
 *	the lower three bits of the address must be 0.
 *	These low three bits are used in the flag in the area queue.
 */
#define ROUNDSZ		( sizeof(QUEUE) )	/* 8 byte */
#define ROUND(sz)	( ((UINT)(sz) + (ROUNDSZ-1)) & ~(ROUNDSZ-1) )

/* Minimum fragment size */
#define MIN_FRAGMENT	( _mem_minfragment )

/*
 * Flag that uses the lower bits of the area queue prev
 */
#define AREA_USE	0x00000001U	/* In use */
#define AREA_TOP	0x00000002U	/* Top of page */
#define AREA_END	0x00000004U	/* End of page */
#define AREA_MASK	0x00000007U

#define setAreaFlag(q, f)     ( (q)->prev = (QUEUE*)((UW)(q)->prev |  (UW)(f)) )
#define clrAreaFlag(q, f)     ( (q)->prev = (QUEUE*)((UW)(q)->prev & ~(UW)(f)) )
#define chkAreaFlag(q, f)     ( ((UW)(q)->prev & (UW)(f)) != 0 )

#define Mask(x)		( (QUEUE*)((UW)(x) & ~AREA_MASK) )
#define Assign(x, y)	( (x) = (QUEUE*)(((UW)(x) & AREA_MASK) | (UW)(y)) )

/*
 * Area size
 */
#define AreaSize(aq)	( (VB*)(aq)->next - (VB*)((aq) + 1) )
#define FreeSize(fq)	( (VB*)((fq) - 1)->next - (VB*)(fq) )

/*
 * Memory allocation check function
 */
IMPORT BOOL (*_mem_chkalloc)( void *ptr, int mode, MACB *macb );
