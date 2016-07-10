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

#ifndef	__BK_PAGE_H__
#define	__BK_PAGE_H__

#include <typedef.h>
#include <bk/typedef.h>
#include <bk/bprocess.h>
#include <bk/memory/prot.h>
#include <bk/memory/vm.h>
#include <tk/typedef.h>
#include <tk/sysmgr.h>
#include <tk/errno.h>
#include <tk/task.h>

#if STD_SH7727
#endif
#if STD_SH7751R
#endif
#if MIC_M32104
#endif
#if STD_S1C38K
#endif
#if STD_MC9328
#endif
#if MIC_VR4131
#endif
#if STD_VR5500
#endif
#if STD_MB87Q1100
#endif
#if STD_SH7760
#endif
#if TEF_EM1D
#  include <bk/memory/sysdepend/em1d/memdef.h>
#  include <bk/memory/sysdepend/em1d/mmu.h>
#  include <bk/memory/sysdepend/em1d/pagedef.h>
#endif
#if _STD_X86_
#  include <bk/memory/sysdepend/x86/memdef.h>
#  include <bk/memory/sysdepend/x86/mmu.h>
#  include <bk/memory/sysdepend/x86/pagedef.h>
#  include <bk/memory/sysdepend/x86/vm.h>
#endif

#include <bk/memory/vm.h>
#include <bk/memory/prot.h>


/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct kmem_cache;
struct slab;
struct vm;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
enum _PAGE_FLAGS {
	_PAGE_ERROR,		// 0
	_PAGE_REFERENCED,	// 1
	_PAGE_UPTODATE,		// 2
	_PAGE_DIRTY,		// 3
	_PAGE_ACTIVE,		// 4
	_PAGE_SLAB,		// 5
	_PAGE_RESERVED,		// 6
	_PAGE_PRIVATE,		// 7
	_PAGE_SWAPCACHE,	// 8
	_PAGE_MAPPEDTODISK,	// 9
	_PAGE_RECLAIM,		// 10
	_PAGE_SWAPBACKED,	// 11
	_PAGE_UNEVICTABLE,	// 12
	_PAGE_MLOCKED,		// 13
	_PAGE_COW,		// 14
	NUM_PAGE_FLAGS,
};

#define	PAGE_ERROR		(1UL << _PAGE_ERROR)
#define	PAGE_REFERENCED		(1UL << _PAGE_REFERENCED)
#define	PAGE_UPTODATE		(1UL << _PAGE_UPTODATE)
#define	PAGE_DIRTY		(1UL << _PAGE_DIRTY)
#define	PAGE_ACTIVE		(1UL << _PAGE_ACTIVE)
#define	PAGE_SLAB		(1UL << _PAGE_SLAB)
#define	PAGE_RESERVED		(1UL << _PAGE_RESERVED)
#define	PAGE_PRIVATE		(1UL << _PAGE_PRIVATE)
#define	PAGE_SWAPCACHE		(1UL << _PAGE_SWAPCACHE)
#define	PAGE_MAPPEDTODISK	(1UL << _PAGE_MAPPEDTODISK)
#define	PAGE_RECLAIM		(1UL << _PAGE_RECLAIM)
#define	PAGE_SWAPBACKED		(1UL << _PAGE_SWAPBACKED)
#define	PAGE_UNEVICTABLE	(1UL << _PAGE_UNEVICTABLE)
#define	PAGE_MLOCKED		(1UL << _PAGE_MLOCKED)
#define	PAGE_COW		(1UL << _PAGE_COW)


struct page {
	unsigned long		flags;
	int			count;
//	void			*s_mem;
	struct slab		*slab_page;
	struct kmem_cache	*slab_cache;
};

#define	PAGE_JUST_COPY		0
#define	PAGE_COW_COPY		1

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
 Funtion	:init_memmgr
 Input		:void
 Output		:void
 Return		:ER
 		 < error code >
 Description	:initialize page frame management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER init_memmgr(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_mm_lately
 Input		:void
 Output		:void
 Return		:void
 Description	:end procedure of initialization for mm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void init_mm_lately(void);

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
IMPORT long get_page_index(unsigned long address);

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
IMPORT unsigned long page_to_address(struct page *page);

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
IMPORT unsigned long page_to_paddr(struct page *page);

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
IMPORT struct page* alloc_pages(int num);

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
IMPORT struct page* alloc_slab_pages(int num);

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
IMPORT struct page* get_page(void *address);

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
IMPORT void free_pages(struct page *page, int num);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_page
 Input		:struct page *page
 		 < page to free >
 Output		:void
 Return		:void
 Description	:free a page
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define	 free_page(page) free_pages(page, 1)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_page
 Input		:void
 Output		:void
 Return		:struct page*
 		 < page information >
 Description	:allocate a page from page allocator
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define alloc_page() alloc_pages(1)

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
IMPORT struct page* alloc_zeroed_page(void);

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
IMPORT pte_t* alloc_pagetable(void);

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
IMPORT pde_t* alloc_pagedir(void);

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
IMPORT void
show_pagetables(struct process *proc, unsigned long start, unsigned long end);

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
IMPORT void show_a_pde_pte(struct process *proc, unsigned long addr);

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
IMPORT void _free_pagetables(pde_t *pde, unsigned long start, unsigned long end);

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
IMPORT void free_pagetable(pte_t *pte);

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
EXPORT void free_pagetables_all(pde_t *pde);

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
IMPORT void free_user_pagetables(pde_t *pde);

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
IMPORT void free_pagetables(pde_t *pde);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_pagedir
 Input		:pde_t *pde
 		 < page directory to free >
 Output		:void
 Return		:void
 Description	:free page directory and page tables
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_pagedir(pde_t *pde);

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
IMPORT void free_pagedir_tables_all(pde_t *pde);

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
IMPORT pde_t* get_page_pde(struct page *page, pde_t *pde);

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
IMPORT pte_t* get_page_pte(struct page *page, pde_t *pde);

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
IMPORT int copy_kernel_pagetables(struct process *from, struct process *to);

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
IMPORT int copy_user_pagetable(struct process *from, struct process *to,
					unsigned long start, unsigned long end,
					int cow);

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
IMPORT int copy_pagetable(struct process *from, struct process *to);

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
IMPORT pde_t* get_user_pde(void);

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
IMPORT int
map_pages_to_vm(struct process *proc, struct vm *vm);

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
IMPORT int
activate_page(pde_t *pde, struct page *page, struct vm *vm, unsigned long la);

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
IMPORT pde_t* get_la_pde(unsigned long laddr);

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
IMPORT pte_t* get_la_pagetable(unsigned long laddr);

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
IMPORT pte_t* get_la_pte(unsigned long laddr);

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
IMPORT void copy_page_contents(struct page *to, struct page *from);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:flush_tlb
 Input		:void
 Output		:void
 Return		:void
 Description	:flush tlb
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void flush_tlb(void);

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
IMPORT pde_t* get_address_pde(struct process *proc, unsigned long address);

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
IMPORT pte_t* get_address_pte(struct process *proc, unsigned long address);

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
IMPORT void* GetSysMemBlk(INT nblk, UINT attr);

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
IMPORT ER RelSysMemBlk(CONST void *addr);

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
IMPORT ER RefSysMemInfo(T_RSMB *pk_rsmb);


#endif	// __BK_PAGE_H__
