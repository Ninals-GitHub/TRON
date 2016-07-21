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

#ifndef	__BK_FS_PAGE_CACHE_H__
#define	__BK_FS_PAGE_CACHE_H__

#include <bk/typedef.h>
#include <bk/kernel.h>

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
struct page_cache {
	loff_t		index;
	struct page	*page;
	struct list	link_cache;
	struct vnode	*host;
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
 Funtion	:init_page_cache
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize page cache management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_page_cache(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_page_cache_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a page cache cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void destroy_page_cache_cache(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:page_cache_alloc
 Input		:void
 Output		:void
 Return		:struct page_cache*
 		 < page cache object >
 Description	:allocate a page cache object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct page_cache* page_cache_alloc(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:page_cache_free
 Input		:struct page_cache *page_cache
 		 < vnode to free >
 Output		:void
 Return		:void
 Description	:free a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void page_cache_free(struct page_cache *page_cache);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:search_page_cache
 Input		:struct vnode *vnode
 		 < vnode to get its page cache >
 		 loff_t block
 		 < block number in a file >
 Output		:void
 Return		:struct page_cache
 		 < a page cached >
 Description	:search a page cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct page_cache* search_page_cache(struct vnode *vnode, loff_t block);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_vm_page_cache
 Input		:struct vnode *vnode
 		 < vnode to get its page cache >
 		 loff_t block
 		 < block number in a file >
 Output		:void
 Return		:struct page*
 		 < a page cached page >
 Description	:get page cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct page* get_vm_page_cache(struct vnode *vnode, loff_t block);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mem_page_cache
 Input		:struct vnode *vnode
 		 < vnode which is page cache host >
 		 void *mem_from
 		 < file memory buffers >
 		 loff_t pos
 		 < start position in a file buffers >
 		 loff_t offset
 		 < offset in a memory buffers >
 		 loff_t size
 		 < size of memory buffers >
 Output		:void
 Return		:size_t
 		 < result >
 Description	:create page cache from memory buffers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT size_t mem_page_cache(struct vnode *vnode, void *mem_from,
				loff_t pos, loff_t offset, loff_t size);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_vnode_page_cache
 Input		:struct vnode *vnode
 		 < vnode to free its page cache >
 Output		:void
 Return		:void
 Description	:free page caches of a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_vnode_page_cache(struct vnode *vnode);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:insert_page_cache_list
 Input		:struct page_cache *new
 		 < page cache to insert >
 		 struct vnode *vnode
 		 < vnode to be inserted a page cache >
 Output		:void
 Return		:int
 		 < result >
 Description	:insert to vnode's page cache
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int
insert_page_cache_list(struct page_cache *new, struct vnode *vnode);

/*
----------------------------------------------------------------------------------
	file generic operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:generic_read
 Input		:struct file *filp
 		 < open file object >
 		 char *buf
 		 < user buffer to output to >
 		 size_t len
 		 < read length >
 		 loff_t *ppos
 		 < file offset >
 Output		:char *buf
 		 < user buffer to output to >
 Return		:ssize_t
 		 < actual read length >
 Description	:generic file read method
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ssize_t
generic_read(struct file *filp, char *buf, size_t len, loff_t *ppos);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:generic_write
 Input		:struct file *filp
 		 < open file object >
 		 const char *buf
 		 < user buffer to input from >
 		 size_t len
 		 < read length >
 		 loff_t *ppos
 		 < file offset >
 Output		:void
 Return		:ssize_t
 		 < actual write length >
 Description	:generic file write method
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ssize_t
generic_write(struct file *filp, const char *buf, size_t len, loff_t *ppos);

#endif	// __BK_FS_PAGE_CACHE_H__
