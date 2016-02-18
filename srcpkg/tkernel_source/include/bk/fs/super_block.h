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

#ifndef	__BK_FS_SUPER_BLOCK_H__
#define	__BK_FS_SUPER_BLOCK_H__

#include <bk/kernel.h>

#include <t2ex/sys/types.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct file_system_type;
struct dentry_operations;
struct vnode;
struct super_block;
struct writeback_control;
struct statfs;
struct seq_file;
struct page;
struct shrink_control;


/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	operations for super-block
----------------------------------------------------------------------------------
*/
struct super_operations {
	struct vnode* (*alloc_vnode)(struct super_block *sb);
	void (*destroy_vnode)(struct vnode *vnode, int flags);
	int (*write_vnode)(struct vnode *vnode, struct writeback_control *wbc);
	int (*drop_vnode)(struct vnode *vnode);
	void (*evict_vnode)(struct vnode *vnode);
	void (*put_super)(struct super_block *sb);
	int (*sync_fs)(struct super_block *sb, int wait);
	int (*freeze_super)(struct super_block *sb);
	int (*freeze_fs)(struct super_block *sb);
	int (*thaw_super)(struct super_block *sb);
	int (*unfreeze_fs)(struct super_block *sb);
	int (*statfs)(struct dentry *dentry, struct statfs *buf);
	int (*remount_fs)(struct super_block *sb, int *flags, char *data);
	void (*umount_begin)(struct super_block *sb);
	int (*show_options)(struct seq_file *sf, struct dentry *dentry);
	int (*show_devname)(struct seq_file *sf, struct dentry *dentry);
	int (*show_path)(struct seq_file *sf, struct dentry *dentry);
	int (*show_stats)(struct seq_file *sf, struct dentry *dentry);
	ssize_t (*quota_read)(struct super_block *sb, int type, char *data,
				size_t len, loff_t off);
	ssize_t (*quota_write)(struct super_block *sb, int type, const char *data,
				size_t len, loff_t off);
	int (*bdev_try_to_free_page)(struct super_block *sb,
					struct page *page, gfp_t wait);
	long (*nr_cached_objects)(struct super_block *sb,
					struct shrink_control *sc);
	long (*free_cached_objects)(struct super_block *sb,
					struct shrink_control *sc);
};

/*
----------------------------------------------------------------------------------
	super-block object
----------------------------------------------------------------------------------
*/
#define	MAX_FILE_SIZE		(0UL - 1)

struct super_block {
	/* -------------------------------------------------------------------- */
	/* super-block information						*/
	/* -------------------------------------------------------------------- */
	struct list		s_list;
	unsigned long		s_blocksize;
	unsigned int		s_blocksize_bits;
	loff_t			s_maxbytes;	/* max file size		*/
	unsigned long		s_flags;
	unsigned long		s_iflags;
	unsigned long		s_magic;
	int			s_count;
	atomic_t		s_active;
	unsigned int		s_quota_types;	/* bitmask of supported quota	*/
	char			s_id[32];	/* informational name		*/
	uint8_t			s_uuid[16];
	dev_t			s_dev;
	atomic_t		s_remove_count;
	int			s_readonly_remount;
	/* -------------------------------------------------------------------- */
	/* fs specific								*/
	/* -------------------------------------------------------------------- */
	void			*s_fs_info;
	/* -------------------------------------------------------------------- */
	/* super-block relationships						*/
	/* -------------------------------------------------------------------- */
	//struct backing_dev_info	*s_bdev;
	struct list		s_mounts;
	struct list		s_dentry_lru;
	struct list		s_inode_lru;
	
	struct file_system_type	*s_type;
	struct dentry		*s_root;
	struct list		s_inodes;

	const struct super_operations	*s_op;
	const struct dentry_operations	*s_d_op;/* default d_op for dentries	*/
};

/*
----------------------------------------------------------------------------------
	fs independent mount flags
----------------------------------------------------------------------------------
*/
#define	MS_RDONLY		0x00000001	/* mount read-only		*/
#define	MS_NOSUID		0x00000002	/* ignore suid and sgid bits	*/
#define	MS_NODEV		0x00000004	/* disallow access to device	*/
						/* special files		*/
#define	MS_NOEXEC		0x00000008	/* disallow program execution	*/
#define	MS_SYNCHRONOUS		0x00000010	/* writes are synced at once	*/
#define	MS_REMOUNT		0x00000020	/* alter flags of a mounted fs	*/
#define	MS_MANDLOCK		0x00000040	/* allow mandatory locks on fs	*/
#define	MS_DIRSYNC		0x00000080	/* directory modifications are	*/
						/* synchronous			*/

#define	MS_NOATIME		0x00000400	/* do not update access time	*/
#define	MS_NODIRATIME		0x00000800	/* do not update directory atime*/
#define	MS_BIND			0x00001000
#define	MS_MOVE			0x00002000
#define	MS_REC			0x00004000
#define	MS_VERBOSE		0x00008000	/* deprecated			*/
#define	MS_SILENT		MS_VERBOSE
#define	MS_POSIXACL		0x00010000	/* vfs does not apply the umask	*/
#define	MS_UNBINDABLE		0x00020000	/* change to unbindable		*/
#define	MS_PRIVATE		0x00040000	/* change to private		*/
#define	MS_SLAVE		0x00080000	/* change to slave		*/
#define	MS_SHARED		0x00100000	/* change to shared		*/
#define	MS_RELATIME		0x00200000	/* update atime relative to	*/
						/* mtime/ctime			*/
#define	MS_KERNMOUNT		0x00400000	/* kern_mount call		*/
#define	MS_I_VERSION		0x00800000	/* update i_version		*/
#define	MS_STRICTATIME		0x01000000	/* always perform atime updates	*/
#define	MS_LAZYTIME		0x02000000	/* update the on-disc [acm]times*/
						/* lazily			*/


#define	MS_NOSEC		0x10000000
#define	MS_BORN			0x20000000
#define	MS_ACTIVE		0x40000000
#define	MS_NOUSER		0x80000000

#define	MS_RMT_MASK		(MS_RDONLY | MS_SYNCHRONOUS | MS_MANDLOCK |	\
				MS_I_VERSION | MS_LAZYTIME)

/*
----------------------------------------------------------------------------------
	vnode flags
----------------------------------------------------------------------------------
*/
#define	S_SYNC		0x00000001	/* writes are synced at once		*/
#define	S_NOATIME	0x00000002	/* do not update access times		*/
#define	S_APPEND	0x00000004	/* append-only file			*/
#define	S_IMMUTABLE	0x00000008	/* immutable file			*/
#define	S_DEAD		0x00000010	/* removed, but still open directory	*/
#define	S_NOQUOTA	0x00000020	/* vnode is not counted to quota	*/
#define	S_DIRSYNC	0x00000040	/* directory modifications are sync	*/
#define	S_NOCMTIME	0x00000080	/* do not update file c/mtime		*/
#define	S_SWAPFILE	0x00000100	/* do not truncate: swapon got its bmaps*/
#define	S_PRIVATE	0x00000200	/* vnode has fs-internal		*/
#define	S_IMA		0x00000400	/* vnode has an associated ima struct	*/
#define	S_AUTOMOUNT	0x00000800	/* automount/referral quasi-directory	*/
#define	S_NOSEC		0x00001000	/* no suid or xattr security attributes	*/
#define	S_DAX		0x00002000	/* direct access, avoiding page caches	*/

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
 Funtion	:init_super_block
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize super block management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_super_block(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_super_block_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a cache of super block
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void destroy_super_block_cache(void);

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
IMPORT struct super_block*
sb_cache_alloc(struct file_system_type *type, int flags);

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
IMPORT void sb_cache_free(struct super_block *sb);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_alloc_vnode
 Input		:struct super_block *sb
 		 < super block object >
 Output		:void
 Return		:struct vnode*
 		 < an allocated vnode >
 Description	:allocate a vfs vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct vnode* vfs_alloc_vnode(struct super_block *sb);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_destroy_vnode
 Input		:struct vnode *vnode
 		 < vnode to free >
 		 int flags
 		 < flags >
 Output		:void
 Return		:void
 Description	:free a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void vfs_destroy_vnode(struct vnode *vnode, int flags);

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
IMPORT void vfs_free_vnode(struct vnode *vnode);

#endif	// __BK_FS_SUPER_BLOCK_H__
