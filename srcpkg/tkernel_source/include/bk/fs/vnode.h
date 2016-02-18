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

#ifndef	__BK_FS_INODE_H__
#define	__BK_FS_INODE_H__

#include <stdint.h>

#include <bk/typedef.h>
#include <bk/kernel.h>
#include <bk/uapi/sys/time.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct super_block;
struct dentry;
struct pipe_vnode_info;
struct block_device;
struct char_device;
struct file_operations;
struct vnode;
struct iattr;
struct stat;
struct fiemap_extent_info;
struct file;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	operations for inode
----------------------------------------------------------------------------------
*/
struct inode_operations {
	struct dentry* (*lookup)(struct vnode *dir,
					struct dentry *dentry, unsigned int flags);
	const char* (*follow_link)(struct dentry *dentry, void **cookie);
	int (*permission)(struct vnode *vnode, int mask);
	struct posix_acl* (*get_acl)(struct vnode *vnode, int type);
	int (*readlink)(struct dentry *dentry, char *buffer, int buflen);
	void (*put_link)(struct vnode *vnode, void *cookie);
	int (*create)(struct vnode *dir, struct dentry *dentry,
			umode_t mode, bool excl);
	int (*link)(struct dentry *old_dentry, struct vnode *dir,
			struct dentry *dentry);
	int (*unlink)(struct vnode *dir, struct dentry *dentry);
	int (*symlink)(struct vnode *dir, struct dentry *dentry,
			const char *symname);
	int (*mkdir)(struct vnode *dir, struct dentry *dentry , umode_t mode);
	int (*rmdir)(struct vnode *dir, struct dentry *dentry);
	int (*mknod)(struct vnode *dir, struct dentry *dentry,
			umode_t mode, dev_t dev);
	int (*rename)(struct vnode *old_dir, struct dentry *old_dentry,
			struct vnode *new_dir, struct dentry *new_dentry);
	int (*rename2)(struct vnode *old_dir, struct dentry *old_dentry,
			struct vnode *new_dir, struct dentry *new_dentry,
			unsigned int flags);
	int (*setattr)(struct dentry *dentry, struct iattr *iattr, int flags);
	int (*getattr)(struct vfsmount *mnt, struct dentry *dentry,
			struct stat *stat);
	ssize_t (*getxattr)(struct dentry *dentry, const char *name,
				void *value, size_t size);
	ssize_t (*listxattr)(struct dentry *dentry, char *buffer, size_t buflen);
	int (*removexattr)(struct dentry *dentry, const char *name);
	int (*fiemap)(struct vnode *vnode, struct fiemap_extent_info *fieinfo,
			uint64_t start, uint64_t len);
	int (*update_time)(struct vnode *vnode, struct timespec *now, int flags);
	int (*atomic_open)(struct vnode *dir, struct dentry *dentry,
				struct file *file, unsigned int open_flag,
				umode_t create_mode, int *opened);
	int (*tmpfile)(struct vnode *dir, struct dentry *dentry, umode_t mode);
	int (*set_acl)(struct vnode *vnode, struct posix_acl *acl, int type);
};

/*
----------------------------------------------------------------------------------
	virtual information node
----------------------------------------------------------------------------------
*/
struct vnode {
	/* -------------------------------------------------------------------- */
	/* vnode information							*/
	/* -------------------------------------------------------------------- */
	uint64_t		v_version;
	loff_t			v_size;
	blkcnt_t		v_blocks;
	uint32_t		v_generation;
	umode_t			v_mode;
	unsigned short		v_opflags;
	unsigned short		v_bytes;
	unsigned int		v_blkbits;
	/* -------------------------------------------------------------------- */
	/* file attributes							*/
	/* -------------------------------------------------------------------- */
	uid32_t			v_uid;
	gid32_t			v_gid;
	unsigned long		v_ino;
	unsigned int		v_flags;
	unsigned int		v_nlink;
	dev_t			v_rdev;
	struct timespec		v_atime;	/* access time			*/
	struct timespec		v_mtime;	/* file modification time	*/
	struct timespec		v_ctime;	/* vnode modification time	*/
	atomic_t		v_count;	/* usage counter		*/
	//struct address_space	v_data;
	/* -------------------------------------------------------------------- */
	/* fs specific								*/
	/* -------------------------------------------------------------------- */
	void			*v_private;
	/* -------------------------------------------------------------------- */
	/* other file type information or symbolic information			*/
	/* -------------------------------------------------------------------- */
	union {
		struct pipe_vnode_info	*v_pipe;
		struct block_device	*v_bdev;
		struct char_device	*v_cdev;
		char			*v_link;/* symbolic link information	*/
	};
	/* -------------------------------------------------------------------- */
	/* vnode relationships							*/
	/* -------------------------------------------------------------------- */
	struct list		v_dentry;
	struct list		v_devices;
	struct list		v_sb_list;
	struct list		v_lru;
	struct super_block	*v_sb;
	const struct inode_operations	*v_op;
	const struct file_operations	*v_fops;
	
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
 Funtion	:init_vnode
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize vnode management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_vnode(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_vnode_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a vnode cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void destroy_vnode_cache(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vnode_cache_alloc
 Input		:void
 Output		:void
 Return		:struct vnode*
 		 < vnode object >
 Description	:allocate a vnode object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct vnode* vnode_cache_alloc(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vnode_cache_free
 Input		:struct vnode *vnode
 		 < vnode to free >
 Output		:void
 Return		:void
 Description	:free a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void vnode_cache_free(struct vnode *vnode);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:INIT_VNODE
 Input		:struct vnode *vnode
 		 < vnode object to initialize >
 Output		:void
 Return		:void
 Description	:initialize vnode object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void INIT_VNODE(struct vnode *vnode);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_special_vnode
 Input		:struct vnode *vnode
 		 < special vnode >
 		 umode_t mode
 		 < mode >
 		 dev_t dev
 		 < device number >
 Output		:void
 Return		:void
 Description	:initialize a special vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void init_special_vnode(struct vnode *vnode, umode_t mode, dev_t dev);

/*
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mknod
 Input		:const char *pathname
 		 < pathname to create a special file >
 		 mode_t mode
 		 < file creation mode >
 		 dev_t dev
 		 < device number of a special file >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a special file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int mknod(const char *pathname, mode_t mode, dev_t dev);

/*
----------------------------------------------------------------------------------
	vfs vnode operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_lookup
 Input		:const char *pathname
 		 < path name to look up >
 		 struct file_name **found
 		 < looked up dentry result >
 		 unsigned int flags
 		 < lookup flags >
 Output		:struct file_name **found
 		 < looked up dentry result >
 Return		:int
 		 < result >
 Description	:lookup dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int
vfs_lookup(const char *pathname, struct file_name **found, unsigned int flags);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_mknod
 Input		:struct vnode *dir
 		 < a directory in which a special file is made >
 		 struct dentry *dentry
 		 < directory entry for a special file >
 		 umode_t mode
 		 < file open mode >
 		 dev_t dev
 		 < device number >
 Output		:void
 Return		:int
 		 < result >
 Description	:vfs make a special file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int
vfs_mknod(struct vnode *dir, struct dentry *dentry, umode_t mode, dev_t dev);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_create
 Input		:struct vnode *dir
 		 < a directory in which a file is made >
 		 struct dentry *dentry
 		 < directory entry for a file >
 		 umode_t mode
 		 < file open mode >
 		 bool excl
 		 < boolean : exclude create a file from other threads >
 Output		:void
 Return		:int
 		 < result >
 Description	:vfs create a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int
vfs_create(struct vnode *dir, struct dentry *dentry, umode_t mode, bool excl);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_mkdir
 Input		:struct vnode *dir
 		 < a directory in which a new directory is made >
 		 struct dentry *dentry
 		 < directory entry for a new directory >
 		 umode_t mode
 		 < directory creation mode >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a new directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int vfs_mkdir(struct vnode *dir, struct dentry *dentry, umode_t mode);

#endif	// __BK_FS_INODE_H__
