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

#ifndef	__BK_FS_MOUNT_H__
#define	__BK_FS_MOUNT_H__

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct mount;
struct dentry;
struct super_block;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	mounted infomation
----------------------------------------------------------------------------------
*/
#define	MNT_NOSUID		0x00000001
#define	MNT_NODEV		0x00000002
#define	MNT_NOEXEC		0x00000004
#define	MNT_NOATIME		0x00000008
#define	MNT_NODIRATIME		0x00000010
#define	MNT_RELATIME		0x00000020
#define	MNT_READONLY		0x00000040
#define	MNT_SHRINKABLE		0x00000100
#define	MNT_WRITE_HOLD		0x00000200
#define	MNT_SHARED		0x00001000
#define	MNT_UNBINDABLE		0x00002000
#define	MNT_INTERNAL		0x00004000
#define	MNT_LOCK_ATIME		0x00040000
#define	MNT_LOCK_NOEXEC		0x00080000
#define	MNT_LOCK_NOSUID		0x00100000
#define	MNT_LOCK_NODEV		0x00200000
#define	MNT_LOCK_READONLY	0x00400000
#define	MNT_LOCKED		0x00800000
#define	MNT_DOOMED		0x01000000
#define	MNT_SYNC_UMOUNT		0x02000000
#define	MNT_MARKED		0x04000000
#define	MNT_UMOUNT		0x08000000

#define	MNT_SHARED_MASK		(MNT_UNBINDABLE)
#define	MNT_USER_SETTABLE_MASK	(MNT_NOSUID | MNT_NODEV | MNT_NOEXEC |		\
				MNT_NOATIME | MNT_NODIRATIME | MNT_RELATIME |	\
				MNT_READONLY)
#define	MNT_ATIME_MASK		(MNT_NOATIME | MNT_NODIRATIME | MNT_RELATIME)
#define	MNT_INTERNAL_FLAGS	(MNT_SHARED | MNT_WRITE_HOLD | MNT_INTERNAL |	\
				MNT_DOOMED | MNT_SYNC_UMOUNT | MNT_MARKED)

struct vfsmount {
	struct dentry		*mnt_root;
	struct super_block	*mnt_sb;
	int			mnt_flags;
};

/*
----------------------------------------------------------------------------------
	mount object
----------------------------------------------------------------------------------
*/
struct mount {
	struct mount		*mnt_parent;
	struct dentry		*mnt_mountpoint;
	struct vfsmount		mnt;
	const char		*mnt_devname;
	int			mnt_count;
	struct list		list_mounts;
	struct list		mnt_child;
	struct list		mnt_instance;
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
 Funtion	:init_mount
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize mount management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_mount(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_mount_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a mount cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void destroy_mount_cache(void);

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
IMPORT struct mount* mount_cache_alloc(const char *devname);

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
IMPORT void mount_cache_free(struct mount *mount);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_mount
 Input		:struct vfsmount *vfsmount
 		 < vfs mount information >
 Output		:void
 Return		:struct mount*
 		 < mount information >
 Description	:get mount information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
LOCAL ALWAYS_INLINE struct mount* get_mount(struct vfsmount *vfsmount)
{
	return(container_of(vfsmount, struct mount, mnt));
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
IMPORT int mount_root_fs(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_vfsmount
 Input		:struct dentry *dentry
 		 < dentry which belongs to mount space to get >
 Output		:void
 Return		:struct vfsmount*
 		 < vfsmount of a mount space to which a dentry belongs >
 Description	:get vfsmount from dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct vfsmount* get_vfsmount(struct dentry *dentry);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_mount_nodev
 Input		:struct super_block *sb
 		 < super block object >
 Output		:void
 Return		:int
 		 < boolean result >
 Description	:test a file system has been mounted on no device
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int is_mount_nodev(struct super_block *sb);

#endif	// __BK_FS_MOUNT_H__
