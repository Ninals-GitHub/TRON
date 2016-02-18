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

#include <bk/fs/vfs.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL struct block_device* block_device_cache_alloc(void);
LOCAL void block_device_cache_free(struct block_device *bdev);

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
LOCAL struct kmem_cache *block_device_cache;
LOCAL const char block_device_cache_name[] = "block_device_cache";

LOCAL struct list list_bdevs;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_block_device
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize block device management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_block_device(void)
{
	block_device_cache = kmem_cache_create(block_device_cache_name,
					sizeof(struct block_device), 0, 0, NULL);
	
	if (UNLIKELY(!block_device_cache)) {
		vd_printf("error:block_device_cache\n");
		return(-ENOMEM);
	}
	
	init_list(&list_bdevs);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_block_device_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a block device cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_block_device_cache(void)
{
	kmem_cache_destroy(block_device_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_block_device
 Input		:dev_t dev
 		 < device number >
 Output		:void
 Return		:struct block_device*
 		 < allocated object >
 Description	:allocate a block device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct block_device* alloc_block_device(dev_t dev)
{
	struct block_device *bdev;
	
	bdev = block_device_cache_alloc();
	
	if (UNLIKELY(!bdev)) {
		return(NULL);
	}
	
	bdev->bd_dev = dev;
	
	return(bdev);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_block_device
 Input		:struct block_device *bdev
 		 < block device to free >
 Output		:void
 Return		:void
 Description	:free a block device object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_block_device(struct block_device *bdev)
{
	block_device_cache_free(bdev);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setup_block_device
 Input		:struct vnode *vnode
 		 < vnode of block device >
 Output		:void
 Return		:int
 		 < result >
 Description	:set up a block device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int setup_block_device(struct vnode *vnode)
{
	struct block_device *bdev;
	
	bdev = alloc_block_device(vnode->v_rdev);
	
	if (UNLIKELY(!bdev)) {
		return(-ENOMEM);
	}
	
	bdev->bd_vnode = vnode;
	bdev->bd_block_size = vnode->v_sb->s_blocksize;
	
	add_list(&bdev->bd_node_bdevs, &list_bdevs);
	
	vnode->v_bdev = bdev;
	
	return(0);
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
 Funtion	:block_device_cache_alloc
 Input		:void
 Output		:void
 Return		:struct block_device*
 		 < block device object >
 Description	:allocate a block device object
==================================================================================
*/
LOCAL struct block_device* block_device_cache_alloc(void)
{
	struct block_device *bdev;
	
	bdev = (struct block_device*)kmem_cache_alloc(block_device_cache, 0);
	
	if (UNLIKELY(!bdev)) {
		return(bdev);
	}
	
	memset((void*)bdev, 0x00, sizeof(struct block_device));
	
	init_list(&bdev->bd_inodes);
	init_list(&bdev->bd_holder_disks);
	init_list(&bdev->bd_queue);
	init_list(&bdev->bd_list);
	init_list(&bdev->bd_node_bdevs);
	
	return(bdev);
}

/*
==================================================================================
 Funtion	:block_device_cache_free
 Input		:struct block_device *bdev
 		 < block device object to free >
 Output		:void
 Return		:void
 Description	:free a block device object
==================================================================================
*/
LOCAL void block_device_cache_free(struct block_device *bdev)
{
	kmem_cache_free(block_device_cache, bdev);
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
