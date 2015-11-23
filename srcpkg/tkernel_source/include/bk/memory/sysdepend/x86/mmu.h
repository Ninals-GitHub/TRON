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
 *	mmu.h (T2EX)
 *	T2EX: MMU definitions (em1d)
 */

#include <typedef.h>
#include <tk/sysdef.h>
#include <cpu.h>
#include <tstdlib/bitop.h>

#include "pagedef.h"

/*
 * MMU configuration
 *
 *	MMU is configured as follows.
 *	These settings are set at the T-Monitor start-up, 
 *	and never (and must not be) changed later.
 *
 *	SCTLR.AFE = 1
 *	Enable page table access flags. Thus, AP[0..2] are treated as follows:
 *
 *		AP[0] = access bit
 *
 *		AP[1] AP[2]
 *		  0     0	kernel  R/W
 *		  0     1	kernel  R Only
 *		  1     0	user    R/W
 *		  1     1	user    R Only
 *
 *	SCTLR.TRE = 1
 *	Enable TEX remap.
 *
 *	PRRR = 0x000a8aa4
 *	NMRR = 0x44e048e0
 *	TEX[0], C, and B are remapped as follows:
 *
 *	TEX[0] C B	Memory type	Internal cache	External cache
 *	  *  0 0 0	Strongly-order	None		None
 *	  *  0 0 1	Device		None		None
 *	  #  0 1 0	Normal		WT-NA		WT-NA
 *	     0 1 1	Normal		WB-NA		WB-NA
 *	     1 0 0	Normal		None		None
 *	     1 0 1	Normal		WT-NA		WB-A
 *	     1 1 0	(unused)
 *	  *  1 1 1	Normal		WB-A		WB-A
 *
 *	Only the attribute sets with * sign are commonly used.
 *	For the ones with # sign, used only when SetCacheMode() is called.
 *
 *		WT-NA	Write-through, no allocate on write
 *		WB-NA	Write-back, no allocate on write
 *		WB-A	Write-back, allocate on write
 *
 *	TEX[2:1] is used by the operating system.
 */

#ifndef _MEMORY_MMU_H_
#define _MEMORY_MMU_H_

#include "pagedef.h"

/*
 * Inter-processor sharing of `Normal' memory
 *	Sharable attribute with disable cache on processors without cache-snoop.
 *	In that case, shared attribute should be set off by default.
 */
#if 0
#  define NormalMem_PShare	( PT_PShare )	/* set shared by default */
#else
#  define NormalMem_PShare	( 0 )		/* do not set shared by default */
#endif

/*
 * Page directory entry (first level descriptor)
 *	s  p
 *	0  0	section/page nonexistent
 *	0  1	page table definition
 *	1  0	section definition
 *	1  1	(reserved)
 */
typedef union PageDirectryEntry {
#if 0
	struct pde {			/* Page table definition */
		UW	p:1;		/* 0:page valid */
		UW	rw:1;		/* 1:r/w bit */
		UW	us:1;		/* 2:u/s bit */
		UW	pwt:1;		/* 3:page level write through */
		UW	pcd:1;		/* 4:page level cache disable */
		UW	a:1;		/* 5:access */
		UW	rsv1:1;		/* 6:reserved (always set to 0) */
		UW	ps:1;		/* 7:page size */
		UW	g:1;		/* 8:global */
		UW	os:3;		/* 9-11:available for os */
		UW	pfa:20;		/* 12-31:page table address */
	} c;
	unsigned long	value;
#endif
	unsigned long val;
} PDE;

/* PDE bits */
#define	PD_Present		MAKE_BIT32(0)
#define	PD_Writable		MAKE_BIT32(1)
#define	PD_User			MAKE_BIT32(2)
#define	PD_WriteThrough		MAKE_BIT32(3)
#define	PD_nCachable		MAKE_BIT32(4)
#define	PD_Access		MAKE_BIT32(5)
#define	PD_PageSize		MAKE_BIT32(7)
#define	PD_Global		MAKE_BIT32(8)
#define	PD_OS			MAKE_MASK32(9, 11)
#define	PD_Address		MAKE_MASK32(12, 31)

#define	DPFAtoLADR(pfa)		( toLogicalAddress((UW)(pfa) << PDIR_INDEX) )
#define	LADRtoDPFA(laddr)	( (UW)toPhysicalAddress(laddr) >> PDIR_INDEX )

#define	DPFAtoPADR(pfa)		( (VP)((UW)(pfa) << PDIR_INDEX) )
#define	PADRtoDPFA(paddr)	( (UW)(paddr) >> PDIR_INDEX )

/*
 * PDE values
 *	PDE_NONE	invalid page table entry
 *
 *	PDE_SYS_RW	System read/write data
 *	PDE_SYS_RO	System read-only data
 *	PDE_SYS_EXEC	System program code data
 *
 *	PDE_USR_RW	User read/write data
 *	PDE_USR_RO	User read-only data
 *	PDE_USR_EXEC	User program code
 */
#define	PDE_NONE	( 0 )

#define	PDE_SYS_RW	( PD_Present|PT_Writable )
#define	PDE_SYS_RO	( PD_Present )
#define PDE_SYS_EXEC	( PD_Present )

#define	PDE_USR_RW	( PD_Present|PD_User|PDE_SYS_RW )
#define	PDE_USR_RO	( PD_Present|PD_User|PDE_SYS_RO )
#define PDE_USR_EXEC	( PD_Present|PD_User|PDE_SYS_EXEC )

/*
 * The number of PTEs in a page table
 */
#define	N_PTE	( PDIR_ENTRIES )

/*
 * Page table entry (second level descriptor)
 *
 *	 p = 0	page nonexistent
 *	 p = 1	page definition
 */
typedef union PageTablEntry {
#if 0
	struct pte {			/* Page definition (p = 1) */
		UW	p:1;		/* 0:present */
		UW	rw:1;		/* 1:r/w */
		UW	us:1;		/* 2:u/s */
		UW	pwt:1;		/* 3:page level write through */
		UW	pcd:1;		/* 4:page level cache disable */
		UW	a:1;		/* 5:access */
		UW	d:1;		/* 6:dirty */
		UW	pat:1;		/* 7:page attribute table */
		UW	g:1;		/* 8:global */
		UW	os:3;		/* 9-11:available for os */
		UW	pfa:20;		/* 12-31:physical page address */
	} a;
	unsigned long	w;
#endif
	unsigned long val;
} PTE;


/* PTE bits */
#define	PT_Present		MAKE_BIT32(0)
#define	PT_Writable		MAKE_BIT32(1)
#define	PT_User			MAKE_BIT32(2)
#define	PT_WriteThrough		MAKE_BIT32(3)
#define	PT_nCachable		MAKE_BIT32(4)
#define	PT_Access		MAKE_BIT32(5)
#define	PT_Dirty		MAKE_BIT32(6)
#define	PT_AttributeTable	MAKE_BIT32(7)
#define	PT_Global		MAKE_BIT32(8)
#define	PT_OS			MAKE_MASK32(9, 11)
#define	PT_Address		MAKE_MASK32(12, 31)

/*
 * PTE Cache settings
 *	Memory type is `Device' or `Strongly Ordered' for cache-disabled areas.
 *	As device memory may become inaccessible when configured non-shared, 
 *	PShare attribute is set as well when turning cache off.
 */
#define	PT_CacheMask	( PT_nCachable )
#define	PT_nCacheOn	( ~PT_nCachable )
#define	PT_nCacheOff_D	( PT_nCachable )
#define	PT_CacheOff_S	( PT_nCachable )

/*
 * PTE values
 *	PTE_NONE	invalid page table entry
 *
 *	PTE_SYS_RW	System read/write data
 *	PTE_SYS_RO	System read-only data
 *	PTE_SYS_EXEC	System program code data
 *
 *	PTE_USR_RW	User read/write data
 *	PTE_USR_RO	User read-only data
 *	PTE_USR_EXEC	User program code
 */
#define	PTE_NONE	( 0 )

#define	PTE_SYS_RW	( PT_Present|PT_Writable )
#define	PTE_SYS_RO	( PT_Present )
#define PTE_SYS_EXEC	( PT_Present )

#define	PTE_USR_RW	( PT_Present | PT_User|PTE_SYS_RW )
#define	PTE_USR_RO	( PT_Present | PT_User|PTE_SYS_RO )
#define PTE_USR_EXEC	( PT_Present | PT_User|PTE_SYS_EXEC )

#define	PAGE_INDEX(laddr)	(((unsigned long)(laddr) >> PAGE_SHIFT) & 0x3FF)	// relative index based on page talbe

/*
 * Cache off
 */
Inline unsigned long PTE_CacheOff( unsigned long pte )
{
	return (pte | PT_nCachable);
}


#define	PDIR_INDEX(laddr)	( (unsigned long)(laddr) >> PDIR_SHIFT )
#define	PDIR_NUM(laddr)		PDIR_INDEX(laddr)
#define	PTBL_INDEX(laddr)	( ((unsigned long)(laddr) & PDIR_HI_MASK) >> PAGE_SHIFT )	// absolute index based on pde
#define	PTBL_NUM(laddr)		((unsigned long)(laddr) >> PAGE_SHIFT)

#define PRAMTBL_INDEX(laddr)	( (((unsigned long)(laddr) - REALMEMORY_TOP) & PDIR_HI_MASK) >> PAGE_SHIFT )

/*
 * Align memory address to the upper address boundary of page table (not its single entry)
 */
#define PageTableAlignU(laddr)	( ((unsigned long)(laddr) + ~PAGE_MASK) & PAGE_MASK )

/*
 * Page frame address
 */
#define	PFAtoLADR(pfa)		( toLogicalAddress((UW)(pfa) << PAGE_SHIFT) )
#define	LADRtoPFA(laddr)	( (UW)toPhysicalAddress(laddr) >> PAGE_SHIFT )

#define	PFAtoPADR(pfa)		( (VP)((UW)(pfa) << PAGE_SHIFT) )
#define	PADRtoPFA(paddr)	( (UW)(paddr) >> PAGE_SHIFT )


/* ------------------------------------------------------------------------ */
/*
 *	TLB control
 */

/*
 * Write back page table entry
 *	cache line size >= PDE size (16 bytes) is assumed
 */
IMPORT void WriteBackPageTable( const void *ptbl );

/*
 * Purge TLB page entry containing logical address `laddr'
 *	Specify to `pte' the page table entry address to purge. 
 *	Page table entry is written back to the main memory.
 */
IMPORT void PurgePageTLB( const void *laddr, PTE *pte );

/* ------------------------------------------------------------------------ */
/*
 *	Cache control**
 */

#  define SCTLR_ON	(~CR0_CD)
#  define SCTLR_OFF	(CR0_CD)

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getPtePfaFromLaddr
 Input		:unsigned long laddr
 		 < logical address >
 Output		:void
 Return		:unsigned long
 		 < page frame address of the pte which is
 		  corresponding to its logical address >
 Description	:translate logical address to page frame address of the pte
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT unsigned long getPtePfaFromLaddr(unsigned long laddr);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_system_pde
 Input		:void
 Output		:void
 Return		:unsigned long
 		 < address of SysPDEBase >
 Description	:get SysPDEBase
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT unsigned long get_system_pde(void);

#endif /* _MEMORY_MMU_H_ */
