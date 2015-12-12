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

#include <bk/memory/page.h>
#include <bk/uapi/berrno.h>
#include <typedef.h>
#include <tstdlib/bitop.h>
#include <sys/rominfo.h>
#include <sys/sysinfo.h>
//#include <sys/sysdepend/std_x86/multiboot.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL void init_page_allocator(void);
LOCAL INLINE void _free_pagetables(pde_t *pde, unsigned long end);


/*
==================================================================================

	DEFINE 

==================================================================================
*/

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct page *pages;
LOCAL long nr_pages;
LOCAL unsigned long *page_bitmap;
LOCAL long page_bitmap_size;
LOCAL long page_bitmap_bit_size;
LOCAL long nr_end_page;
LOCAL long nr_free_pages;


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_memmgr
 Input		:void
 Output		:void
 Return		:ER
 		 < error code >
 Description	:initialize page frame management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER init_memmgr(void)
{
	struct boot_info *info = getBootInfo();
	unsigned long lowmem_end = info->lowmem_base + info->lowmem_limit;
	unsigned long memend;
	ER ercd;
	
	/* -------------------------------------------------------------------- */
	/* acquire system configuration						*/
	/* -------------------------------------------------------------------- */
	ercd = _tk_get_cfn(SCTAG_REALMEMEND, (INT*)&memend, 1);
	if ( ercd < 1 || memend > lowmem_end ) {
		memend = lowmem_end;
	} else if (memend == ~(-1UL)) {
		/* use boot information				*/
		memend = lowmem_end;
	}
	
	/* -------------------------------------------------------------------- */
	/* initailize page frame allocator					*/
	/* -------------------------------------------------------------------- */
	init_page_allocator();

	return E_OK;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:page_to_address
 Input		:struct page *page
 		 < page information to get its address >
 Output		:void
 Return		:unsigned long
 		 < virtual address to which page attributes >
 Description	:get page address
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT unsigned long page_to_address(struct page *page)
{
	unsigned long index = page - &pages[0];
	
	return((unsigned long)toLogicalAddress(index << PAGE_SHIFT));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:page_to_paddr
 Input		:struct page *page
 		 < page information to get its physical address >
 Output		:void
 Return		:unsigned long
 		 < physical address to which page attributes >
 Description	:get page physical address
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT unsigned long page_to_paddr(struct page *page)
{
	unsigned long index = page - &pages[0];
	
	return((unsigned long)(index << PAGE_SHIFT));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_pages
 Input		:int num
 		 < number of pages to allocate >
 Output		:void
 Return		:struct page*
 		 < page information >
 Description	:allocate consecutive pages from page allocator
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct page* alloc_pages(int num)
{
	long index;
	int i;
	
	BEGIN_CRITICAL_SECTION;
	index = tstdlib_bitsearch0_window((void*)page_bitmap,
						page_bitmap_bit_size, num);
	tstdlib_bitset_window((void*)page_bitmap, index, num);
	END_CRITICAL_SECTION;
	
	if (0 <= index) {
		for (i = 0;i < num;i++) {
			pages[index + i].count++;
		}
		nr_free_pages -= num;
		
		return(&pages[index]);
	}
	return(NULL);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_zeroed_page
 Input		:void
 Output		:void
 Return		:struct page*
 		 < page information >
 Description	:allocate a zero-cleared page
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct page* alloc_zeroed_page(void)
{
	struct page *page = alloc_page();
	
	if (page) {
		memset((void*)page_to_address(page), 0x00, PAGESIZE);
	}
	
	return(page);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_slab_pages
 Input		:int num
 		 < number of pages to allocate for a slab >
 Output		:void
 Return		:void
 Description	:allocate consecutive pages for a slab
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct page* alloc_slab_pages(int num)
{
	int i;
	struct page *page = alloc_pages(num);
	
	if (page) {
		for (i = 0;i < num;i++) {
			page[i].flags = PAGE_SLAB;
		}
	}
	
	return(page);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_page
 Input		:struct page *page
 		 < page to free >
 Output		:void
 Return		:void
 Description	:free a pages
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_page(struct page *page)
{
	long index;
	
	index = page - &pages[0];
	
	BEGIN_CRITICAL_SECTION {
		tstdlib_bitclr((void*)page_bitmap, index);
		page->flags = PAGE_RESERVED;
		nr_free_pages++;
	} END_CRITICAL_SECTION;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pages
 Input		:struct page *page
 		 < first consecutive page >
 		 int num
 		 < number of pages to free >
 Output		:void
 Return		:void
 Description	:free consecutive pages
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_pages(struct page *page, int num)
{
	int i;
	
	if (!page || (num < 0)) {
		return;
	}
	
	BEGIN_CRITICAL_SECTION {
		for (i = 0;i < num ;i++) {
			if (page->count) {
				page->count--;
			}
			
			if (!page->count) {
				free_page(page++);
			}
		}
	} END_CRITICAL_SECTION;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_page
 Input		:void *address
 		 < address of object >
 Output		:void
 Return		:struct page*
 		 < page to which address belongs >
 Description	:get a page to which address belongs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct page* get_page(void *address)
{
	int index;
	index = ((unsigned long)address & PAGE_MASK) - KERNEL_BASE_ADDR;
	index >>= PAGE_SHIFT;
	
	return(&pages[index]);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_pagetable
 Input		:void
 Output		:void
 Return		:pte_t*
 		 < page table address >
 Description	:allocate a page for a page talbe from page allocator
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pte_t* alloc_pagetable(void)
{
	struct page *page = alloc_page();
	
	if (!page) {
		return(NULL);
	}
	
	return((pte_t*)page_to_address(page));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_pagedir
 Input		:void
 Output		:void
 Return		:pde_t*
 		 < page directory address >
 Description	:allocate a page for a page directory from page allocator
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pde_t* alloc_pagedir(void)
{
	struct page *page = alloc_page();
	
	if (!page) {
		return(NULL);
	}
	
	return((pde_t*)page_to_address(page));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pagetable
 Input		:pte_t *pte
 		 < address of page table to free >
 Output		:void
 Return		:void
 Description	:free page table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_pagetable(pte_t *pte)
{
	struct page *page = get_page((void*)pte);
	
	free_page(page);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pagetables_all
 Input		:pde_t *pde
 		 < address of page direcotry to free page talbes >
 Output		:void
 Return		:void
 Description	:free all page talbes
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_pagetables_all(pde_t *pde)
{
	/* -------------------------------------------------------------------- */
	/* free page tables for a user space					*/
	/* -------------------------------------------------------------------- */
	_free_pagetables(pde, (~0UL));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pagetables
 Input		:pde_t *pde
 		 < address of page direcotry to free page talbes >
 Output		:void
 Return		:void
 Description	:free page talbes only for user space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_pagetables(pde_t *pde)
{
	/* -------------------------------------------------------------------- */
	/* free page tables for a user space					*/
	/* -------------------------------------------------------------------- */
	_free_pagetables(pde, KERNEL_BASE_ADDR);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pagedir
 Input		:pde_t *pde
 		 < page directory to free >
 Output		:void
 Return		:void
 Description	:free page directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_pagedir(pde_t *pde)
{
	struct page *page;
	
	if (!pde) {
		return;
	}
	
	page = get_page((void*)pde);
	
	if (!page) {
		return;
	}
	
	free_page(page);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pagedir_tables
 Input		:pde_t *pde
 		 < page directory to free >
 Output		:void
 Return		:void
 Description	:free page directory and page tables
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_pagedir_tables(pde_t *pde)
{
	struct page *dir_page;
	
	if (!pde) {
		return;
	}
	
	dir_page = get_page((void*)pde);
	
	if (!dir_page) {
		return;
	}
	
	free_pagetables_all(pde);
	
	free_page(dir_page);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_page_pde
 Input		:struct page *page
 		 < page infromation >
 		 pde_t *pde
 		 < page directory >
 Output		:void
 Return		:pde_t*
 		 < page directory entry which struct page represents >
 Description	:get page directory entry from page struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pde_t* get_page_pde(struct page *page, pde_t *pde)
{
	unsigned long index = page - &page[0];
	unsigned long pde_index = index >> PDIR_INDEX_SHIFT;
	
	return(pde + pde_index);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_page_pte
 Input		:struct page *page
 		 < page infromation >
 		 pde_t *pde
 		 < page directory >
 Output		:void
 Return		:pte_t*
 		 < page table entry which struct page represents >
 Description	:get page table entry from page struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pte_t* get_page_pte(struct page *page, pde_t *pde)
{
	pde_t *page_pde = get_page_pde(page, pde);
	
	pte_t *pte = (pte_t*)toLogicalAddress(*(page_pde) & PAGE_MASK);
	
	return(pte);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_pagetable
 Input		:struct process *from
 		 < copy from >
 		 struct process *to
 		 < copy to >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy pagetables
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int copy_pagetable(struct process *from, struct process *to)
{
	int err;
	int nr;
	int nr_page;
	pde_t *old_pde = from->mspace->pde;
	pde_t *new_pde;
	
	/* -------------------------------------------------------------------- */
	/* allocate new page directory						*/
	/* -------------------------------------------------------------------- */
	new_pde = alloc_pagedir();
	
	if (!new_pde) {
		err = -ENOMEM;
		goto err_alloc_pagedir;
	}
	
	/* -------------------------------------------------------------------- */
	/* copy old pde to new pde for user space				*/
	/* -------------------------------------------------------------------- */
	for (nr = 0;nr < PDIR_INDEX(KERNEL_BASE_ADDR);nr++) {
		pte_t *old_pte;
		pte_t *new_pte;
		
		*(new_pde + nr) = *(old_pde + nr);
		
		if (!(*(old_pde + nr) & PAGE_MASK)) {
			continue;
		}
		
		old_pte = (pte_t*)toLogicalAddress(*(old_pde + nr) & PAGE_MASK);
		
		new_pte = alloc_pagetable();
		
		if (!new_pte) {
			err = -ENOMEM;
			goto err_alloc_pagetable;
		}
		
		for (nr_page = 0;nr_page < PT_ENTRIES;nr_page++) {
			*(new_pte + nr_page) = *(old_pte + nr_page);
		}
		
		*(new_pde + nr) =
			(pde_t)(page_to_paddr(get_page(new_pte)) & PAGE_MASK);
	}
	
	/* -------------------------------------------------------------------- */
	/* copy old pde to new pde for kernel space				*/
	/* -------------------------------------------------------------------- */
	for (nr = PDIR_INDEX(KERNEL_BASE_ADDR);nr < PDIR_ENTRIES;nr++) {
		*(new_pde + nr) = *(old_pde + nr);
		/* ------------------------------------------------------------ */
		/* there is no need to copy kernel page talbes			*/
		/* ------------------------------------------------------------ */
		/* do nothing							*/
	}
	
	to->mspace->pde = new_pde;
	
	return(0);
	
err_alloc_pagetable:
	free_pagedir_tables(new_pde);
err_alloc_pagedir:
	return(err);
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_user_pde
 Input		:void
 Output		:void
 Return		:pde_t*
 		 < get pde of current user space >
 Description	:get pde of current user space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pde_t* get_user_pde(void)
{
	return((get_current())->mspace->pde);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:map_page_to_vm
 Input		:struct prcess *proc
 		 < process to map pages to vm >
 		 struct vm *vm
 		 < vm to map pages to >
 Output		:void
 Return		:int
 		 < result >
 Description	:map pages to user space vm
 		 this function is currently used for test only
 		 future work;
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
map_pages_to_vm(struct process *proc, struct vm *vm)
{
	pde_t *pde;
	int nr;
	int nr_page;
	int err_nr;
	int make_pages = 0;
	unsigned long la;
	unsigned long prot;
	unsigned long prot_pde;
	
	if (vm->nr_pages != PageCount(vm->end - vm->start)) {
		return(-EINVAL);
	}
	
	if (vm->end <= vm->start) {
		return(-EINVAL);
	}
	
	pde = proc->mspace->pde;
	
	la = vm->start;
	
	if (vm->prot & PROT_WRITE) {
		prot = PTE_USR_RW;
		prot_pde = PDE_USR_RW;
	} else {
		prot = PTE_USR_RO;
		prot_pde = PDE_USR_RO;
	}
	
	for (nr = PDIR_INDEX(vm->start);nr <= PDIR_INDEX(vm->end);nr++) {
		pte_t *pte;
		
		if (!(*(pde + nr) & PAGE_MASK)) {
			pte = alloc_pagetable();
			
			if (!pte) {
				err_nr = nr;
				goto failed_alloc_pagetable;
			}
			
			*(pde + nr) = prot_pde |
					(page_to_paddr(get_page(pte)) & PAGE_MASK);
		} else {
			*(pde + nr) = (pde_t)((*(pde + nr) & PAGE_MASK ) | prot_pde);
			pte = (pte_t*)toLogicalAddress(*(pde + nr) & PAGE_MASK);
		}
		
		for (nr_page = PAGE_INDEX(la);nr_page < N_PTE;nr_page++) {
			*(pte + nr_page) =
				(pte_t)(prot |
					(page_to_paddr(vm->pages[make_pages++])));
			la += PAGESIZE;
			if (vm->nr_pages <= make_pages) {
				return(0);
			}
		}
	}
	
	return(0);

failed_alloc_pagetable:
	for (nr = err_nr;PDIR_INDEX(vm->start) <= nr;nr--) {
		pte_t *pt;
		pt = (pte_t*)toLogicalAddress(*(pde + nr) & PAGE_MASK);
		
		free_page(get_page(pt));
	}
	
	return(-ENOMEM);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_la_pde
 Input		:unsigned long laddr
 		 < logical address of user space >
 Output		:void
 Return		:pde_t*
 		 < page directory entry indexed by specified user address >
 Description	:get pde entry of current user space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pde_t* get_la_pde(unsigned long laddr)
{
	pde_t *pde;
	if (UNLIKELY((unsigned long)toLogicalAddress(0) < laddr)) {
		return(NULL);
	}
	
	pde = get_user_pde();
	
	return(pde + PDIR_INDEX(laddr));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_la_pagetable
 Input		:unsigned long laddr
 		 < logical address of user space >
 Output		:void
 Return		:pte_t*
 		 < page table indexed by specified user address >
 Description	:get page table of current user space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pte_t* get_la_pagetable(unsigned long laddr)
{
	pde_t *pde;
	pte_t *pte;
	if (UNLIKELY((unsigned long)toLogicalAddress(0) < laddr)) {
		return(NULL);
	}
	
	pde = get_la_pde(laddr);
	pte = (pte_t*)(toLogicalAddress(*pde & PAGE_MASK));
	return(pte);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_la_pte
 Input		:unsigned long laddr
 		 < logical address of user space >
 Output		:void
 Return		:pte_t*
 		 < page table entry indexed by specified user address >
 Description	:get page table entry of current user space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pte_t* get_la_pte(unsigned long laddr)
{
	pte_t *pte;
	
	if (UNLIKELY(toLogicalAddress(0) < laddr)) {
		return(NULL);
	}
	
	pte = get_la_pagetable(laddr);
	
	//pte += PTBL_INDEX(laddr);
	pte += PAGE_INDEX(laddr);
	
	return(pte);
}


/*
----------------------------------------------------------------------------------
	T-Kernel Interface
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:GetSysMemBlk
 Input		:INT nblk
 		 < number of blocks to allocate for >
 		 UINT attr
 		 < memory attributes >
 Output		:void
 Return		:void
 Description	:allocate system memory
 		 T-Kernel api
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* GetSysMemBlk(INT nblk, UINT attr)
{
	if (attr & TA_RNG3) {
		/* ------------------------------------------------------------ */
		/* as for now, system memory allocation for user is		*/
		/* not supported.						*/
		/* ------------------------------------------------------------ */
		return(NULL);
	}
	
	return((void*)page_to_address(alloc_pages(nblk)));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:RelSysMemBlk
 Input		:CONST void *addr
 		 < system memory address to free >
 Output		:void
 Return		:void
 Description	:free system memory address
 		 T-Kernel api
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER RelSysMemBlk(CONST void *addr)
{
	unsigned long page = ((unsigned long)addr) & PAGE_MASK;
	
	if (page < (unsigned long)&pages[0] ||
		(unsigned long)&pages[nr_pages - 1] < page) {
		return(E_PAR);
	}
	
	/* -------------------------------------------------------------------- */
	/* as for now, free for consecutive pages is not supported		*/
	/* -------------------------------------------------------------------- */
	free_pages((struct page*)page, 1);
	
	return(E_OK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:RefSysMemInfo
 Input		:T_RSMB *pk_rsmb
 		 < system memory information buffer >
 Output		:T_RSMB *pk_rsmb
 		 < system memory information buffer >
 Return		:void
 Description	:get system memory information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER RefSysMemInfo(T_RSMB *pk_rsmb)
{
	pk_rsmb->blksz = PAGESIZE;
	pk_rsmb->total = nr_pages;
	pk_rsmb->free  = nr_free_pages;
	
	return(E_OK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:smPageCount
 Input		:UW byte
 		 < byte to count >
 Output		:void
 Return		:void
 Description	:count number of pages
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT UW smPageCount( UW byte )
{
	return PageCount(byte);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:init_page_allocator
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize page allocator
==================================================================================
*/
LOCAL void init_page_allocator(void)
{
	int i;
	struct boot_info *info = getBootInfo();
	
	nr_pages = PTBL_NUM(info->lowmem_limit);
	
	pages = (struct page*)allocLowMemory(nr_pages * sizeof(struct page));
	
	page_bitmap_bit_size = nr_pages;
	page_bitmap_size = page_bitmap_bit_size / (sizeof(unsigned long) * 8);
	
	
	vd_printf("nr_pages:%d pages:%d bitmap_size:%d\n", nr_pages, nr_pages * sizeof(struct page), page_bitmap_size);
	
	page_bitmap = (unsigned long*)allocLowMemory(page_bitmap_size);
	
	memset((void*)page_bitmap, 0x00, page_bitmap_size);
	
	nr_end_page = PTBL_NUM(info->lowmem_top);
	
	/* -------------------------------------------------------------------- */
	/* set flags for system memory						*/
	/* -------------------------------------------------------------------- */
	for (i = 0;i < nr_end_page;i++) {
		pages[i].flags = PAGE_UNEVICTABLE;
		pages[i].count = 1;
		pages[i].s_mem = NULL;
		pages[i].slab_page = NULL;
		pages[i].slab_cache = NULL;
		
		/* ------------------------------------------------------------ */
		/* set bitmap							*/
		/* ------------------------------------------------------------ */
		tstdlib_bitset((void*)page_bitmap, i);
	}
	
	/* -------------------------------------------------------------------- */
	/* set flags for allocation memory					*/
	/* -------------------------------------------------------------------- */
	for (i = nr_end_page;i < nr_pages;i++) {
		pages[i].flags = PAGE_RESERVED;
		pages[i].count = 0;
		pages[i].s_mem = NULL;
		pages[i].slab_page = NULL;
		pages[i].slab_cache = NULL;
	}
	
	nr_free_pages = nr_pages - nr_end_page;
}

/*
==================================================================================
 Funtion	:_free_pagetables
 Input		:pde_t *pde
 		 < address of page directory to free >
 		 unsigned long end
 		 < end address to free page talbes >
 Output		:void
 Return		:void
 Description	:free page tables
==================================================================================
*/
LOCAL INLINE void _free_pagetables(pde_t *pde, unsigned long end)
{
	int nr;
	int nr_page;
	
	/* -------------------------------------------------------------------- */
	/* free page tables							*/
	/* -------------------------------------------------------------------- */
	for (nr = 0;nr < PDIR_INDEX(end);nr++) {
		pte_t *table = (pte_t*)(*(pde + nr) & PAGE_MASK);
		
		if (!table) {
			continue;
		}
		
		table = (pte_t*)toLogicalAddress(table);
		
		for(nr_page = 0;nr_page < PT_ENTRIES;nr_page++) {
			pte_t *pte = (pte_t*)(*(table + nr_page) & PAGE_MASK);
			
			if (!pte) {
				continue;
			}
			
			pte = (pte_t*)toLogicalAddress(pte);
			
			free_pagetable(pte);
		}
	}
}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
