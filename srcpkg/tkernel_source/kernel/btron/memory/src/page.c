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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
