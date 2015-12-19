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
enum PAGE_FLAGS {
	PAGE_ERROR,
	PAGE_REFERENCED,
	PAGE_UPTODATE,
	PAGE_DIRTY,
	PAGE_ACTIVE,
	PAGE_SLAB,
	PAGE_RESERVED,
	PAGE_PRIVATE,
	PAGE_SWAPCACHE,
	PAGE_MAPPEDTODISK,
	PAGE_RECLAIM,
	PAGE_SWAPBACKED,
	PAGE_UNEVICTABLE,
	PAGE_MLOCKED,
	NUM_PAGE_FLAGS,
};


struct page {
	unsigned long		flags;
	int			count;
	void			*s_mem;
	struct slab		*slab_page;
	struct kmem_cache	*slab_cache;
};

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
 Funtion	:free_pagetable
 Input		:pte_t *pte
 		 < address of page table to free >
 Output		:void
 Return		:void
 Description	:free page table
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
EXPORT void free_pagedir_tables(pde_t *pde);

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
