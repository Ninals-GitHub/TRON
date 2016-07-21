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
LOCAL ssize_t
generic_rw(struct file *filp, char *buf, size_t len, loff_t *ppos, int rw);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	GENERIC_READ	0
#define	GENERIC_WRITE	1

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct kmem_cache *page_cache_cache;
LOCAL const char page_cache_cache_name[] = "page_cache_cache";

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
EXPORT int init_page_cache(void)
{
	page_cache_cache = kmem_cache_create(page_cache_cache_name,
					sizeof(struct page_cache), 0, 0, NULL);
	
	if (UNLIKELY(!page_cache_cache)) {
		vd_printf("error:page_cache_cache\n");
		return(-ENOMEM);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_page_cache_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a page cache cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_page_cache_cache(void)
{
	kmem_cache_destroy(page_cache_cache);
}

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
EXPORT struct page_cache* page_cache_alloc(void)
{
	struct page_cache *cache;
	
	cache = (struct page_cache*)kmem_cache_alloc(page_cache_cache, 0);
	
	if (UNLIKELY(!cache)) {
		return(cache);
	}
	
	cache->page = NULL;
	cache->host = NULL;
	init_list(&cache->link_cache);
	cache->index = 0;
	
	return(cache);
}

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
EXPORT void page_cache_free(struct page_cache *page_cache)
{
	kmem_cache_free(page_cache_cache, page_cache);
}

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
EXPORT struct page_cache* search_page_cache(struct vnode *vnode, loff_t block)
{
	struct list *pos;
	
	if (UNLIKELY(is_empty_list(&vnode->v_cache))) {
		goto request_io;
	}
	
	list_for_each(pos, &vnode->v_cache) {
		struct page_cache *p;
		
		p = get_entry(pos, struct page_cache, link_cache);
		
		if (p->index == block) {
			//p->page->count++;
			return(p);
		}
	}
	
request_io:
	panic("request io[%d] is not implemented at %s\n", block, __func__);
	return(NULL);
}


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
EXPORT struct page* get_vm_page_cache(struct vnode *vnode, loff_t block)
{
	struct page_cache *p;
	
	p = search_page_cache(vnode, block);
	
	if (p) {
		p->page->count++;
		
		return(p->page);
	}
	
	return(NULL);
}

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
EXPORT size_t mem_page_cache(struct vnode *vnode, void *mem_from,
				loff_t pos, loff_t offset, loff_t size)
{
	loff_t start_pos = pos + offset;
	loff_t end_pos = start_pos + size;
	loff_t start = start_pos / PAGESIZE;
	loff_t end = start + PageCount(size);
	loff_t cur;
	size_t len = 0;
	char *from = (char*)mem_from;
	int ret;
	
	for (cur = start;cur < end;cur++) {
		struct page_cache *p = page_cache_alloc();
		struct page *page;
		char *addr_to;
		
		if (UNLIKELY(!p)) {
			return(-ENOMEM);
		}
		
		page = alloc_page();
		
		if (UNLIKELY(!page)) {
			page_cache_free(p);
			return(-ENOMEM);
		}
		
		addr_to = (char*)page_to_address(page);
		
		p->host = vnode;
		p->index = cur;
		p->page = page;
		
		if (UNLIKELY(cur == start)) {
			loff_t off_page = start_pos & PAGE_UMASK;
			
			memset((void*)addr_to, 0x00, off_page);
			if (size < PAGESIZE) {
				len = size;
				memcpy((void*)(addr_to + off_page),
					(void*)(from + start_pos), size);
				memset((void*)(addr_to + off_page + size),
					0x00, PAGESIZE - off_page - size);
			} else {
				len = PAGESIZE - off_page;
				memcpy((void*)(addr_to + off_page),
					(void*)(from + start_pos), len);
			}

		} else if (UNLIKELY(cur == (end - 1))) {
			loff_t off_page = end_pos & PAGE_UMASK;
			
			memcpy((void*)addr_to, (void*)(from + len), off_page);
			len += off_page;
			memset((void*)(addr_to + end_pos),
					0x00, PAGESIZE - off_page);
		} else {
			memcpy((void*)addr_to,  (void*)(from + len), PAGESIZE);
			len += PAGESIZE;
		}
		
		ret = insert_page_cache_list(p, vnode);
		
		if (UNLIKELY(ret)) {
			panic("unexpected error %s\n", __func__);
		}
	}
	
	return(len);
}

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
EXPORT void free_vnode_page_cache(struct vnode *vnode)
{
	struct page_cache *p;
	struct page_cache *temp;
	
	if (UNLIKELY(is_empty_list(&vnode->v_cache))) {
		return;
	}
	
	list_for_each_entry_safe(p, temp, &vnode->v_cache, link_cache) {
		if (LIKELY(p->page)) {
			free_page(p->page);
		}
		page_cache_free(p);
	}
}

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
EXPORT int
insert_page_cache_list(struct page_cache *new, struct vnode *vnode)
{
	struct list *pos;
	
	if (UNLIKELY(is_empty_list(&vnode->v_cache))) {
		add_list(&new->link_cache, &vnode->v_cache);
		return(0);
	}
	
	list_for_each(pos, &vnode->v_cache) {
		struct page_cache *p;
		struct page_cache *p_next;
		
		p = get_entry(pos, struct page_cache, link_cache);
		
		if (UNLIKELY(pos->next == &vnode->v_cache)) {
			if (new->index < p->index) {
				add_list(&new->link_cache, pos->prev);
			} else if (p->index < new->index) {
				add_list(&new->link_cache, pos);
			} else {
				panic("unexpected error occurence %s[1]\n", __func__);
			}
			return(0);
		}
		
		p_next = get_entry(pos->next, struct page_cache, link_cache);
		
		if (UNLIKELY(p_next->index == new->index)) {
			panic("unexpected error occurence %s[2]\n", __func__);
		}
		
		if (UNLIKELY(p->index == new->index)) {
			panic("unexpected error occurence %s[3]\n", __func__);
		}
		
		if ((p->index < new->index) && (new->index < p_next->index)) {
			add_list(&new->link_cache, pos);
			return(0);
		}
		
	}
	
	return(-1);
}


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
EXPORT ssize_t
generic_read(struct file *filp, char *buf, size_t len, loff_t *ppos)
{
	return(generic_rw(filp, buf, len, ppos, GENERIC_READ));
}

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
EXPORT ssize_t
generic_write(struct file *filp, const char *buf, size_t len, loff_t *ppos)
{
	return(generic_rw(filp, (char*)buf, len, ppos, GENERIC_WRITE));
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
 Funtion	:generic_rw
 Input		:struct file *filp
 		 < open file object >
 		 char *buf
 		 < user buffer to output/input to/from >
 		 size_t len
 		 < read length >
 		 loff_t *ppos
 		 < file offset >
 		 int rw
 		 < 0:read 1:write >
 Output		:void
 Return		:ssize_t
 		 < actual read/write length >
 Description	:generic file read/write method
==================================================================================
*/
LOCAL ssize_t
generic_rw(struct file *filp, char *buf, size_t len, loff_t *ppos, int rw)
{
	struct vnode *vnode = filp->f_vnode;
	struct list *pos;
	struct page_cache *p = NULL;
	char *addr;
	ssize_t rw_len = 0;
	size_t copy_len;
	loff_t blocksize = PAGESIZE;
	loff_t count;
	loff_t index = *ppos / blocksize;
	loff_t offset = *ppos & PAGE_UMASK;
	loff_t block_len = PageCount(len + offset);
	loff_t size = vnode->v_size;
	int err;
	
	//vd_printf("len:%d *ppos:%d rw:%d\n", len, *ppos, rw);
	//vd_printf("index:0x%08X ", index);
	//vd_printf("offset:0x%08X", offset);
	//vd_printf("block_len:0x%08X\n", block_len);
	
	if (rw) {
		if (size < len) {
			size = len;
		}
	}
	
	if (UNLIKELY(is_empty_list(&vnode->v_cache))) {
		//printf("block i/o request is not implemented %s[1]\n", __func__);
		//printf("file:%s\n", dentry_name(filp->f_path.dentry));
		//printf("buf:%s\n", buf);
		//panic("");
		err = pages_rw_request(vnode, index, block_len, rw);
		
		if (UNLIKELY(err)) {
			printf("unexpected error at %s [1]\n", __func__);
			panic("");
		}
		//printf("extend page cache\n");
	}
	
	list_for_each(pos, &vnode->v_cache) {
		p = get_entry(pos, struct page_cache, link_cache);
		
		if (UNLIKELY(p->index == index)) {
			break;
		}
		
		p = NULL;
	}
	
	if (UNLIKELY(!p)) {
		//panic("block i/o request is not implemented %s[2]\n", __func__);
		//return(rw_len);
		err = pages_rw_request(vnode, index, block_len, rw);
		
		if (UNLIKELY(err)) {
			printf("unexpected error at %s [2]\n", __func__);
			panic("");
		}
		
		p = search_page_cache(vnode, index);
	}
	
	/* -------------------------------------------------------------------- */
	/* read/write from/to a start page					*/
	/* -------------------------------------------------------------------- */
	addr = (char*)page_to_address(p->page);
	addr += offset;
	
	if (blocksize <= size) {
		copy_len = blocksize - offset;
	} else {
		copy_len = size - offset;
	}
	
	if (len < copy_len) {
		copy_len = len;
	}
	
	if (rw) {
		memcpy((void*)addr, (void*)buf, copy_len);
	} else {
		memcpy((void*)buf, (void*)addr, copy_len);
	}
	
	rw_len = copy_len;
	
	*ppos += rw_len;
	
	if (rw) {
		vnode->v_size += rw_len;
	}
	
	if (block_len == 1) {
		return(rw_len);
	}
	
	/* -------------------------------------------------------------------- */
	/* read/write from/to midrange pages					*/
	/* -------------------------------------------------------------------- */
	for (count = index + 1;count < (index + block_len - 1);count++) {
		if (UNLIKELY(p->link_cache.next == &vnode->v_cache)) {
			return(rw_len);
		}
		
		p = get_entry(p->link_cache.next, struct page_cache, link_cache);
		
		if (UNLIKELY(p->index != count)) {
			//panic("block i/o request is not implemented %s[3]\n", __func__);
			err = pages_rw_request(vnode, count, 1, rw);
			
			if (UNLIKELY(err)) {
				printf("unexpected error at %s [3]\n", __func__);
				panic("");
			}
			
			p = search_page_cache(vnode, count);
			
			if (UNLIKELY(!p)) {
				panic("unexpected error at %s[4]\n", __func__);
			}
		}
		
		addr = (char*)page_to_address(p->page);
		
		if (rw) {
			memcpy((void*)addr, (void*)(buf + rw_len), blocksize);
		} else {
			memcpy((void*)(buf + rw_len), (void*)addr, blocksize);
		}
		
		rw_len += blocksize;
		*ppos += rw_len;
		
		if (rw) {
			vnode->v_size += rw_len;
		}
	}
	
	if (UNLIKELY(p->link_cache.next == &vnode->v_cache)) {
		printf("unexpected behavior? at %s\n", __func__);
		return(rw_len);
	}
	
	/* -------------------------------------------------------------------- */
	/* read/write from/to a last page					*/
	/* -------------------------------------------------------------------- */
	p = get_entry(p->link_cache.next, struct page_cache, link_cache);
	
	if (UNLIKELY(p->index != (index + block_len - 1))) {
		//vd_printf("p->index:%ld ", p->index);
		//vd_printf("index:%ld ", index);
		//vd_printf("block_len:%ld\n", block_len);
		//panic("block i/o request is not implemented %s[4]\n", __func__);
		err = pages_rw_request(vnode, index + block_len - 1, 1, rw);
		
		if (UNLIKELY(err)) {
			printf("unexpected error at %s [3]\n", __func__);
			panic("");
		}
	}
	
	addr = (char*)page_to_address(p->page);

	copy_len = len - rw_len;
	
	if (UNLIKELY(blocksize < copy_len)) {
		panic("unexpected error occur:blocksize < copy_len\n");
	}
	
	//vd_printf("rw_len:%ld ", rw_len);
	//vd_printf("copy_len:%ld\n", copy_len);
	//vd_printf("*ppos:%ld\n", *ppos);
	
	if (rw) {
		memcpy((void*)addr, (void*)(buf + rw_len), copy_len);
	} else {
		memcpy((void*)(buf + rw_len), (void*)addr, copy_len);
	}
	rw_len += copy_len;
	*ppos += rw_len;
	
	if (rw) {
		vnode->v_size += rw_len;
	}
	
	//vd_printf("+rw_len:%ld ", rw_len);
	//vd_printf("*ppos:%ld\n", *ppos);
	
	return(rw_len);
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
