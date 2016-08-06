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
 *	space.c (T2EX)
 *	T2EX: logical space management (em1d)
 */

#include <typedef.h>
/*
#include "sysmgr.h"
#include "segmgr.h"
#include "pagedef.h"
#include "mmu.h"
#include "memdef.h"*/
#include <bk/memory/page.h>
#include <bk/memory/segmgr.h>
#include <sys/rominfo.h>
#include <sys/segment.h>
#include <sys/sysinfo.h>
#include <tk/cpu_conf.h>
#include <cpu.h>
#include <libstr.h>

#include <bk/bprocess.h>

/*
==================================================================================

	Prototype Statement

==================================================================================
*/
IMPORT void* allocLowMemory(uint32_t size);
LOCAL INLINE PTE* getFirstPteFromPde(PDE *pde);

/*
==================================================================================

	DEFINES

==================================================================================
*/

/*
==================================================================================

	Kernel Management 

==================================================================================
*/
/*
 * Base address for page directory entries
 *
 *	SysPDEBase[i] is assigned as the PDE for logical address
 */
LOCAL	PDE	*SysPDEBase;

/*
 * Base address for SDRAM page table entries
 *
 *	SysRamPTEBase[i] is assigned as the PTE for logical address
 */
LOCAL	PTE	*SysRamPTEBase;

/*
 * SDRAM address limit
 */
LOCAL	UW	SysRamLimit;

/* ------------------------------------------------------------------------ */
/*
 *	Page table operations
 */
/*
 * Get page directory entry
 */
Inline PDE* GetPDE( const void *laddr )
{
//	return SysPDEBase + PDIR_INDEX(laddr);
	return (PDE*)(get_current_mspace()->pde + PDIR_INDEX(laddr));
}

/*
 * Get page table entry
 */
Inline PTE* GetPTE( const void *laddr )
{
	int index;
	void *pte_base_relative = (void*)((unsigned long)laddr - KERNEL_BASE_ADDR);
	
	if ((unsigned long)pte_base_relative < REALMEMORY_TOP ||
		(unsigned long)pte_base_relative >= SysRamLimit) {
		return NULL;
	}

	index = PDIR_INDEX(pte_base_relative) * NUM_PDIR
				+ PRAMTBL_INDEX(pte_base_relative);

	return(SysRamPTEBase + index);
}

/*
 * Set value to a page directory entry 
 *	Note: No TLB purge/cache operations are performed here.
 */
Inline void SetPDE( const void* laddr, PDE pde_v )
{
	PDE* pde = GetPDE(laddr);
	*pde = pde_v;
}

/*
 * Set value to a page table entry
 *	When purge is TRUE,  corresponding TLB entry is purged.
 *	When purge is FALSE, no TLB operation is performed.
 */
LOCAL ER SetPTE( const void *laddr, PTE pte_v, BOOL purge )
{
	PTE* pte = GetPTE(laddr);
	if (pte == NULL)
		return E_MACV;

	*pte = pte_v;

	if ( purge )
		PurgePageTLB(laddr, pte);
	else
		WriteBackPageTable(pte);

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * Flush entire cache
 *	Writes back the data cache, and invalidate both instruction and data cache.
 */
EXPORT void FlushAllCache( void )
{
	/* Clean and invalidate entire data cache */
	//Asm("mcr p15, 0, %0, cr7, c14, 0":: "r"(0));
	/* Invalidate entire instruction cache */
	//Asm("mcr p15, 0, %0, cr7, c5, 0":: "r"(0));
	DSB();
}

/* ------------------------------------------------------------------------ */

/*
 * Make logical address space
 *	Allocate 'npage' pages of logical space having attribute 'set_pte',
 *	starting from the page containing 'laddr'.
 *	'set_pte' is the value to be set in the page table entry (PTE).
 *
 *	This implementation forbids specification of non-SDRAM area (E_MACV).
 */
EXPORT ER __MakeSpace( void *laddr, INT npage, INT lsid, UINT set_pte )
{
	VP	la = laddr;
	INT	np;
	PTE	*pte, pte_v;
	ER	ercd;

	for ( np = 0 ; np < npage; np++ ) {
		/* Get page table entry */
		pte = GetPTE(la);
		if ( pte == NULL ) {
			/* Invalid address region, or the page is made already */
			ercd = E_MACV;
			goto err_ret;
		}

		/* Update page table entry value */
		pte_v.val = set_pte | getPtePfaFromLaddr((unsigned long)la);
		SetPTE(la, pte_v, TRUE);

		la = NextPage(la);	/* Next page */
	}

	return E_OK;

err_ret:
	__UnmakeSpace(laddr, np, lsid);
#ifdef DEBUG
	TM_DEBUG_PRINT(("__MakeSpace ercd = %d\n", ercd));
#endif
	return ercd;
}

EXPORT ER _MakeSpace( void *laddr, INT npage, INT lsid, UINT set_pte )
{
	ER	ercd;

	set_pte |= PT_Present;

	LockSEG();
	ercd = __MakeSpace(laddr, npage, lsid, set_pte);
	UnlockSEG();
	return ercd;
}

/*
 * Unmake logical address space
 *	Deallocate 'npage' pages of logical space, starting from the page
 *	containing 'laddr'.
 *
 *	This implementation forbids specification of non-SDRAM area (E_MACV).
 */
EXPORT ER __UnmakeSpace( void *laddr, INT npage, INT lsid )
{
	VP	la = laddr;
	INT	np;
	PTE	*pte, pte_v;
	ER	ercd;

	for ( np = 0 ; np < npage; np++ ) {
		/* Get page table entry */
		pte = GetPTE(la);
		if ( pte == NULL ) {
			/* Invalid address region */
			ercd = E_MACV;
			goto err_ret;
		}

		/* Ignore the entries that are already invalid, and continue */
		if ( !(pte->val & PT_Present) ) {
			la = NextPage(la);
			continue;
		}

		/* Update page table entry value */
		pte_v.val = PTE_NONE;
		SetPTE(la, pte_v, TRUE);

		la = NextPage(la);	/* Next page */
	}

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("__UnmakeSpace ercd = %d\n", ercd));
#endif
	return ercd;
}

EXPORT ER _UnmakeSpace( void *laddr, INT npage, INT lsid )
{
	ER	ercd;

	LockSEG();
	ercd = __UnmakeSpace(laddr, npage, lsid);
	UnlockSEG();
	return ercd;
}

/*
 * Change logical address space attributes
 *	Change logical space address attributes of `npage' pages of 
 *	logical memory to `chg_pte', starting from the page that 
 *	contains logical address `laddr'. 
 *	Only access mode privilege is modified, and other attributes
 *	remain unchanged. `lsid' is ignored.
 *
 *	This implementation forbids specification of non-SDRAM area (E_MACV).
 */
EXPORT ER __ChangeSpace( void *laddr, INT npage, INT lsid, unsigned long chg_pte )
{
const	UW	CHG_MSK = (PT_Writable|PT_User);
	VP	la  = laddr;
	PTE	*pte, pte_v;
	INT	np;
	ER	ercd;

	chg_pte &= CHG_MSK;

	for ( np = 0; np < npage; np++ ) {
		/* Get page table entry */
		pte = GetPTE(la);
		if ( pte == NULL || !(pte->val & PT_Present) ) {
			/* Invalid address region, or the page is invalid */
			ercd = E_MACV;
			goto err_ret;
		}

		/* Update page table entry value */
		pte_v.val = pte->val;
		pte_v.val = (pte_v.val & ~CHG_MSK) | chg_pte;
		pte->val = pte_v.val;
		PurgePageTLB(la, pte);

		la = NextPage(la);	/* Next page */
	}

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("__ChangeSpace ercd = %d\n", ercd));
#endif
	return ercd;
}

EXPORT ER _ChangeSpace( void *laddr, INT npage, INT lsid, INT chg_pte )
{
	ER	ercd;

	LockSEG();
	ercd = __ChangeSpace(laddr, npage, lsid, chg_pte);
	UnlockSEG();
	return ercd;
}

/*
 * Set memory access privilege
 *	Set the memory access permission specified in mode for the len bytes
 *	from addr. Returns the size of memory area that memory access privilege 
 *	has actually been set. 
 *
 *	This implementation forbids specification of non-SDRAM area.
 */
EXPORT INT _SetMemoryAccess( CONST void *addr, INT len, UINT mode )
{
	const void	*la;
	PDE		*pde;
	PTE		*pte, set_pte;
	INT		i, alen;

	if ( len < 0 ) return E_PAR;
#if MM_READ != 0
	if ( !(mode & MM_READ) ) return E_NOSPT;
#endif

	LockSEG();

	for ( la = PageAlignL(addr) ;
		(unsigned long)la < (unsigned long)addr + len; ) {
		pde = GetPDE(la);
		if ( pde == NULL || !(pde->val & PD_Present) ) {
			/* PDE not found, is a section, or set invalid */
			break;
		}

		/* Check the page table entries if page table is defined */
		pte = getFirstPteFromPde(pde);
		//for ( i = PTBL_INDEX(la); i < N_PTE; i++ ) {
		for ( i = PAGE_INDEX(la); i < N_PTE; i++ ) {
			if ( !pte[i].val & PD_Present ) goto ret;

			/* Set PTE value */
			set_pte.val = pte[i].val & PT_Writable;//~(PT_nExecutable|PT_nWritable);
			if ( !(mode & MM_WRITE) )
				set_pte.val |= PT_Writable;
			if ( !(mode & MM_EXECUTE) )
				//set_pte.val |= PT_nExecutable;

			if ( pte[i].val != set_pte.val ) {
				FlushCache(la, PAGESIZE);
				SetPTE(la, set_pte, TRUE);
			}

			/* Next page */
			la = NextPage(la);
			if ((unsigned long)la >= (unsigned long)addr + len) {
				goto ret;
			}
		}
	}
ret:
	UnlockSEG();
	
	if ((unsigned long)la < (unsigned long)addr)
		return 0;

	alen = (unsigned long)la - (unsigned long)addr;
	return (len < alen)? len: alen;
}

/*
 * Get physical address
 *	Converts logical address `laddr' to physical address and 
 *	returns it to *paddr. The number of contiguous bytes 
 *	with respect to physical address is returned. 
 *
 *	This implementation forbids specification of non-SDRAM area.
 *
 *	N.B. Access to *paddr may cause page fault.
 */
EXPORT INT _CnvPhysicalAddr( CONST void *laddr, INT len, void **paddr )
{
	const void	*la;
	PDE		*pde;
	PTE		*pte, set_pte;
	INT		i, alen;

	/* Reject non-RAM area except for some special cases, as it does not 
	   accept cache operations and dirty bit settings, etc. */
	if ( !isRAM(laddr) ) {
		/* ROM area is inherently read-only, thus no problem */
		if ( !isROM(laddr) ) return E_MACV;
	}

	if ( len <= 0 ) {
		return E_PAR;
	}

	LockSEG();

	for ( la = PageAlignL(laddr) ;
		(unsigned long)la < (unsigned long)laddr + len; ) {
		pde = GetPDE(la);
		if ( pde == NULL || !(pde->val & PD_Present)) {
			/* PDE not found, or set invalid */
			break;
		}
		/* Check the page table entries if page table is defined */
		pte = getFirstPteFromPde(pde);
		//for ( i = PTBL_INDEX(la); i < N_PTE; i++ ) {
		for ( i = PAGE_INDEX(la); i < N_PTE; i++ ) {
			if ( !(pte[i].val & PT_Present) ) goto ret;

			/* Disable cache */
			set_pte.val = PTE_CacheOff(pte[i].val);
			if ( pte[i].val != set_pte.val ) {
				FlushCache(la, PAGESIZE);
				SetPTE(la, set_pte, TRUE);
			}

			/* Next page */
			la = NextPage(la);
			if ((unsigned long)la >= ((unsigned long)laddr + len)) {
				goto ret;
			}
		}
	}
ret:
	UnlockSEG();
	
	if ((unsigned long)la < (unsigned long)laddr)
		return 0;

	alen = (unsigned long)la - (unsigned long)laddr;
	*paddr = (void*)getPtePfaFromLaddr((unsigned long)laddr);
	return (len < alen)? len: alen;
}

/*
 * Checks access privileges of logical space
 *	Checks whether the `len' bytes of logical space starting from address `laddr'
 *	is accessible with `mode' from the execution environment `env'.
 *	E_OK is returned if possible, E_MACV otherwise.
 *	mode = [MA_READ] | [MA_WRITE] | [MA_EXECUTE]
 */
EXPORT ER _ChkSpace( CONST void *laddr, INT len, UINT mode, UINT env )
{
	INT alen = _ChkSpaceLen(laddr, len, mode, env, 0);

	if ( alen < E_OK ) {
		/* If _ChkSpaceLen returned error, return the error as it is */
		return alen;
	}

	return ( alen >= len )? E_OK: E_MACV;
}

/*
 * Checks access privileges of logical space
 *	Checks whether the maximum of `len' bytes of logical space starting from 
 *	address `laddr' is accessible with `mode' from the execution environment `env'.
 *	`lsid' is ignored.
 *	mode = [MA_READ] | [MA_WRITE] | [MA_EXECUTE]
 *	mode = 0 means memory existence check.
 *	The number of bytes accessible is returned.
 *	Return value	0-len	the number of bytes accessible
 *			< 0	error code
 */
EXPORT INT _ChkSpaceLen( CONST void *laddr, INT len, UINT mode, UINT env, INT lsid )
{
	const void	*la;
	PDE		*pde;
	PTE		*pte;
	//BOOL		chk_u, chk_w, chk_x;
	BOOL		chk_u, chk_w;
	INT		i, alen;

	if ( len < 0 ) {
		return E_PAR;
	}

	/* Requested capabilities */
	chk_u = ( (env & TMF_PPL(3)) >= TMF_PPL(MMU_MIN_USER_LEVEL) );
	chk_w = ( (mode & MA_WRITE)   != 0 );
	//chk_x = ( (mode & MA_EXECUTE) != 0 );

	LockSEG();

	for ( la = PageAlignL(laddr) ; (unsigned long)la < (unsigned long)laddr + len; ) {
		pde = GetPDE(la);
		if ( pde == NULL || !(pde->val & PD_Present)) {
			/* PDE not found, or set invalid */
			break;
		}
		/* Check the page table entries if page table is defined */
		pte = getFirstPteFromPde(pde);

		//for ( i = PTBL_INDEX(la); i < N_PTE; i++ ) {
		for ( i = PAGE_INDEX(la); i < N_PTE; i++ ) {
#if 1
			if ( !(pte[i].val & PT_Present) ) goto ret;
			if ( chk_u && !(pte[i].val & PT_User) ) goto ret;
			if ( chk_w && !(pte[i].val & PT_Writable) ) goto ret;
#endif
#if 0
			if ( !(pte[i].val & PT_Present) ) {printf("not present:0x%08X ", pte[i].val);printf("addr:0x%08X\n", la);goto ret;}
			if ( chk_u && !(pte[i].val & PT_User) ) {printf("not user page:0x%08X\n", pte[i].val);goto ret;}
			if ( chk_w && !(pte[i].val & PT_Writable) ) {printf("not writable:0x%08X\n", pte[i].val);goto ret;}
#endif
			//if ( chk_x && pte[i].a.xn ) goto ret;

			/* Next page */
			la = NextPage(la);
			if ( (unsigned long)la >= (unsigned long)laddr + len ) goto ret;
		}
	}
ret:
	UnlockSEG();
	
	if ((unsigned long)la < (unsigned long)laddr) 
		return 0;

	alen = (unsigned long)la - (unsigned long)laddr;

	return (len < alen)? len: alen;
}

/*
 * Checks access privileges of logical space (TC string) 
 *	Checks whether the logical address starting from `str' is accessible 
 *	with `mode' from the execution environment `env', until TNULL is found 
 *	or `max' characters is reached. If max = 0, the number of characters (max)
 *	is ignored.
 *	If accessible, the number of characters or `max' (if max > 0 and TNULL 
 *	is not found before `max' bytes) is returned. Returns E_MACV otherwise.
 *	mode = [MA_READ] | [MA_WRITE]
 */
EXPORT INT _ChkSpaceTstr( CONST TC *str, INT max, UINT mode, UINT env )
{
	const void	*la = PageAlignL(str);
	const TC	*tp = str;
	PDE		*pde;
	PTE		*pte;
	BOOL		chk_u, chk_w;
	INT		i;
	ER		ret = E_MACV;

	/* Requested capabilities */
	chk_u  = ( (env & TMF_PPL(3)) >= TMF_PPL(MMU_MIN_USER_LEVEL) );
	chk_w = ( (mode & MA_WRITE)   != 0 );

	LockSEG();

	while ( max <= 0 || (const TC*)la < str + max ) {
		pde = GetPDE(la);
		if ( pde == NULL || !(pde->val & PD_Present)) {
			/* PDE not found, or set invalid */
			break;
		}

		/* Check the page table entries if page table is defined */
		pte = getFirstPteFromPde(pde);
		//for ( i = PTBL_INDEX(la); i < N_PTE; i++ ) {
		for ( i = PAGE_INDEX(la); i < N_PTE; i++ ) {
			if ( !(pte[i].val & PT_Present) ) goto ret;
			if ( chk_u && !(pte[i].val & PT_User) ) goto ret;
			if ( chk_w && !(pte[i].val & PT_Writable) ) goto ret;

			/* Check if TNULL exists before the next page */
			la = NextPage(la);
			for ( ; tp < (const TC*)la; tp++ ) {
				if ( *tp == TNULL ) {
					ret = tp - str;
					if ( max > 0 && ret > max )
						ret = max;
					goto ret;
				}
			}
		}
	}
ret:
	UnlockSEG();

	return ret;
}

/*
 * Checks access privileges of logical space (UB string)
 *	Checks whether the logical address starting from `str' is accessible 
 *	with `mode' from the execution environment `env', until '\0' is found 
 *	or `max' characters is reached. If max = 0, the number of characters (max)
 *	is ignored.
 *	If accessible, the number of characters or `max' (if max > 0 and '\0' 
 *	is not found before `max' bytes) is returned. Returns E_MACV otherwise.
 *	mode = [MA_READ] | [MA_WRITE]
 */
EXPORT INT _ChkSpaceBstr( CONST UB *str, INT max, UINT mode, UINT env )
{
	const void	*la = PageAlignL(str);
	const UB	*tp = str;
	PDE		*pde;
	PTE		*pte;
	BOOL		chk_u, chk_w;
	INT		i;
	ER		ret = E_MACV;

	/* Requested capabilities */
	chk_u  = ( (env & TMF_PPL(3)) >= TMF_PPL(MMU_MIN_USER_LEVEL) );
	chk_w = ( (mode & MA_WRITE)   != 0 );

	LockSEG();

	while ( max <= 0 || (const UB*)la < str + max ) {
		pde = GetPDE(la);
		if ( pde == NULL || !(pde->val & PD_Present)) {
			/* PDE not found, or set invalid */
			break;
		}

		/* Check the page table entries if page table is defined */
		pte = getFirstPteFromPde(pde);
		//for ( i = PTBL_INDEX(la); i < N_PTE; i++ ) {
		for ( i = PAGE_INDEX(la); i < N_PTE; i++ ) {
			if ( !(pte[i].val & PT_Present) ) goto ret;
			if ( chk_u && !(pte[i].val & PT_User) ) goto ret;
			if ( chk_w && !(pte[i].val & PT_Writable) ) goto ret;

			/* Check if '\0' exists before the next page */
			la = NextPage(la);
			for ( ; tp < (const UB*)la; tp++ ) {
				if ( *tp == '\0' ) {
					ret = tp - str;
					if ( max > 0 && ret > max )
						ret = max;
					goto ret;
				}
			}
		}
	}
ret:
	UnlockSEG();

	return ret;
}

/*
 * Initialize logical address space
 */
EXPORT ER InitLogicalSpace( void )
{
	UINT	npg_ram;
	unsigned long memend;
	unsigned long paddr;
	void *laddr;
	PDE *pde;
	PTE *pte;
	int err;

	struct boot_info *info = getBootInfo();

	SysRamLimit = info->lowmem_limit;

	/* -------------------------------------------------------------------- */
	/* set up global segment descriptor					*/
	/* -------------------------------------------------------------------- */
	initGdt();
	
	vd_printf("info->lowmem_top(InitLogicalSpace):0x%08X\n", info->lowmem_top);

	/* -------------------------------------------------------------------- */
	/* allocate memory for pde						*/
	/* -------------------------------------------------------------------- */
	SysPDEBase =
		(PDE*)PageAlignU((const void*)toLogicalAddress(info->lowmem_top));

	memend = (unsigned long)toPhysicalAddress(SysPDEBase)
					+ sizeof(PDE) * NUM_PDIR;

	if (info->lowmem_limit < memend) {
		return(E_NOMEM);
	}

	memset((void*)SysPDEBase, 0x00, sizeof(PDE) * NUM_PDIR);

	/* -------------------------------------------------------------------- */
	/* allocate memory for page tables					*/
	/* -------------------------------------------------------------------- */
	npg_ram = PTBL_NUM(SysRamLimit);

	/* Allocate memory for page tables */
	SysRamPTEBase = (PTE*)PageTableAlignU(toLogicalAddress(memend));
	memend = (unsigned long)toPhysicalAddress(PageAlignU(SysRamPTEBase + npg_ram));

	if (info->lowmem_limit < memend) {
		return(E_NOMEM);
	}
	
	memset((void*)SysRamPTEBase, 0x00, sizeof(PTE) * npg_ram);
	
	info->lowmem_top = memend;
	vd_printf("info->lowmem_top(InitLogicalSpace):0x%08X\n", memend);

	/* -------------------------------------------------------------------- */
	/* set up pdes								*/
	/* -------------------------------------------------------------------- */
	for (laddr = (void*)KERNEL_BASE_ADDR,
		paddr =  (unsigned long)PageAlignU(REALMEMORY_TOP);
		paddr <= (REALMEMORY_TOP + SysRamLimit) ;
		paddr += PDIRSIZE, laddr = (void*)((char*)laddr + PDIRSIZE)) {
		pde = SysPDEBase + PDIR_INDEX(laddr);
		pte = GetPTE(laddr);
		pde->val = ((unsigned long)pte - KERNEL_BASE_ADDR) & PAGE_MASK;
		pde->val |= PDE_SYS_RW;
	}

	/* -------------------------------------------------------------------- */
	/* set up ptes								*/
	/* -------------------------------------------------------------------- */
	for (laddr = (void*)KERNEL_BASE_ADDR,
		paddr = (unsigned long)PageAlignU(REALMEMORY_TOP);
		paddr < (REALMEMORY_TOP + SysRamLimit) ;
		paddr += PAGESIZE, laddr = (void*)((char*)laddr + PAGESIZE)) {
		pte = GetPTE(laddr);
		
		pte->val = (paddr & PAGE_MASK) | PTE_SYS_RW;
	}
	
	/* -------------------------------------------------------------------- */
	/* load page directory base to cr3 register				*/
	/* -------------------------------------------------------------------- */
	loadPdbr((unsigned long)SysPDEBase - KERNEL_BASE_ADDR);
	ASM (
	"jmp	flush_tlb_after_load_pdbr	\n\t"
	"flush_tlb_after_load_pdbr:"
	);
	/* after load to cr3, tlb is flushed					*/
	/* -------------------------------------------------------------------- */
	/* set up kernel tss							*/
	/* -------------------------------------------------------------------- */
	err = initKernelTss();
	
	if (err) {
		vd_printf("error:set up kernel tss\n");
	}
	
	return E_OK;
}

/* ------------------------------------------------------------------------ */
/*
 *	TLB control
 */

/*
 * Write back page table entry
 *	cache line size >= PDE size (16 bytes) is assumed
 */
EXPORT void WriteBackPageTable( const void *ptbl )
{
	unsigned long ptbl_addr = (unsigned long)ptbl;
	ASM ("invlpg %[ptbl]	\n\t" ::[ptbl]"m"(ptbl_addr):);
	DSB();
}

/*
 * Purge TLB page entry containing logical address `laddr'
 *	Specify to `pte' the page table entry address to purge. 
 *	Page table entry is written back to the main memory.
 */
EXPORT void PurgePageTLB( const void *laddr, PTE *pte )
{
	struct cpu_info *info;

	info = getCpuInfo();

	if ( info->cpu_type < CPU_TYPE_80486 )
	{
		PurgeTLB();
	} else {
		WriteBackPageTable(laddr);
	}
	
	DSB(); ISB();
}

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
EXPORT unsigned long getPtePfaFromLaddr(unsigned long laddr)
{
	return(GetPTE((const void*)laddr)->val & PAGE_MASK);
}

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
EXPORT unsigned long get_system_pde(void)
{
	return((unsigned long)SysPDEBase);
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:getFirstPteFromPde
 Input		:unsigned long laddr
 		 < logical address >
 Output		:void
 Return		:unsigned long
 		 < page frame address of the firts pte of the page table >
 Description	:translate pde to first pte of page table
==================================================================================
*/
LOCAL INLINE PTE* getFirstPteFromPde(PDE *pde)
{
	unsigned long paddr;
	unsigned long laddr;

	paddr = (pde->val & PAGE_MASK);
	laddr =  paddr + KERNEL_BASE_ADDR;
	
	return((PTE*)laddr);
}
