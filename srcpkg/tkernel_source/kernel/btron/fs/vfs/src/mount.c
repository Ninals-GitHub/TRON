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


#include <bk/kernel.h>
#include <bk/fs/vfs.h>
#include <bk/memory/slab.h>

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
LOCAL struct kmem_cache *mount_cache;
LOCAL const char mount_cache_name[] = "mount_cache";

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_mount
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize mount management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_mount(void)
{
	mount_cache = kmem_cache_create(mount_cache_name,
					sizeof(struct mount), 0, 0, NULL);
	
	if (UNLIKELY(!mount_cache)) {
		vd_printf("error:mount_cache\n");
		return(-ENOMEM);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_mount_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a mount cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_mount_cache(void)
{
	kmem_cache_destroy(mount_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mount_cache_alloc
 Input		:const char *devname
 		 < device name >
 Output		:void
 Return		:struct mount*
 		 < mount object >
 Description	:allocate a mount object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct mount* mount_cache_alloc(const char *devname)
{
	size_t len;
	struct mount *mount = (struct mount*)kmem_cache_alloc(mount_cache, 0);
	
	if (UNLIKELY(!mount)) {
		return(mount);
	}
	
	memset((void*)mount, 0x00, sizeof(struct mount));
	
	len = strnlen(devname, PAGESIZE);
	
	mount->mnt_devname = kstrndup(devname, len, 0);
	
	if (!mount->mnt_devname) {
		goto failed_dup_devname;
	}
	
	init_list(&mount->list_mounts);
	init_list(&mount->mnt_child);
	init_list(&mount->mnt_instance);
	
	mount->mnt_count = 1;
	
	return(mount);
	
failed_dup_devname:
	mount_cache_free(mount);
	return(NULL);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mount_cache_free
 Input		:struct mount *mount
 		 < mount object to free >
 Output		:void
 Return		:void
 Description	:free a mount object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void mount_cache_free(struct mount *mount)
{
	kmem_cache_free(mount_cache, mount);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mount_root_fs
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:mount root file system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int mount_root_fs(void)
{
	struct dentry *root;
	struct mount *mnt_root;
	struct file_system_type *root_fs_type;
	const char root_dev_name[] = "root";
	int mount_flags = 0;
	int err = 0;
	
	mnt_root = mount_cache_alloc(root_dev_name);
	
	if (UNLIKELY(!mnt_root)) {
		vd_printf("make_mount_root:error:mount\n");
		return(-ENOMEM);
	}
	
	root_fs_type = get_filesystem_type("rootfs");
	
	if (UNLIKELY(!root_fs_type)) {
		vd_printf("cannot get root_fs\n");
		err = -EINVAL;
		goto failed_get_filesystem_type;
	}
	
	vd_printf("root_fs_type->mount\n");
	root = root_fs_type->mount(root_fs_type, mount_flags, root_dev_name, NULL);
	
	if (UNLIKELY(!root)) {
		err = -ENOMEM;
		goto failed_mount;
	}
	
	mnt_root->mnt.mnt_root = root;
	mnt_root->mnt.mnt_sb = root->d_sb;
	mnt_root->mnt.mnt_flags = mount_flags;
	
	set_root(get_current(), &mnt_root->mnt, root);
	set_cwd(get_current(), &mnt_root->mnt, root);
	
	return(0);

failed_get_filesystem_type:
failed_mount:
	mount_cache_free(mnt_root);
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mount_bdev
 Input		:struct file_system_type *fs_type
 		 < file system type to mount >
 		 int flags
 		 < mount flags >
 		 const char *dev_name
 		 < device name to mount >
 		 void *data
 		 < mount options >
 		 int (*fill_super)(struct super *sb, void *data, int silent)
 		 < fill super call back function >
 Output		:void
 Return		:struct dentry*
 		 < dentry of root directory >
 Description	:mount block device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct dentry*
mount_bdev(struct file_system_type *fs_type, int flags,
		const char *dev_name, void *data,
		int (*fill_super)(struct super_block *sb, void *data, int silent))
{
	return(NULL);
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
