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

#include <cpu.h>
#include <typedef.h>
#include <bk/memory/page.h>
#include <bk/uapi/berrno.h>
#include <tstdlib/bitop.h>
#include <sys/rominfo.h>
#include <sys/sysinfo.h>


/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL void init_page_allocator(void);
LOCAL void __free_page(struct page *page);
LOCAL void free_initrd_pages(void);
LOCAL void alloc_reserved_pages(unsigned long start_address, unsigned long size);
LOCAL void free_initrd_pages(void);

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
void print_page_bitmap(void)
{
	int i;
	char *bitmap = (char*)page_bitmap;
	
	vd_printf("page_bitmap_size:%lu\n", page_bitmap_size);
	
	vd_printf("page_bitmap:");
	for(i = 0;i < page_bitmap_size;i++) {
		vd_printf("%02X", *(bitmap + i));
		//if (i == (80*25 / 2) *3)break;
	}
	vd_printf("\n");
}

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
	/* -------------------------------------------------------------------- */
	/* allocate pages for initial ram disk loaded by grub			*/
	/* -------------------------------------------------------------------- */
	if (isInitramfs()) {
		alloc_reserved_pages(getInitramfsAddress(), getInitramfsSize());
	}
	/* -------------------------------------------------------------------- */
	/* allocate pages for vdso loaded by grub				*/
	/* -------------------------------------------------------------------- */
	if (isVdso()) {
		alloc_reserved_pages(getVdsoAddress(), getVdsoSize());
	}
	
	return E_OK;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_mm_lately
 Input		:void
 Output		:void
 Return		:void
 Description	:end procedure of initialization for mm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void init_mm_lately(void)
{
	/* -------------------------------------------------------------------- */
	/* initrd is already copied to page caches				*/
	/* -------------------------------------------------------------------- */
	free_initrd_pages();
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_page_index
 Input		:unsigned long address
 		 < address to get its page index >
 Output		:void
 Return		:long
 		 < page index >
 Description	:get page index of specified address
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT long get_page_index(unsigned long address)
{
	long index;
	
	index = (address & PAGE_MASK) - KERNEL_BASE_ADDR;
	index >>= PAGE_SHIFT;
	
	return(index);
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
	unsigned long index;
	
	if (page < (struct page*)KERNEL_BASE_ADDR) {
		printf("unexpected page : 0x%08X\n", page);
		for(;;);
	}
	
	index = page - &pages[0];
	
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
	
	if (UNLIKELY(nr_free_pages < num)) {
		panic("%s:there are not enough pages [nr_free_pages=%d, num=%d\n", nr_free_pages, num);
		return(NULL);
	}
	
	BEGIN_CRITICAL_SECTION;
	index = tstdlib_bitsearch0_window((void*)page_bitmap,
						page_bitmap_bit_size, num);
	if (LIKELY(0 <= index)) {
		tstdlib_bitset_window((void*)page_bitmap, index, num);
	} else {
		panic("page is exhausted at [%s]\n", __func__);
	}
	END_CRITICAL_SECTION;
	
	if (UNLIKELY(page_bitmap_bit_size <= ( index + num))) {
		printf("alloc_pages:unexpected error:index[%d]", index);
		printf(" num[%d]\n", num);
		panic("alloc_pages:unexpected error:index[%d], num[%d]\n", index, num);
	}
	
	if (0 <= index) {
		for (i = 0;i < num;i++) {
			pages[index + i].count = 1;
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
	
	if (page < (struct page*)KERNEL_BASE_ADDR) {
		panic("unexpected error at %s [page=0x%08X]\n", __func__, page);
	}
	
	BEGIN_CRITICAL_SECTION {
		for (i = 0;i < num ;i++) {
			if (page[i].count) {
				page[i].count--;
			}
			
			if (!page[i].count) {
				__free_page(page + i);
			}
		}
	} END_CRITICAL_SECTION;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_page
 Input		:void *address
 		 < physical address of object >
 Output		:void
 Return		:struct page*
 		 < page to which address belongs >
 Description	:get a page to which address belongs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct page* get_page(void *address)
{
	long index;
	
	index = get_page_index((unsigned long)address);
	
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
	struct page *page = alloc_zeroed_page();
	
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
	struct page *page = alloc_zeroed_page();
	
	if (!page) {
		return(NULL);
	}
	
	if (page_to_address(page) == 0xC0579000) {
		printf("alloc_pagedir:0x%08X \n", page);
	}
	
	return((pde_t*)page_to_address(page));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:show_pagetables
 Input		:struct process *proc
 		 < show its own page tables >
 		 unsigned long start
 		 < start address to show >
 		 unsigned long end
 		 < end address to show >
 Output		:void
 Return		:void
 Description	:show page tables
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void
show_pagetables(struct process *proc, unsigned long start, unsigned long end)
{
	int nr;
	int nr_page;
	pde_t *pde = proc->mspace->pde;
	unsigned long la = start;
	
	for (nr = PDIR_INDEX(start);nr <= PDIR_INDEX(end);nr++) {
		pte_t *pte;
		
		printf("show pde:0x%08X *pde:0x%08X\n", pde + nr, *(pde + nr));
		pte = (pte_t*)toLogicalAddress(*(pde + nr) & PAGE_MASK);
		
		for(nr_page = PAGE_INDEX(la);nr_page < PT_ENTRIES;nr_page++) {
			printf("0x%08X ", *(pte + nr_page));
			la += PAGESIZE;
			if (end <= la) {
				printf("\n");
				return;
			}
		}
	}
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:show_a_pde_pte
 Input		:struct process *proc
 		 < process to show its pde and pte >
 		 unsigned long addr
 		 < address to show its pde and ptes >
 Output		:void
 Return		:void
 Description	:show a pde and a pte
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void show_a_pde_pte(struct process *proc, unsigned long addr)
{
	pde_t *pde;
	pte_t *pte;
	
	pde = get_address_pde(proc, addr);
	pte = get_address_pte(proc, addr);
	
	printf("pid=%d *pde:0x%08X *pte:0x%08X\n", proc->pid, *pde, *pte);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_free_pagetables
 Input		:pde_t *pde
 		 < address of page directory to free >
 		 unsigned long start
 		 < start address to free page tables >
 		 unsigned long end
 		 < end address to free page talbes >
 Output		:void
 Return		:void
 Description	:free page tables specified by logica addresses
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void _free_pagetables(pde_t *pde, unsigned long start, unsigned long end)
{
	int nr_page;
	int nr_page_end;
	int not_present;
	pde_t *pde_base;
	unsigned long la;
	
	//printf("%s:start=0x%08X end=0x%08X\n", __func__, start, end);
	
	la = start;
	pde_base = pde;
	
	/* -------------------------------------------------------------------- */
	/* free page tables							*/
	/* -------------------------------------------------------------------- */
	for (la = start; la < end ;la += PDIRSIZE) {
		pte_t *table;
		
		pde = pde_base + PDIR_INDEX(la);
		
		table = (pte_t*)(*pde & PAGE_MASK);
		
		if (!table) {
			*pde = 0;
			continue;
		}
		
		table = (pte_t*)toLogicalAddress(table);
		
		if (UNLIKELY(la == start)) {
			nr_page = PAGE_INDEX(start);
		} else {
			nr_page = 0;
		}
		
		if (UNLIKELY(end < (la + PDIRSIZE))) {
			if (UNLIKELY(!PAGE_INDEX(end))) {
				nr_page_end = PT_ENTRIES;
			} else {
				nr_page_end = PAGE_INDEX(end);
			}
		} else {
			nr_page_end = PT_ENTRIES;
		}
		
		if ((nr_page_end - nr_page) == PT_ENTRIES) {
			not_present = 1;
		} else {
			not_present = 0;
		}
		
		for ( ;nr_page < nr_page_end;nr_page++) {
			pte_t *pte = (pte_t*)(*(table + nr_page) & PAGE_MASK);
			
			if (!pte) {
				*(table + nr_page) = 0;
				continue;
			}
			
			pte = (pte_t*)toLogicalAddress(pte);
			
			*(table + nr_page) = 0;
		}
		
		
		if (not_present) {
			free_pagetable(table);
			
			*pde = 0;
		} else {
			not_present = 1;
			for (nr_page = 0 ;nr_page < PT_ENTRIES;nr_page++) {
				pte_t *pte = (pte_t*)(*(table + nr_page) & PAGE_MASK);
				
				if (pte) {
					//printf("not present = 0\n");
					not_present = 0;
					break;
				}
				
				*(table + nr_page) = 0;
			}
			
			if (not_present) {
				free_pagetable(table);
				*pde = 0;
			}
		}
	}
	flush_tlb();
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pagetable
 Input		:pte_t *pte
 		 < address of page table to free >
 Output		:void
 Return		:void
 Description	:free a page of page table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_pagetable(pte_t *pte)
{
	struct page *page;
	
	page = get_page((void*)pte);
	
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
	//printf("%s\n", __func__);
	/* -------------------------------------------------------------------- */
	/* free page tables for a user space					*/
	/* -------------------------------------------------------------------- */
	_free_pagetables(pde, 0, (~0UL));
	
	free_pagetable((pte_t*)pde);
	
	/* -------------------------------------------------------------------- */
	/* switch ms is responsible for flush tlb				*/
	/* so do nothing for tlb as of the moment				*/
	/* -------------------------------------------------------------------- */
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
EXPORT void free_user_pagetables(pde_t *pde)
{
	//printf("%s pde:0x%08X *pde:0x%08\n", __func__, pde, *pde);
	
	/* -------------------------------------------------------------------- */
	/* free page tables for a user space					*/
	/* -------------------------------------------------------------------- */
	_free_pagetables(pde, 0, KERNEL_BASE_ADDR);
	//printf("%s freed\n", __func__);
	/* -------------------------------------------------------------------- */
	/* switch ms is responsible for flush tlb				*/
	/* so do nothing for tlb as of the moment				*/
	/* -------------------------------------------------------------------- */
	
	//flush_tlb();
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
	
	//printf("------------------------------------\n");
	//printf("%s\n", __func__);
	
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
EXPORT void free_pagedir_tables_all(pde_t *pde)
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
 Funtion	:copy_kernel_pagetables
 Input		:struct process *from
 		 < copy from >
 		 struct process *to
 		 < copy to >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy pagetables of kernel space.
 		 before calling this process, must be set page directory
 		 *to->mspace->pde
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int copy_kernel_pagetables(struct process *from, struct process *to)
{
	int nr;
	pde_t *old_pde = from->mspace->pde;
	pde_t *new_pde;
	
	if (UNLIKELY(!to->mspace->pde)) {
		return(-ENOMEM);
	}
	
	new_pde = to->mspace->pde;
	
	/* -------------------------------------------------------------------- */
	/* copy old pde to new pde for kernel space				*/
	/* -------------------------------------------------------------------- */
	for (nr = PDIR_INDEX(KERNEL_BASE_ADDR);nr < PDIR_ENTRIES;nr++) {
		if (!*(old_pde + nr)) {
			continue;
		}
		
		*(new_pde + nr) = *(old_pde + nr);
		/* ------------------------------------------------------------ */
		/* there is no need to copy kernel page talbes			*/
		/* ------------------------------------------------------------ */
		/* do nothing							*/
	}
	
	return(0);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_user_pagetable
 Input		:struct process *from
 		 < copy from >
 		 struct process *to
 		 < copy to >
 		 unsigned long start
 		 < start address to copy from >
 		 unsigned long end
 		 < end address to copy from >
 		 int cow
 		 < boolean cow flag. 0: just copy 1: cow copy
 Output		:void
 Return		:int
 		 < result >
 Description	:copy user pagetables
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int copy_user_pagetable(struct process *from, struct process *to,
					unsigned long start, unsigned long end,
					int cow)
{
	int err;
	int nr;
	int nr_page;
	pde_t *old_pde = from->mspace->pde;
	pde_t *new_pde = to->mspace->pde;
	unsigned long la = start;
	
	if (UNLIKELY(!to->mspace->pde)) {
		return(-ENOMEM);
	}
	
	if (UNLIKELY(end <= start)) {
		return(-EINVAL);
	}
	
	if (UNLIKELY(KERNEL_BASE_ADDR < end)) {
		return(-ENOMEM);
	}
	
	/* -------------------------------------------------------------------- */
	/* copy old pde to new pde for user space				*/
	/* -------------------------------------------------------------------- */
	for (nr = PDIR_INDEX(start);nr < PDIR_INDEX(end);nr++) {
		pte_t *old_pte;
		pte_t *new_pte;
		
		if (!(*(old_pde + nr))) {
			la += PAGESIZE * PT_ENTRIES;
			continue;
		}
		
		old_pte = (pte_t*)toLogicalAddress(*(old_pde + nr) & PAGE_MASK);
		
		if (!*(new_pde +nr)) {
			struct page *page = alloc_zeroed_page();
			
			if (UNLIKELY(!page)) {
				printf("copy_pagetalbe:error:cannot allocate new pte\n");
				err = -ENOMEM;
				goto err_alloc_pagetable;
			}
			
			new_pte = (pte_t*)page_to_address(page);
			
			*(new_pde + nr) = (pde_t)(page_to_paddr(page) & PAGE_MASK);
		} else {
			new_pte = (pte_t*)toLogicalAddress(*(new_pde + nr) & PAGE_MASK);
		}
		
		*(new_pde + nr) |= *(old_pde + nr) & ~ PAGE_MASK;
		
		for (nr_page = PAGE_INDEX(la);
				nr_page < PT_ENTRIES;nr_page++, la += PAGESIZE) {
			if (!(*(old_pte + nr_page))) {
				if (end < la) {
					return(0);
				}
				continue;
			}
			
			/* ---------------------------------------------------- */
			/* clear write flag for cow				*/
			/* ---------------------------------------------------- */
			if (cow && (*(old_pte + nr_page) & PT_Writable)) {
				*(old_pte + nr_page) &= ~(PT_Writable);
			}
			
			*(new_pte + nr_page) = *(old_pte + nr_page);
			
			if (end < la) {
				return(0);
			}
		}
	}
	
	return(0);

err_alloc_pagetable:
	printf("unexpected error at %s [free_pagedir_tables_all]\n", __func__);
	free_pagedir_tables_all(new_pde);
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
	pde_t *pde_base;
	pde_t *pde;
	int nr_page;
	int nr_page_end;
	int make_pages = 0;
	unsigned long la;
	unsigned long prot;
	
	if (vm->nr_pages != PageCount(vm->end - vm->start)) {
		return(-EINVAL);
	}
	
	if (vm->end <= vm->start) {
		return(-EINVAL);
	}
	
	pde_base = proc->mspace->pde;
	
	la = vm->start;
	
	if (vm->prot & PROT_WRITE) {
		prot = PTE_USR_RW_NOT_P;
	} else {
		prot = PTE_USR_RO_NOT_P;
	}
	
	for (la = vm->start;la < vm->end;la += PDIRSIZE) {
		pte_t *pte;
		
		pde = pde_base + PDIR_INDEX(la);
		
		if (!(*pde & PAGE_MASK)) {
			struct page *page;
			
			page = alloc_zeroed_page();
			
			if (UNLIKELY(!page)) {
				goto failed_alloc_pagetable;
			}
			
			pte = (pte_t*)page_to_address(page);
			
			*pde = (pde_t)(page_to_paddr(page)& PAGE_MASK);
			*pde |= (pde_t)PDE_USR_RW;
		} else {
			pte = (pte_t*)toLogicalAddress(*pde & PAGE_MASK);
		}
		
		if (UNLIKELY(la == vm->start)) {
			nr_page = PAGE_INDEX(vm->start);
		} else {
			nr_page = 0;
		}
		
		if (UNLIKELY(la == vm->end - PAGESIZE)) {
			if (UNLIKELY(!PAGE_INDEX(vm->end))) {
				nr_page_end = PT_ENTRIES;
			} else {
				nr_page_end = PAGE_INDEX(vm->end);
			}
		} else {
			nr_page_end = PT_ENTRIES;
		}
		
		for ( ;nr_page < nr_page_end;nr_page++) {
			unsigned long prot_pte;
			
			if (UNLIKELY(vm->pages[make_pages])) {
				prot_pte = prot | PT_Present;
				
				*(pte + nr_page) = (pte_t)
					((page_to_paddr(vm->pages[make_pages])));
				*(pte + nr_page) |= prot_pte;
			
			} else {
				prot_pte = prot;
				*(pte + nr_page) = (pte_t)(prot_pte);
			}
			
			make_pages++;
			
			if (vm->nr_pages<= make_pages) {
				//printf("flush_tlb:%s\n", __func__);
				flush_tlb();
				return(0);
			}
		}
	}
	
	printf("flush_tlb:%s[2] %d %d\n", __func__, vm->nr_pages, make_pages);
	flush_tlb();
	
	return(0);

failed_alloc_pagetable:
	panic("memory is exhausted in %s\n", __func__);
	
	printf("flush_tlb:%s [3]\n", __func__);
	flush_tlb();
	
	return(-ENOMEM);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:activate_page
 Input		:pde_t *pde
 		 < pde which a process has >
 		 struct page *page
 		 < page to activate >
 		 struct vm *vm
 		 < virtual memory to activate its page >
 		 unsigned int la
 		 < virtual address which vm includes >
 Output		:void
 Return		:int
 		 < result >
 Description	:activate a paged whic is already mapped to vm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
activate_page(pde_t *pde, struct page *page, struct vm *vm, unsigned long la)
{
	pte_t *pte;
	unsigned long prot;
	
	pte = (pte_t*)toLogicalAddress(*(pde + PDIR_INDEX(la)) & PAGE_MASK);
	
	if (UNLIKELY(!pte)) {
		struct page *pte_page = alloc_zeroed_page();
		
		if (UNLIKELY(!page)) {
			panic("cannot allocate new pte at %s\n", __func__);
			return(-ENOMEM);
		}
		
		pte = (pte_t*)page_to_address(pte_page);
		
		*(pde + PDIR_INDEX(la)) = (pde_t)(page_to_paddr(pte_page) & PAGE_MASK);
		*(pde + PDIR_INDEX(la)) |= PDE_SYS_RW;
	}
	
	if (vm->prot & PROT_WRITE) {
		prot = PTE_USR_RW;
	} else {
		prot = PTE_USR_RO;
	}
	
	*(pte + PAGE_INDEX(la)) = (pte_t)(page_to_paddr(page));
	*(pte + PAGE_INDEX(la)) |= prot;
	
	//printf("flush_tlb:%s\n", __func__);
	flush_tlb();
	
	return(0);
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
	
	if (UNLIKELY((unsigned long)toLogicalAddress(0) < laddr)) {
		return(NULL);
	}
	
	pte = get_la_pagetable(laddr);
	
	//pte += PTBL_INDEX(laddr);
	pte += PAGE_INDEX(laddr);
	
	return(pte);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_page_contents
 Input		:struct page *to
 		 < copy to >
 		 struct page *from
 		 < copy from >
 Output		:void
 Return		:void
 Description	:copy page contents
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void copy_page_contents(struct page *to, struct page *from)
{
	unsigned long addr_to;
	unsigned long addr_from;
	
	addr_to = page_to_address(to);
	addr_from = page_to_address(from);
	
	memcpy((void*)addr_to, (void*)addr_from, PAGESIZE);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:flush_tlb
 Input		:void
 Output		:void
 Return		:void
 Description	:flush tlb
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void flush_tlb(void)
{
	pde_t *pde = get_current()->mspace->pde;
	
	/* -------------------------------------------------------------------- */
	/* load page directory base to cr3 register				*/
	/* -------------------------------------------------------------------- */
	loadPdbr((unsigned long)pde - KERNEL_BASE_ADDR);
	ASM (
	"jmp	flush_tlb_after_load_pdbr	\n\t"
	"flush_tlb_after_load_pdbr:"
	);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_address_pde
 Input		:struct process *proc
 		 < get pde corresponds to the address from the process's vm space>
 		 unsigned long address
 		 < address to get its pde >
 Output		:void
 Return		:pde_t *pde
 		 < pde >
 Description	:get pde from address
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pde_t* get_address_pde(struct process *proc, unsigned long address)
{
	pde_t *pde = proc->mspace->pde;
	
	return(pde + PDIR_INDEX(address));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_address_pte
 Input		:struct process *proc
 		 < get pte corresponds to the address from the process's vm space>
 		 unsigned long address
 		 < address to get its pte >
 Output		:void
 Return		:pde_t *pte
 		 < pte >
 Description	:get pte from address
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT pte_t* get_address_pte(struct process *proc, unsigned long address)
{
	pte_t *pte;
	pde_t *pde = get_address_pde(proc, address);
	
	pte = (pte_t*)toLogicalAddress(*pde & PAGE_MASK);
	
	return(pte + PAGE_INDEX(address));
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:map_paddr_pages
 Input		:unsigned long start
 		 < start physical address to map >
 		 unsinged long size
 		 < size of physcal address memory to map >
 Output		:void
 Return		:struct page*
 		 < mapped start page >
 Description	:map physical address space to pages for acpica
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct page* map_paddr_pages(unsigned long start, unsigned long size)
{
	struct page *map_pages;
	int nr_map = PageCount(size);
	int index;
	int i;
	unsigned long org = start;
	struct boot_info *info = getBootInfo();
	
	if (UNLIKELY(info->lowmem_limit < start)) {
		printf("unexpected physical address 0x%08X\n", start);
		return(NULL);
	}
	
	
	start = toLogicalAddress(start);
	
	index = get_page_index(start);
	
	if (UNLIKELY(nr_pages < index + nr_map)) {
		panic("unexpected error at %s: nr_pages = %d, index = %d\n", __func__, nr_pages, index);
	}
	
	map_pages = &pages[index];
	
	i = index;
	
	do {
		pages[i].count++;
		i++;
	} while (i < index + nr_map);
	
	//printf("%s:0x%08X 0x%08X -> 0x%08X\n", __func__, org, size, page_to_address(map_pages));
	
	return(map_pages);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unmap_paddr_pages
 Input		:unsigned long start
 		 < start logical address to unmap >
 		 unsinged long size
 		 < size of logical address memory to unmap >
 Output		:void
 Return		:void
 Description	:unmap physical address space to pages for acpica
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void unmap_paddr_pages(unsigned long start, unsigned long size)
{
	int index;
	int i;
	int nr_unmap = PageCount(size);
	struct boot_info *info = getBootInfo();
	
	//printf("%s:0x%08X 0x%08X\n", __func__, start, size);
	
	if (UNLIKELY(KERNEL_BASE_ADDR + info->lowmem_limit < start)) {
		printf("unexpected logical address 0x%08X\n", start);
		return;
	}
	
	index = get_page_index(start);
	
	if (UNLIKELY(nr_pages < index + nr_unmap)) {
		panic("unexpected error at %s: nr_pages = %d, index = %d\n", __func__, nr_pages, index);
	}
	
	i = index;
	
	do {
		free_page(&pages[i]);
		i++;
	} while (i < index + nr_unmap);
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
	page_bitmap_size = page_bitmap_bit_size / 8;
	
	vd_printf("nr_pages:%d pages:%d bitmap_size:%d\n", nr_pages, nr_pages * sizeof(struct page), page_bitmap_size);
	
	page_bitmap = (unsigned long*)allocLowMemory(page_bitmap_size);
	
	memset((void*)page_bitmap, 0x00, page_bitmap_size);
	
	nr_end_page = PTBL_NUM(info->lowmem_top);
	vd_printf("lowmem_top:0x%08X\n", nr_pages);
	
	/* -------------------------------------------------------------------- */
	/* set flags for system memory						*/
	/* -------------------------------------------------------------------- */
	for (i = 0;i < nr_end_page;i++) {
		pages[i].flags = PAGE_UNEVICTABLE;
		pages[i].count = 1;
		//pages[i].s_mem = NULL;
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
		//pages[i].s_mem = NULL;
		pages[i].slab_page = NULL;
		pages[i].slab_cache = NULL;
	}
	
	/* -------------------------------------------------------------------- */
	/* set flags for acpi reclaims memory					*/
	/* -------------------------------------------------------------------- */
	for (i = 0;i < info->num_mmap_entries;i++) {
		if (info->mmap2[i].type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) {
			long acpi_pages = PTBL_NUM(info->mmap2[i].len);
			long index = get_page_index(info->mmap2[i].addr +
							KERNEL_BASE_ADDR);
			int nr;
			
			for (nr = index;nr < index + acpi_pages;nr++) {
				pages[nr].flags = PAGE_UNEVICTABLE;
				pages[nr].count = 1;
				//pages[nr].s_mem = NULL;
				pages[nr].slab_page = NULL;
				pages[nr].slab_cache = NULL;
				
				/* -------------------------------------------- */
				/* set bitmap					*/
				/* -------------------------------------------- */
				tstdlib_bitset((void*)page_bitmap, nr);
			}
		}
	}
	
	
	nr_free_pages = nr_pages - nr_end_page;
}

/*
==================================================================================
 Funtion	:__free_page
 Input		:struct page *page
 		 < page to free >
 Output		:void
 Return		:void
 Description	:free a pages
==================================================================================
*/
LOCAL void __free_page(struct page *page)
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
==================================================================================
 Funtion	:alloc_reserved_pages
 Input		:unsigned long start_address
 		 < start address to allocate >
 		 unsigned long size
 		 < size of memry space >
 Output		:void
 Return		:void
 Description	:allocate pages for reserved pages
==================================================================================
*/
LOCAL void alloc_reserved_pages(unsigned long start_address, unsigned long size)
{
	long first;
	long num;
	long i;
	
	first = get_page_index(start_address);
	num = PageCount(size);
	tstdlib_bitset_window((void*)page_bitmap, first, num);
	
	for (i = first;i < (first + num);i++) {
		pages[i].flags = PAGE_RESERVED;
		pages[i].count = 1;
		//pages[i].s_mem = NULL;
		pages[i].slab_page = NULL;
		pages[i].slab_cache = NULL;
	}
	
	nr_free_pages -= num;
}

/*
==================================================================================
 Funtion	:free_initrd_pages
 Input		:void
 Output		:void
 Return		:void
 Description	:free pages for initrd
==================================================================================
*/
LOCAL void free_initrd_pages(void)
{
	long first;
	long num;
	long i;
	
	first = get_page_index(getInitramfsAddress());
	num = PageCount(getInitramfsSize());
	
	for (i = first;i < (first + num);i++) {
		tstdlib_bitclr((void*)page_bitmap, i);
		pages[i].flags = PAGE_RESERVED;
		pages[i].count = 0;
		//pages[i].s_mem = NULL;
		pages[i].slab_page = NULL;
		pages[i].slab_cache = NULL;
	}
	
	nr_free_pages += num;
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
