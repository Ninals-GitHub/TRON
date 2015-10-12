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
 *	mmu.h (T2EX)
 *	T2EX: MMU definitions (em1d)
 */

#include <typedef.h>
#include <tk/sysdef.h>
#include "cpu_insn.h"

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
	struct pde {			/* Page table definition */
		UW	p:1;		/* page valid */
		UW	s:1;		/* section valid */
		UW	rsv1:1;		/* reserved (always set to 0) */
		UW	ns:1;		/* non-secure */
		UW	rsv2:1;		/* reserved (always set to 0) */
		UW	domain:4;	/* domain */
		UW	rsv3:1;		/* reserved (always set to 0) */
		UW	pfa:22;		/* page table address */
	} c;
	struct sce {			/* Section definition */
		UW	p:1;		/* page valid */
		UW	s:1;		/* section valid */
		UW	b:1;		/* memory attribute */
		UW	c:1;		/* memory attribute */
		UW	xn:1;		/* execute never */
		UW	domain:4;	/* domain */
		UW	rsv3:1;		/* reserved (always set to 0) */
		UW	a:1;		/* access bit  (always set to 1 for valid sections)*/
		UW	u:1;		/* user access allowed */
		UW	tex0:1;		/* memory attribute */
		UW	tex1:2;		/* reserved for operating systems (always set to 0) */
		UW	nw:1;		/* never write */
		UW	ps:1;		/* inter-processor sharable */
		UW	ng:1;		/* non-global */
		UW	ss:1;		/* super-section (always set to 0) */
		UW	ns:1;		/* non-secure */
		UW	sba:12;		/* section base address */
	} s;
	UW	w;
} PDE;

#define	SECTION_NO(laddr)	( (UW)(laddr) >> 20 )
#define	SECTION_OFS(laddr)	( (UW)(laddr) & 0xfffff )

#define	DPFAtoLADR(pfa)		( toLogicalAddress((UW)(pfa) << 10) )
#define	LADRtoDPFA(laddr)	( (UW)toPhysicalAddress(laddr) >> 10 )

#define	DPFAtoPADR(pfa)		( (VP)((UW)(pfa) << 10) )
#define	PADRtoDPFA(paddr)	( (UW)(paddr) >> 10 )

/*
 * PDE values
 *	PDE_NONE	invalid page directory entry
 *	PDE_NORM	page table definition
 *	PDE_SECTION	section definition
 */
#define	PDE_NONE	( 0 )
#define	PDE_NORM	( 0x001 )
#define PDE_SECTION	( 0x010 )

/*
 * The number of PTEs in a page table
 */
#define	N_PTE	( 256 )

/*
 * Page table entry (second level descriptor)
 *
 *	 p = 0	page nonexistent
 *	 p = 1	page definition
 */
typedef union PageTablEntry {
	struct pte {			/* Page definition (p = 1) */
		UW	xn:1;		/* execute never */
		UW	p:1;		/* page valid */
		UW	b:1;		/* memory attribute */
		UW	c:1;		/* memory attribute */
		UW	a:1;		/* access bit (always set to 1 for valid pages) */
		UW	u:1;		/* user access allowed */
		UW	tex0:1;		/* memory attribute */
		UW	tex1:2;		/* memory attribute */
		UW	nw:1;		/* never write */
		UW	ps:1;		/* inter-processor sharable */
		UW	ng:1;		/* non-global */
		UW	pfa:20;		/* physical page address */
	} a;
	UW	w;
} PTE;

#define	MAX_PFA		( 0x00100000 )

/* PTE bits */
#define	PT_nExecutable		0x00000001
#define	PT_Present		0x00000002
#define	PT_Bufferable		0x00000004
#define	PT_Cachable		0x00000008
#define	PT_Access		0x00000010
#define	PT_User			0x00000020
#define	PT_TEX0			0x00000040
#define	PT_nWritable		0x00000200
#define	PT_PShare		0x00000400
#define	PT_nGlobal		0x00000800
#define	PT_Address		0xfffff000

/*
 * PTE Cache settings
 *	Memory type is `Device' or `Strongly Ordered' for cache-disabled areas.
 *	As device memory may become inaccessible when configured non-shared, 
 *	PShare attribute is set as well when turning cache off.
 */
#define	PT_CacheMask	( PT_Cachable|PT_Bufferable|PT_TEX0|PT_PShare )
#define	PT_CacheOn	( PT_Cachable|PT_Bufferable|PT_TEX0|NormalMem_PShare )
#define	PT_CacheOff_D	( PT_Bufferable|PT_PShare )
#define	PT_CacheOff_S	( PT_PShare )

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

#define	PTE_SYS_RW	( PT_Present|PT_nExecutable|PT_Access|PT_Bufferable|PT_Cachable|PT_TEX0|NormalMem_PShare )
#define	PTE_SYS_RO	( PT_Present|PT_nExecutable|PT_Access|PT_Bufferable|PT_Cachable|PT_TEX0|PT_nWritable|NormalMem_PShare )
#define PTE_SYS_EXEC	( PT_Present|PT_Bufferable|PT_Access|PT_Cachable|PT_TEX0|PT_nWritable|NormalMem_PShare )

#define	PTE_USR_RW	( PT_User|PTE_SYS_RW )
#define	PTE_USR_RO	( PT_User|PTE_SYS_RO )
#define PTE_USR_EXEC	( PT_User|PTE_SYS_EXEC )

/*
 * Cache off
 */
Inline UW PTE_CacheOff( UW pte )
{
	return (pte & ~PT_CacheMask) | PT_CacheOff_D;
}

/*
 * Logical address masks
 */
#define	PDIR_MASK		0xfff00000	/* page directory */
#define	PTBL_MASK		0x000ff000	/* page table */
#define	POFS_MASK		0x00000fff	/* page offset */

#define	PDIR_NUM(laddr)		( (UW)(laddr) >> 20 )
#define	PTBL_NUM(laddr)		( ((UW)(laddr) & PTBL_MASK) >> 12 )
#define	POFS_NUM(laddr)		( (UW)(laddr) & POFS_MASK )

#define PRAMTBL_NUM(laddr)	( ((UW)(laddr) - REALMEMORY_TOP) >> 12 )

/*
 * Align memory address to the upper address boundary of page table (not its single entry)
 */
#define PageTableAlignU(laddr)	( ((UW)(laddr) + 0x3ff) & ~0x3ff )

/*
 * Page frame address
 */
#define	PFAtoLADR(pfa)		( toLogicalAddress((UW)(pfa) << 12) )
#define	LADRtoPFA(laddr)	( (UW)toPhysicalAddress(laddr) >> 12 )

#define	PFAtoPADR(pfa)		( (VP)((UW)(pfa) << 12) )
#define	PADRtoPFA(paddr)	( (UW)(paddr) >> 12 )

/* ------------------------------------------------------------------------ */
/*
 *	TLB control
 */

/*
 * Write back page table entry
 *	cache line size >= PDE size (16 bytes) is assumed
 */
Inline void WriteBackPageTable( const void *ptbl )
{
	/* Clean data cache to PoC */
	Asm("mcr p15, 0, %0, cr7, c10, 1":: "r"(ptbl));
	DSB();
}

/*
 * Purge TLB page entry containing logical address `laddr'
 *	Specify to `pte' the page table entry address to purge. 
 *	Page table entry is written back to the main memory.
 */
Inline void PurgePageTLB( const void *laddr, PTE *pte )
{
	UW	lsid = 0; /* has no meaning on T2EX memory management */
	UW	mva = ((UW)laddr & ~0xfff) | lsid;

	WriteBackPageTable(pte);
	Asm("mcr p15, 0, %0, cr8, c7, 1":: "r"(mva));
	DSB(); ISB();
}

/* ------------------------------------------------------------------------ */
/*
 *	Cache control
 */

#  define SCTLR_ON	(CR1_M|CR1_A|CR1_C|CR1_Z|CR1_I|CR1_TRE|CR1_AFE|CR1_XP)
#  define SCTLR_OFF	(CR1_EE|CR1_S|CR1_R)

#endif /* _MEMORY_MMU_H_ */
