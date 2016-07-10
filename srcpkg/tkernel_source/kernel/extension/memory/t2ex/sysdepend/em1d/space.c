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
#include "sysmgr.h"
#include "segmgr.h"
#include "mmu.h"
#include "pagedef.h"
#include "memdef.h"
#include <sys/rominfo.h>
#include <sys/segment.h>
#include <tk/cpu_conf.h>

/*
 * Base address for page directory entries
 *
 *	SysPDEBase[i] is assigned as the PDE for logical address (i << SECTIONSIZE)
 */
LOCAL	PDE	*SysPDEBase;

/*
 * Base address for SDRAM page table entries
 *
 *	SysRamPTEBase[i] is assigned as the PTE for logical address (REALMEMORY_TOP + (i << PAGESIZE))
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
	return SysPDEBase + PDIR_NUM(laddr);
}

/*
 * Get page table entry
 */
Inline PTE* GetPTE( const void *laddr )
{
	/* All the Non-SDRAM areas are controlled under 1MB sections, thus have no PTE */
	if ((UW)laddr < REALMEMORY_TOP || (UW)laddr >= SysRamLimit) {
		return NULL;
	}

	return SysRamPTEBase + PRAMTBL_NUM(laddr);
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
	Asm("mcr p15, 0, %0, cr7, c14, 0":: "r"(0));
	/* Invalidate entire instruction cache */
	Asm("mcr p15, 0, %0, cr7, c5, 0":: "r"(0));
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
		if ( pte == NULL || pte->a.p ) {
			/* Invalid address region, or the page is made already */
			ercd = E_MACV;
			goto err_ret;
		}

		/* Update page table entry value */
		pte_v.w = set_pte;
		pte_v.a.pfa = LADRtoPFA(la);
		SetPTE(la, pte_v, TRUE);

		la = NextPage(la);	/* Next page */
	}

	return E_OK;

err_ret:
	__UnmakeSpace(laddr, np, lsid);
	TM_DEBUG_PRINT(("__MakeSpace ercd = %d\n", ercd));
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
		if ( !pte->a.p ) {
			la = NextPage(la);
			continue;
		}

		/* Update page table entry value */
		pte_v.w = PTE_NONE;
		SetPTE(la, pte_v, TRUE);

		la = NextPage(la);	/* Next page */
	}

	return E_OK;

err_ret:
	TM_DEBUG_PRINT(("__UnmakeSpace ercd = %d\n", ercd));
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
EXPORT ER __ChangeSpace( void *laddr, INT npage, INT lsid, INT chg_pte )
{
const	UW	CHG_MSK = (PT_nWritable|PT_User|PT_nExecutable);
	VP	la  = laddr;
	PTE	*pte, pte_v;
	INT	np;
	ER	ercd;

	chg_pte &= CHG_MSK;

	for ( np = 0; np < npage; np++ ) {
		/* Get page table entry */
		pte = GetPTE(la);
		if ( pte == NULL || !pte->a.p ) {
			/* Invalid address region, or the page is invalid */
			ercd = E_MACV;
			goto err_ret;
		}

		/* Update page table entry value */
		pte_v = *pte;
		pte_v.w = (pte_v.w & ~CHG_MSK) | chg_pte;
		*pte = pte_v;
		PurgePageTLB(la, pte);

		la = NextPage(la);	/* Next page */
	}

	return E_OK;

err_ret:
	TM_DEBUG_PRINT(("__ChangeSpace ercd = %d\n", ercd));
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

	for ( la = PageAlignL(addr) ; (UW)la < (UW)addr + len; ) {
		pde = GetPDE(la);
		if ( pde == NULL || pde->c.s || (pde->c.p == 0 && pde->c.s == 0) ) {
			/* PDE not found, is a section, or set invalid */
			break;
		}

		/* Check the page table entries if page table is defined */
		pte = DPFAtoLADR(pde->c.pfa);
		for ( i = PTBL_NUM(la); i < N_PTE; i++ ) {
			if ( !pte[i].a.p ) goto ret;

			/* Set PTE value */
			set_pte.w = pte[i].w & ~(PT_nExecutable|PT_nWritable);
			if ( !(mode & MM_WRITE) )
				set_pte.w |= PT_nWritable;
			if ( !(mode & MM_EXECUTE) )
				set_pte.w |= PT_nExecutable;

			if ( pte[i].w != set_pte.w ) {
				FlushCache(la, PAGESIZE);
				SetPTE(la, set_pte, TRUE);
			}

			/* Next page */
			la = NextPage(la);
			if ( (UW)la >= (UW)addr + len ) goto ret;
		}
	}
ret:
	UnlockSEG();
	
	if ((UW)la < (UW)addr)
		return 0;

	alen = (UW)la - (UW)addr;
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

	for ( la = PageAlignL(laddr) ; (UW)la < (UW)laddr + len; ) {
		pde = GetPDE(la);
		if ( pde == NULL || (pde->c.p == 0 && pde->c.s == 0)) {
			/* PDE not found, or set invalid */
			break;
		}

		if ( pde->c.s ) {
			/* Assumes here that RAM area is not assigned to sections.
			   If not, it is needed to disable cache for that memory */
			;

			/* Next section */
			la = NextSection(SectionAlignL(la));
		}
		else {
			/* Check the page table entries if page table is defined */
			pte = DPFAtoLADR(pde->c.pfa);
			for ( i = PTBL_NUM(la); i < N_PTE; i++ ) {
				if ( !pte[i].a.p ) goto ret;

				/* Disable cache */
				set_pte.w = PTE_CacheOff(pte[i].w);
				if ( pte[i].w != set_pte.w ) {
					FlushCache(la, PAGESIZE);
					SetPTE(la, set_pte, TRUE);
				}

				/* Next page */
				la = NextPage(la);
				if ( (UW)la >= (UW)laddr + len ) goto ret;
			}
		}
	}
ret:
	UnlockSEG();
	
	if ((UW)la < (UW)laddr)
		return 0;

	alen = (UW)la - (UW)laddr;
	*paddr = toPhysicalAddress(laddr);
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
	BOOL		chk_u, chk_w, chk_x;
	INT		i, alen;

	if ( len < 0 ) {
		return E_PAR;
	}

	/* Requested capabilities */
	chk_u  = ( (env & TMF_PPL(3)) >= TMF_PPL(MMU_MIN_USER_LEVEL) );
	chk_w = ( (mode & MA_WRITE)   != 0 );
	chk_x = ( (mode & MA_EXECUTE) != 0 );

	LockSEG();

	for ( la = PageAlignL(laddr) ; (UW)la < (UW)laddr + len; ) {
		pde = GetPDE(la);
		if ( pde == NULL || (pde->c.p == 0 && pde->c.s == 0)) {
			/* PDE not found, or set invalid */
			break;
		}

		if ( pde->c.s ) {
			/* Check PDE attributes directly if section is defined */
			if ( chk_u && !pde->s.u ) break;
			if ( chk_w && pde->s.nw ) break;
			if ( chk_x && pde->s.xn ) break;

			/* Next section */
			la = NextSection(SectionAlignL(la));
		}
		else {
			/* Check the page table entries if page table is defined */
			pte = DPFAtoLADR(pde->c.pfa);
			for ( i = PTBL_NUM(la); i < N_PTE; i++ ) {
				if ( !pte[i].a.p ) goto ret;
				if ( chk_u && !pte[i].a.u ) goto ret;
				if ( chk_w && pte[i].a.nw ) goto ret;
				if ( chk_x && pte[i].a.xn ) goto ret;

				/* Next page */
				la = NextPage(la);
				if ( (UW)la >= (UW)laddr + len ) goto ret;
			}
		}
	}
ret:
	UnlockSEG();
	
	if ((UW)la < (UW)laddr)
		return 0;

	alen = (UW)la - (UW)laddr;
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
		if ( pde == NULL || (pde->c.p == 0 && pde->c.s == 0)) {
			/* PDE not found, or set invalid */
			break;
		}

		if ( pde->c.s ) {
			/* Check PDE attributes directly if section is defined */
			if ( chk_u && !pde->s.u ) break;
			if ( chk_w && pde->s.nw ) break;

			/* Check if TNULL exists before the next section */
			la = NextSection(SectionAlignL(la));
			for ( ; tp < (const TC*)la; tp++ ) {
				if ( *tp == TNULL ) {
					ret = tp - str;
					if ( max > 0 && ret > max )
						ret = max;
					goto ret;
				}
			}
		}
		else {
			/* Check the page table entries if page table is defined */
			pte = DPFAtoLADR(pde->c.pfa);
			for ( i = PTBL_NUM(la); i < N_PTE; i++ ) {
				if ( !pte[i].a.p ) goto ret;
				if ( chk_u && !pte[i].a.u ) goto ret;
				if ( chk_w && pte[i].a.nw ) goto ret;

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
		if ( pde == NULL || (pde->c.p == 0 && pde->c.s == 0)) {
			/* PDE not found, or set invalid */
			break;
		}

		if ( pde->c.s ) {
			/* Check PDE attributes directly if section is defined */
			if ( chk_u && !pde->s.u ) break;
			if ( chk_w && pde->s.nw ) break;

			/* Check if '\0' exists before the next section */
			la = NextSection(SectionAlignL(la));
			for ( ; tp < (const UB*)la; tp++ ) {
				if ( *tp == '\0' ) {
					ret = tp - str;
					if ( max > 0 && ret > max )
						ret = max;
					goto ret;
				}
			}
		}
		else {
			/* Check the page table entries if page table is defined */
			pte = DPFAtoLADR(pde->c.pfa);
			for ( i = PTBL_NUM(la); i < N_PTE; i++ ) {
				if ( !pte[i].a.p ) goto ret;
				if ( chk_u && !pte[i].a.u ) goto ret;
				if ( chk_w && pte[i].a.nw ) goto ret;

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
	}
ret:
	UnlockSEG();

	return ret;
}

/* ------------------------------------------------------------------------ */

/*
 * Section boundaries
 */
IMPORT INT __stext;
IMPORT INT __erodata_usr;
IMPORT INT __data_start;
IMPORT INT _end;
IMPORT INT __data_usr_start;
IMPORT INT _end_usr;
IMPORT INT _end_bss;

/*
 * Initialize logical address space
 */
EXPORT ER InitLogicalSpace( void )
{
/* Low-level memory management information */
IMPORT	void	*lowmem_top, *lowmem_limit;
	ER	ercd;
	UW	cr;
	UINT	npd_ram, npg_ram, i;
	PTE	pte;
	PDE	pde;
	void	*memend, *la;

	/* Get system page table
	 *	MMU is already enabled by T-Monitor at this point.
	 */
	Asm("mrc p15, 0, %0, cr2, c0, 1": "=r"(cr)); /* TTBR1 */
	SysPDEBase = (PDE*)(cr & 0xffffc000);

	/* Get the physical memory address boundary managed by the segment manager */
	ercd = _tk_get_cfn(SCTAG_REALMEMEND, (INT*)&SysRamLimit, 1);
	if ( ercd < 1 || SysRamLimit > (UINT)lowmem_limit ) {
		SysRamLimit = (UINT)lowmem_limit;
	}

	/* Calculate the number of pages for SDRAM */
	npg_ram = PRAMTBL_NUM(SysRamLimit);
	npd_ram = (npg_ram + (N_PTE-1)) / N_PTE;

	/* Allocate memory for page tables */
	SysRamPTEBase = (PTE*)PageTableAlignU(lowmem_top);
	memend = PageAlignU(SysRamPTEBase + npg_ram);

	/* Initialize PTE for SDRAM space */
	la = PageAlignL((void*)REALMEMORY_TOP);

	if ( isRAM(&__stext) ) {
		/* Initialize RAM kernel specific space */
		for ( ; (UW)la < (UW)&__stext; la = NextPage(la) ) {
			/* Header */
			pte.w = PTE_SYS_RW;
			pte.a.pfa = LADRtoPFA(la);
			SetPTE(la, pte, FALSE);
		}

		for ( ; (UW)la < (UW)&__erodata_usr; la = NextPage(la) ) {
			/* Executable and read-only data (__stext ... __erodata) */
			pte.w = PTE_USR_EXEC;
			pte.a.pfa = LADRtoPFA(la);
			SetPTE(la, pte, FALSE);
		}
	}

	for ( ; (UW)la < (UW)&_end; la = NextPage(la) ) {
		/* System data area (__data_start ... _end) */
		pte.w = PTE_SYS_RW;
		pte.a.pfa = LADRtoPFA(la);
		SetPTE(la, pte, FALSE);
	}

	for ( ; (UW)la < (UW)&_end_usr; la = NextPage(la) ) {
		/* User data area (__data_usr_start ... _end_usr) */
		pte.w = PTE_USR_RW;
		pte.a.pfa = LADRtoPFA(la);
		SetPTE(la, pte, FALSE);
	}

	for ( ; (UW)la < (UW)memend; la = NextPage(la) ) {
		/* Dynamically allocated memory area (including PTE) */
		pte.w = PTE_SYS_RW;
		pte.a.pfa = LADRtoPFA(la);
		SetPTE(la, pte, FALSE);
	}

	for ( ; (UW)la < (UW)SysRamLimit; la = NextPage(la) ) {
		/* Unallocated area */
		pte.w = PTE_NONE;
		SetPTE(la, pte, FALSE);
	}

	/* Initialize PDE for SDRAM space */
	la = SectionAlignL((void*)REALMEMORY_TOP);

	for ( i = 0; i < npd_ram; i++, la = NextSection(la) ) {
		pde.w = PDE_NORM;
		pde.c.pfa = LADRtoDPFA(SysRamPTEBase + i * N_PTE);
		SetPDE(la, pde);
	}

	lowmem_top = memend;  /* Update memory free space */

	/* Modify PDE for ROM area */
	if ( isROM(&__stext) ) {
		/* Modify ROM kernel spacific space */
		la = SectionAlignL((void*)&__stext);

		for ( ; (UW)la < (UW)&__erodata_usr; la = NextSection(la) ) {
			/* Allow user access (__stext ... __erodata) */
			pde = *GetPDE(la);
			pde.s.u = 1;
			SetPDE(la, pde);
		}
	}

	/* Purge all entries in TLB */
	FlushAllCache();
	PurgeTLB();

	/* Configure access domain */
	cr = 0x55555555;
	Asm("mcr p15, 0, %0, cr3, c0":: "r"(cr));

	/*
	 * MMU and cache mode configuration
	 *	Should have been set in T-Monitor, but redo it here to clarify 
	 *	MMU configuration requirements of this segment manager.
	 */
	Asm("mrc p15, 0, %0, cr1, c0, 0": "=r"(cr));	/* SCTLR */
	cr = (cr & ~SCTLR_OFF) | SCTLR_ON;
	Asm("mcr p15, 0, %0, cr1, c0, 0":: "r"(cr));	/* SCTLR */
	ISB();

	return E_OK;
}

