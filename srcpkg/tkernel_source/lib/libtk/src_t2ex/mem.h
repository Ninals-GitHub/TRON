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

/* Minimum fragment size
 *	A size smaller than 'sizeof(QUEUE) * 2' will result in malfunction.
 */
#define MIN_FRAGMENT	( sizeof(QUEUE) * 2 )

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
