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

#include <tk/typedef.h>
#include <bk/kernel.h>
#include <bk/fs/vfs.h>
#include <bk/memory/page.h>
#include <bk/memory/slab.h>
#include <bk/memory/access.h>

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
 Funtion	:pages_rw_request
 Input		:struct vnode *vnode
 		 < a page cache host >
 		 loff_t index
 		 < current page cache position in a file >
 		 loff_t block_len
 		 < length of block to read/write >
 		 int rw
 		 < io direction >
 Output		:void
 Return		:int
 		 < result >
 Description	:start to read/write blocks from/to page cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
pages_rw_request(struct vnode *vnode, loff_t index, loff_t block_len, int rw)
{
	loff_t cur;
	struct list page_list;
	struct page_cache *p;
	struct page_cache *temp;
	int err;
	
	init_list(&page_list);
	
	for (cur = index;cur < (index + block_len);cur++) {
		p = page_cache_alloc();
		
		if (UNLIKELY(!p)) {
			panic("cannot allocate a page cache at %s\n", __func__);
		}
		
		p->page = alloc_zeroed_page();
		
		if (UNLIKELY(!p->page)) {
			page_cache_free(p);
			goto failed_alloc_page;
		}
		
		p->host = vnode;
		p->index = cur;
		
		add_list_tail(&p->link_cache, &page_list);
	}
	
	if (UNLIKELY(is_mount_nodev(vnode->v_sb))) {
		goto out;
	}
	
	panic("error:io request is currently not implemented at %s\n", __func__);
	
out:
	list_for_each_entry_safe(p, temp, &page_list, link_cache) {
		del_list(&p->link_cache);
		
		err = insert_page_cache_list(p, vnode);
		
		if (UNLIKELY(err)) {
			goto failed_insert_page_cache;
		}
	}
	
	return(0);
	
failed_insert_page_cache:
failed_alloc_page:
	list_for_each_entry_safe(p, temp, &page_list, link_cache) {
		free_page(p->page);
		page_cache_free(p);
	}
	
	return(-ENOMEM);
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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
