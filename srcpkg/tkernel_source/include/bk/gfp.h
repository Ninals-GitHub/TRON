/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
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

#ifndef	__BK_GFP_H__
#define	__BK_GFP_H__

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
#define	___GFP_DMA		0x00000001
#define	___GFP_HIGHMEM		0x00000002
#define	___GFP_DMA32		0x00000004
#define	___GFP_MOVALE		0x00000008
#define	___GFP_RECLAIMABLE	0x00000010
#define	___GFP_HIGH		0x00000020
#define	___GFP_IO		0x00000040
#define	___GFP_FS		0x00000080
#define	___GFP_COLD		0x00000100
#define	___GFP_NOWARN		0x00000200
#define	___GFP_REPEAT		0x00000400
#define	___GFP_NOFAIL		0x00000800
#define	___GFP_NORETRY		0x00001000
#define	___GFP_MEMALLOC		0x00002000
#define	___GFP_COMP		0x00004000
#define	___GFP_ZERO		0x00008000
#define	___GFP_NOMEMALLOC	0x00010000
#define	___GFP_HARDWALL		0x00020000
#define	___GFP_THISNODE		0x00040000
#define	___GFP_ATOMIC		0x00080000
#define	___GFP_ACCOUNT		0x00100000
#define	___GFP_NOTRACK		0x00200000
#define	___GFP_DIRECT_RECLAIM	0x00400000
#define	___GFP_OTHER_NODE	0x00800000
#define	___GFP_WRITE		0x01000000
#define	___GFP_KSWAPD_RECLAIM	0x02000000


#define	__GFP_DMA		___GFP_DMA
#define	__GFP_HIGHMEM		___GFP_HIGHMEM
#define	__GFP_DMA32		___GFP_DMA32
#define	__GFP_MOVABLE		___GFP_MOVABLE
#define	GFP_ZONEMASK		(__GFP_DMA | __GFP_HIGHMEM | __GFP_DMA32 | __GFP_MOVABLE)


#define	__GFP_RECLAIMABLE	___GFP_RECLAIMABLE
#define	__GFP_WRITE		___GFP_WRITE
#define	__GFP_HARDWALL		___GFP_HARDWALL
#define	__GFP_THISNODE		___GFP_THISNODE
#define	__GFP_ACCOUNT		___GFP_ACCOUNT


#define	__GFP_ATOMIC		___GFP_ATOMIC
#define	__GFP_HIGH		___GFP_HIGH
#define	__GFP_MEMALLOC		___GFP_MEMALLOC
#define	__GFP_NOMEMALLOC	___GFP_NOMEMALLOC

#define	__GFP_IO		___GFP_IO
#define	__GFP_FS		___GFP_FS
#define	__GFP_DIRECT_RECLAIM	___GFP_DIRECT_RECLAIM
#define	__GFP_KSWAPD_RECLAIM	___GFP_KSWAPD_RECLAIM
#define	__GFP_RECLAIM		(___GFP_DIRECT_RECLAIM | ___GFP_KSWAPD_RECLAIM)
#define	__GFP_NOFAIL		___GFP_NOFAIL
#define	__GFP_NORETRY		___GFP_NORETRY

#define	__GFP_COLD		___GFP_COLD
#define	__GFP_NOWARN		___GFP_NOWARN
#define	__GFP_COMP		___GFP_COMP
#define	__GFP_ZERO		___GFP_ZERO
#define	__GFP_NOTRACK		___GFP_NOTRACK
#define	__GFP_NOTRACK_FALSE_POSITIVE	__GFP_NOTRACK
#define	__GFP_OTHER_NODE	___GFP_OTHER_NODE

#define	__GFP_BITS_SHIFT	26
#define	__GFP_BITS_MASK		(1 << __GFP_BITS_SHIfT)

#define	GFP_ATOMIC		(__GFP_HIGH | __GFP_ATOMIC | __GFP_KSWAPD_RECLAIM)
#define	GFP_KERNEL		(__GFP_RECLAIM | __GFP_IO | __GFP_FS)
#define	GFP_KERNEL_ACCOUNT	(GFP_KERNEL | __GFP_ACCOUNT)
#define	GFP_NOWAIT		(__GFP_KSWAPD_RECLAIM)
#define	GFP_NOIO		(__GFP_RECLAIM)
#define	GFP_NOFS		(__GFP_RECLAIM | __GFP_IO)
#define	GFP_TEMPORARY		(__GFP_RECLAIM | __GFP_IO | __GFP_FS | __GFP_RECLAIMABLE)
#define	GFP_USER		(__GFP_RECLAIM | __GFP_IO | __GFP_FS | __GFP_HARDWALL)
#define	GFP_DMA			(__GFP_DMA)
#define	GFP_DMA32		(__GFP_DMA32)
#define	GFP_HIGHUSER		(GFP_USER | __GFP_HIGHMEM)
#define	GFP_HIGHUSER_MOVABLE	(GFP_HIGHUSER | __GFP_MOVABLE)
#define	GFP_TRANSHUGE		((GFP_HIGHUSER_MOVABLE | __GFP_COMP | __GFP_NOMEMALLOC	\
					| __GFP_NORETRY | __GFP_NOWARN) & ~__GFP_RECLAIM)



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
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BK_GFP_H__
