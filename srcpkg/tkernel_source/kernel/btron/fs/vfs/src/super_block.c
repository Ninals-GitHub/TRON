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
LOCAL struct kmem_cache *super_block_cache;
LOCAL const char super_block_cache_name[] = "super_block_cache";

/*
----------------------------------------------------------------------------------
	vfs inode number management
----------------------------------------------------------------------------------
*/
LOCAL unsigned long next_ino;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_super_block
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize super block management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_super_block(void)
{
	super_block_cache = kmem_cache_create(super_block_cache_name,
					sizeof(struct super_block), 0, 0, NULL);
	
	if (UNLIKELY(!super_block_cache)) {
		vd_printf("error:super_block_cache\n");
		return(-ENOMEM);
	}
	
	next_ino = 1;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_super_block_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a cache of super block
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_super_block_cache(void)
{
	kmem_cache_destroy(super_block_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:sb_cache_alloc
 Input		:struct file_system_type *type
 		 < a file system type of sb >
 		 int flags
 		 < file system flags >
 Output		:void
 Return		:struct super_block*
 		 < super block object >
 Description	:allocate a super block object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct super_block*
sb_cache_alloc(struct file_system_type *type, int flags)
{
	struct super_block *sb;
	
	sb = (struct super_block*)kmem_cache_alloc(super_block_cache, 0);
	
	if (UNLIKELY(!sb)) {
		return(sb);
	}
	
	memset((void*)sb, 0x00, sizeof(struct super_block));
	
	init_list(&sb->s_list);
	init_list(&sb->s_mounts);
	init_list(&sb->s_dentry_lru);
	init_list(&sb->s_inode_lru);
	init_list(&sb->s_inodes);
	
	sb->s_flags = flags;
	sb->s_count = 1;
	sb->s_type = type;
	atomic_add(1, &sb->s_active);
	
	return(sb);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:sb_cache_free
 Input		:struct super_block *sb
 		 < super block object to free >
 Output		:void
 Return		:void
 Description	:free a super block object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void sb_cache_free(struct super_block *sb)
{
	kmem_cache_free(super_block_cache, sb);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_alloc_vnode
 Input		:struct super_block *sb
 		 < super block object >
 Output		:void
 Return		:struct vnode*
 		 < an allocated vnode >
 Description	:allocate a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct vnode* vfs_alloc_vnode(struct super_block *sb)
{
	struct vnode *vnode;
	
	if (sb->s_op->alloc_vnode) {
		return(sb->s_op->alloc_vnode(sb));
	}
	
	vnode = vnode_cache_alloc();
	
	if (UNLIKELY(!vnode)) {
		return(NULL);
	}
	
	vnode->v_ino = next_ino++;
	
	vnode->v_atime.tv_sec = 0;
	vnode->v_atime.tv_nsec = 0;
	vnode->v_mtime.tv_sec = 0;
	vnode->v_mtime.tv_nsec = 0;
	vnode->v_ctime.tv_sec = 0;
	vnode->v_ctime.tv_nsec = 0;
	
	atomic_inc(&vnode->v_count);
	
	vnode->v_sb = sb;
	
	return(vnode);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_destroy_vnode
 Input		:struct vnode *vnode
 		 < vnode to destroy >
 		 int flags
 		 < flags >
 Output		:void
 Return		:void
 Description	:destroy a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void vfs_destroy_vnode(struct vnode *vnode, int flags)
{
	if (vnode->v_sb->s_op->destroy_vnode) {
		vnode->v_sb->s_op->destroy_vnode(vnode, flags);
	} else {
		vnode_cache_free(vnode);
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_free_vnode
 Input		:struct vnode *vnode
 		 < vnode to free >
 Output		:void
 Return		:void
 Description	:free a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void vfs_free_vnode(struct vnode *vnode)
{
	int err = 0;;
	
	if (UNLIKELY(!vnode)) {
		return;
	}
	
	if (!atomic_dec_and_test(&vnode->v_count)) {
		return;
	}
	
	if (vnode->v_sb->s_op->drop_vnode) {
		err = vnode->v_sb->s_op->drop_vnode(vnode);
	} else {
		/* future work:unhashed						*/
	}
	
	if (!err && (vnode->v_sb->s_flags & MS_ACTIVE)) {
		/* future work:add lru						*/
		return;
	}
	
	if (!err) {
		/* future work:write inode					*/
	}
	
	vfs_destroy_vnode(vnode, 0);
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
