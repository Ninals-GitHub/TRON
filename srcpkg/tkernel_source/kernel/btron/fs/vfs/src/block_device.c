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

#include <tstdlib/bitop.h>

#include <bk/fs/vfs.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct blk_dev_list;
struct blk_minor_list;

LOCAL struct block_device* block_device_cache_alloc(void);
LOCAL void block_device_cache_free(struct block_device *bdev);
LOCAL struct blk_dev_list*
blk_dev_list_alloc(const char *name, unsigned int major);
LOCAL void blk_dev_list_free(struct blk_dev_list *blist);
LOCAL unsigned int* minor_list_alloc(void);
LOCAL void minor_list_free(unsigned int *minor_list);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	MAX_BLK_DEVICE		255
#define	MAX_BLK_MINOR		255
#define	MAX_BLK_DEV_NAME	64

struct blk_dev_list {
	unsigned int		*minor_list;
	unsigned int		major;
	char			*name;
};

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct kmem_cache *block_device_cache;
LOCAL const char block_device_cache_name[] = "block_device_cache";

LOCAL struct blk_dev_list *list_bdevs[MAX_BLK_DEVICE];

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
	
	vnode->v_bdev = bdev;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:register_block_device
 Input		:unsigned int major
 		 < major number to register >
 		 const char *name
 		 < name of the block device driver >
 Output		:void
 Return		:int
 		 < result >
 Description	:register a block device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int register_block_device(unsigned int major, const char *name)
{
	struct blk_dev_list **blist;
	//unsigned int nr_free;
	int first;
	int i;
	int alloc_major = 0;

	if (UNLIKELY(!name)) {
		return(-EINVAL);
	}
	
	if (UNLIKELY(MAX_BLK_DEVICE < major)) {
		return(-EINVAL);
	}
	
	if (major) {
		first = major - 1;
	} else {
		first = major;
	}
	
	for (i = first;i < MAX_BLK_DEVICE;i++) {
		blist = &list_bdevs[i];
		
		if (UNLIKELY(blist)) {
			return(-EBUSY);
		}
		
		alloc_major = i + 1;
		
		*blist = blk_dev_list_alloc(name, alloc_major);
		
		if (UNLIKELY(!*blist)) {
			return(-ENOMEM);
		}
		
		break;
	}
	
	if (MAX_BLK_DEVICE <= i) {
		return(-EBUSY);
	}
	
	return(alloc_major);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unregister_block_device
 Input		:unsigned int major
 		 < major number of the block device to unregister >
 		 const char *name
 		 < name of the block device to unregister >
 Output		:void
 Return		:void
 Description	:unregister a block device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void unregister_block_device(unsigned int major, const char *name)
{
	if (UNLIKELY(MAX_BLK_DEVICE < major)) {
		return;
	}
	
	if (UNLIKELY(!name)) {
		return;
	}
	
	if (UNLIKELY(!major)) {
		return;
	}
	
	if ((list_bdevs[major - 1])->major == major) {
		blk_dev_list_free(list_bdevs[major - 1]);
		list_bdevs[major - 1] = NULL;
	}
	
	return;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:add_block_device_minor
 Input		:unsigned int major
 		 < a major number of a block device >
 		 unsigned int minor
 		 < a minor number to add >
 Output		:void
 Return		:int
 		 < minor number >
 Description	:add a minor number
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int add_block_device_minor(unsigned int major, unsigned int minor)
{
	struct blk_dev_list *blist;
	int ret;
	
	if (UNLIKELY(MAX_BLK_DEVICE < major || !major)) {
		return(-EINVAL);
	}
	
	blist = list_bdevs[major - 1];
	
	if (UNLIKELY(!blist)) {
		return(-EINVAL);
	}
	
	if (UNLIKELY(!blist->minor_list)) {
		blist->minor_list = minor_list_alloc();
		
		if (UNLIKELY(!blist->minor_list)) {
			return(-ENOMEM);
		}
	}
	
	ret = (int)tstdlib_bitsearch0_set((void*)blist->minor_list,
					minor, MAX_BLK_MINOR);
	
	return(ret);
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
 Funtion	:blk_dev_list_alloc
 Input		:const char *name
 		 < name of a device >
 		 unsigned int major
 		 < major number of the block device >
 Output		:void
 Return		:struct blk_dev_list*
 		 < block device list >
 Description	:allocate a block device list
==================================================================================
*/
LOCAL struct blk_dev_list*
blk_dev_list_alloc(const char *name, unsigned int major)
{
	struct blk_dev_list *blist;
	
	blist = (struct blk_dev_list*)kmalloc(sizeof(struct blk_dev_list), 0);
	
	if (!blist) {
		return(blist);
	}
	
	memset((void*)blist, 0x00, sizeof(struct blk_dev_list));
	
	blist->name = kstrndup(name, MAX_BLK_DEV_NAME, 0);
	
	if (UNLIKELY(!blist->name)) {
		blk_dev_list_free(blist);
		return(NULL);
	}
	
	blist->major = major;
	
	return(blist);
}

/*
==================================================================================
 Funtion	:blk_dev_list_free
 Input		:struct blk_dev_list *blist
 		 < block device list to free >
 Output		:void
 Return		:void
 Description	:free a block device list
==================================================================================
*/
LOCAL void blk_dev_list_free(struct blk_dev_list *blist)
{
	if (LIKELY(blist->name)) {
		kfree(blist->name);
	}
	
	if (LIKELY(blist->minor_list)) {
		minor_list_free(blist->minor_list);
	}
	
	kfree(blist);
}

/*
==================================================================================
 Funtion	:minor_list_alloc
 Input		:void
 Output		:void
 Return		:unsigned int*
 		 < a bitmap for minor list>
 Description	:allocate a bitmap for minor list
==================================================================================
*/
LOCAL unsigned int* minor_list_alloc(void)
{
	unsigned int *map;
	
	map = (unsigned int*)kmalloc((MAX_BLK_MINOR + 1) / 8, 0);
	
	if (map) {
		memset((void*)map, 0x00, (MAX_BLK_MINOR + 1) / 8);
	}
	
	return(map);
}

/*
==================================================================================
 Funtion	:minor_list_free
 Input		:unsigned int *minor_list
 		 < a bitmap for minor list to free >
 Output		:void
 Return		:void
 Description	:free a bitomap for minor list
==================================================================================
*/
LOCAL void minor_list_free(unsigned int *minor_list)
{
	kfree((void*)minor_list);
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
