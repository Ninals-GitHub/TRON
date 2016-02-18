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

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
EXPORT struct dentry_operations ramfs_d_op;
LOCAL struct dentry* ramfs_mount(struct file_system_type *fs_type, int flags,
					const char *dev_name, void *data);
LOCAL void ramfs_kill_sb(struct super_block *sb);
LOCAL int ramfs_fill_super(struct super_block *sb, void *data, int silent);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	RAMFS_BLOCK_SIZE	512
#define	RAMFS_BLOCK_SIZE_BIT	9

/*
==================================================================================

	Management 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	ramfs file sytem types
----------------------------------------------------------------------------------
*/
LOCAL struct file_system_type rootfs_fs_type =
{
	.name		= "rootfs",
	.fs_flags	= FS_REQUIRES_DEV,
	.mount		= ramfs_mount,
	.kill_sb	= ramfs_kill_sb,
};

LOCAL struct file_system_type ramfs_fs_type =
{
	.name		= "ramfs",
	.fs_flags	= FS_REQUIRES_DEV,
	.mount		= ramfs_mount,
	.kill_sb	= ramfs_kill_sb,
};

/*
----------------------------------------------------------------------------------
	ramfs super block operations
----------------------------------------------------------------------------------
*/
LOCAL struct super_operations ramfs_super_ops = {
	.alloc_vnode	= NULL,		// use vfs if
	.destroy_vnode	= NULL,		// use vfs if
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_rootfs
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize a rootfs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int _INIT_ init_rootfs(void)
{
	register_filesystem(&rootfs_fs_type);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_ramfs
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize a ramfs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int _INIT_ init_ramfs(void)
{
	register_filesystem(&ramfs_fs_type);
	
	/* -------------------------------------------------------------------- */
	/* if register_filesystem returns with error, ignore it			*/
	/* because of ramfs is already registered				*/
	/* -------------------------------------------------------------------- */
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
 Funtion	:ramfs_mount
 Input		:struct file_system_type *fs_type
 		 < file system type of ramfs >
 		 int flags
 		 < mount flags >
 		 const char *dev_name
 		 < device name to mount >
 		 void *data
 		 < mount options >
 Output		:void
 Return		:struct dentry*
 		 < root dentry of ramfs >
 Description	:mount ramfs
==================================================================================
*/
LOCAL struct dentry* ramfs_mount(struct file_system_type *fs_type, int flags,
					const char *dev_name, void *data)
{
	struct super_block *root_sb;
	int err;
	
	root_sb = sb_cache_alloc(fs_type, 0);
	
	if (UNLIKELY(!root_sb)) {
		vd_printf("ramfs_mount:error:root sb\n");
		return(NULL);
	}
	
	vd_printf("ramfs_fill_super\n");
	err = ramfs_fill_super(root_sb, NULL, 0);
	
	if (UNLIKELY(err)) {
		sb_cache_free(root_sb);
		return(NULL);
	}
	
	return(root_sb->s_root);
}

/*
==================================================================================
 Funtion	:ramfs_kill_sb
 Input		:struct super_block *sb
 		 < super block of ramfs >
 Output		:void
 Return		:void
 Description	:kill super block of ramfs on unmounting
==================================================================================
*/
LOCAL void ramfs_kill_sb(struct super_block *sb)
{
	sb_cache_free(sb);
}


/*
==================================================================================
 Funtion	:ramfs_fill_super
 Input		:struct super_block *sb
 		 < super block obejct to fill >
 		 void *data
 		 < user mount options >
 		 int silent
 		 < vorbose or not >
 Output		:void
 Return		:int
 		 < result >
 Description	:fill ramfs super
==================================================================================
*/
LOCAL int ramfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct vnode *root_vnode;
	struct dentry *root;
	
	sb->s_blocksize = RAMFS_BLOCK_SIZE;
	sb->s_blocksize_bits = RAMFS_BLOCK_SIZE_BIT ;
	sb->s_maxbytes = MAX_FILE_SIZE;
	sb->s_count = 1;
	atomic_inc(&sb->s_active);
	sb->s_op = &ramfs_super_ops;
	
	root_vnode = vfs_alloc_vnode(sb);
	
	if (UNLIKELY(!root_vnode)) {
		return(-ENOMEM);
	}
	
	root = alloc_root_dentry(root_vnode);
	
	if (UNLIKELY(!root)) {
		vfs_free_vnode(root_vnode);
		return(-ENOMEM);
	}
	
	sb->s_root = root;
	
	return(0);
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
