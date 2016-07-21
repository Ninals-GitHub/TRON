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

#ifndef	__BK_FS_DENTRY_H__
#define	__BK_FS_DENTRY_H__

#include <stdbool.h>
#include <bk/kernel.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct dentry;
struct super_block;
struct vnode;
struct path;
struct linux_dirent64;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	PTR_ERR_NOT_FOUND		((struct dentry*)(-1))
/*
----------------------------------------------------------------------------------
	query string
----------------------------------------------------------------------------------
*/
struct qstr {
	char		*name;
	long		len;
};

/*
----------------------------------------------------------------------------------
	operations for directory entry cache
----------------------------------------------------------------------------------
*/
struct dentry_operations {
	int (*d_revalidate)(struct dentry *dentry, unsigned int flags);
	int (*d_weak_revalidate)(struct dentry *dentry, unsigned int flags);
	int (*d_hash)(const struct dentry *dentry, struct qstr *name);
	int (*d_compare)(const struct dentry *parent, const struct dentry *dentry,
				unsigned int len, const char *str,
				const struct qstr *name);
	int (*d_delete)(const struct dentry *dentry);
	void (*d_release)(struct dentry *dentry);
	void (*d_prune)(struct dentry *dentry);
	void (*d_iput)(struct dentry *dentry, struct vnode *vnode);
	char *(*d_dname)(struct dentry *dentry, char *name_buf, int buflen);
	struct vfsmount *(*d_automount)(struct path *path);
	int (*d_manage)(struct dentry *dentry, bool rcu_walk);
	struct vnode* (*d_select_inode)(struct dentry *dentry, unsigned int flags);
};

/*
----------------------------------------------------------------------------------
	directory entry cache
----------------------------------------------------------------------------------
*/
#define	DENTRY_NAME_LEN		32

struct dentry {
	/* -------------------------------------------------------------------- */
	/* directory entry managenment						*/
	/* -------------------------------------------------------------------- */
	atomic_t			d_count;
	/* -------------------------------------------------------------------- */
	/* directory entry information						*/
	/* -------------------------------------------------------------------- */
	unsigned int			d_flags;
	struct dentry			*d_parent;
	struct qstr			d_name;
	char				d_iname[DENTRY_NAME_LEN];
	/* -------------------------------------------------------------------- */
	/* fs specific								*/
	/* -------------------------------------------------------------------- */
	void				*d_fsdata;
	/* -------------------------------------------------------------------- */
	/* directory entry relationships					*/
	/* -------------------------------------------------------------------- */
	struct vnode			*d_vnode;
	struct super_block		*d_sb;
	struct list			d_lru;
	struct list			d_child;	/* node of d_subdirs	*/
	struct list			d_subdirs;	/* our children		*/
	struct list			d_alias;	/* inode aliases	*/
	struct dentry_operations	*d_op;
};

/*
----------------------------------------------------------------------------------
	d_flags
----------------------------------------------------------------------------------
*/
#define	DCACHE_OP_HASH		0x00000001
#define	DCACHE_OP_COMPARE	0x00000002
#define	DCACHE_OP_REVALIDATE	0x00000004
#define	DCACHE_OP_DELETE	0x00000008
#define	DCACHE_OP_PUNE		0x00000010

#define	DCACHE_DISCONNECTED	0x00000020	/* the dentry may not be connected
						   to the dentry cache treee	*/
#define	DCACHE_REFERENCED	0x00000040	/* recently used, don't discard	*/
#define	DCACHE_RCUACCESS	0x00000080	/* ever has been rcu-visible	*/
#define	DCACHE_CANT_MOUNT	0x00000100
#define	DCACHE_GENOCIDE		0x00000200
#define	DCACHE_SHRINK_LIST	0x00000400
#define	DCACHE_OP_WEAK_REVALIDATE						\
				0x00000800
#define	DCACHE_NFSFS_RENAMED	0x00001000
#define	DCACHE_COOKIE		0x00002000
#define	DCACHE_FSNOTIFYPARENT_WATCHED						\
				0x00004000
#define	DCACHE_DENTRY_KILLED	0x00008000
#define	DCACHE_MOUNTED		0x00010000
#define	DCACHE_NEED_AUTOMOUNT	0x00020000
#define	DCACHE_MANAGE_TRANSIT	0x00040000
#define	DCACHE_MANAGED_DENTRY	(DCACHE_MOUNTED | DCACHE_NEED_AUTOMOUNT |	\
				 DCACHE_MANAGE_TRANSIT)
#define	DCACHE_LRU_LIST		0x00080000

#define	DCACHE_ENTRY_TYPE	0x00700000
#define	DCACHE_MISS_TYPE	0x00000000	/* negative dentry		*/
#define	DCACHE_WHITEOUT_TYPE	0x00100000	/* whiteout (stop pathwalk)	*/
#define	DCACHE_DIRECTORY_TYPE	0x00200000	/* normal directory		*/
#define	DCACHE_AUTODIR_TYPE	0x00300000	/* lookupless directory		*/
#define	DCACHE_REGULAR_TYPE	0x00400000	/* regular file			*/
#define	DCACHE_SPECIAL_TYPE	0x00500000	/* other file			*/
#define	DCACHE_SYMLINK_TYPE	0x00600000	/* symlink or faullthru to such	*/

#define	DCACHE_MAY_FREE		0x00800000
#define	DCACHE_FALLTHRU		0x01000000	/* fall through to lower layer	*/
#define	DCACHE_OP_SELECT_INODE	0x02000000	/* unioned entry		*/

#define	DCACHE_ENCRYPTED_WITH_KEY						\
				0x04000000	/* dir is encrypted with a key	*/
#define	DCACHE_OP_REAL		0x08000000


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
 Funtion	:init_dentry
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize dentry management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_dentry(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_dentry_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a dentry cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void destroy_dentry_cache(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_alloc_dentry
 Input		:struct vnode *vnode
 		 < vnode to be associated with a new dentry >
 		 struct dentry *parent
 		 < a parent of a new dentry >
 		 struct qstr *name
 		 < name for a new dentry >
 Output		:void
 Return		:struct dentry*
 		 < an allocated dentry >
 Description	:allocate a new dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct dentry*
vfs_alloc_dentry(struct vnode *vnode, struct dentry *parent, struct qstr *name);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_alloc_negative_dentry
 Input		:struct dentry *parent
 		 < a parent of a new dentry >
 		 struct qstr *name
 		 < name for a new dentry >
 Output		:void
 Return		:struct dentry*
 		 < an allocated dentry >
 Description	:allocate a new negative dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define vfs_alloc_negative_dentry(parent, name)					\
			vfs_alloc_dentry(NULL, parent, name)

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_root_dentry
 Input		:struct vnode *root
 		 < root vnode >
 Output		:void
 Return		:struct dentry*
 		 < root dentry >
 Description	:allocate a root dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct dentry* alloc_root_dentry(struct vnode *root);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_free_dentry
 Input		:struct dentry *dentry
 		 < directory entry to free >
 Output		:void
 Return		:void
 Description	:free a directory entry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void vfs_free_dentry(struct dentry *dentry);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:lookup_dentry_children
 Input		:struct dentry *dir
 		 < dentry of directory to lookup in >
 		 const struct qstr *name
 		 < name to lookup >
 		 unsigned int flags
 		 < look up flags >
 Output		:void
 Return		:struct dentry*
 		 < found result >
 Description	:lookup name in a dentry of directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct dentry*
lookup_dentry_children(struct dentry *dir, const struct qstr *name,
						unsigned int flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dentry_associated
 Input		:struct dentry *dentry
 		 < dentry to associate with its vnode >
 		 struct vnode *vnode
 		 < vnode to be associted with >
 Output		:void
 Return		:void
 Description	:associate dentry with its vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void dentry_associated(struct dentry *dentry, struct vnode *vnode);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dentry_unassociated
 Input		:struct dentry *dentry
 		 < dentry to associate with its vnode >
 		 struct vnode *vnode
 		 < vnode to be associted with >
 Output		:void
 Return		:void
 Description	:associate dentry with its vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void dentry_unassociated(struct dentry *dentry, struct vnode *vnode);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dentry_add_dir
 Input		:struct dentry *dir
 		 < parent directory to which add the new entry >
 		 struct dentry *dentry
 		 < dentry to add to its parent directory >
 Output		:void
 Return		:void
 Description	:add dentry to its parent directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void dentry_add_dir(struct dentry *dir, struct dentry *dentry);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dentry_name
 Input		:struct dentry *dentry
 		 < dentry to get its name >
 Output		:void
 Return		:char *
 		 < dentry name >
 Description	:get dentry name
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL INLINE char* dentry_name(struct dentry *dentry)
{
	if (UNLIKELY(dentry->d_name.name)) {
		return(dentry->d_name.name);
	}
	
	return(dentry->d_iname);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:show_subdirs
 Input		:struct dentry *dir
 		 < directory to show its children >
 Output		:void
 Return		:void
 Description	:show subdirectories
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void show_subdirs(struct dentry *dir);


#endif	// __BK_FS_DENTRY_H__
