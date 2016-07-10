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

#ifndef	__BK_FS_STATES_H__
#define	__BK_FS_STATES_H__

#include <bk/kernel.h>
#include <bk/fs/path.h>

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
----------------------------------------------------------------------------------
	fs states
----------------------------------------------------------------------------------
*/
struct fs_states {
	struct path		root;
	struct path		cwd;
	int			users;
	int			umask;
	int			in_exec;
	/* -------------------------------------------------------------------- */
	/* open file descriptors						*/
	/* -------------------------------------------------------------------- */
	atomic_t		fd_count;
	struct fdtable		*files;
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
 Funtion	:alloc_fs_states
 Input		:struct process *proc
 		 < process to allocate fs states >
 Output		:void
 Return		:int
 		 < result >
 Description	:allocate fs states
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int alloc_fs_states(struct process *proc);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_fs_states
 Input		:struct process *proc
 		 < process to free its fs states >
 Output		:void
 Return		:void
 Description	:free fs states
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_fs_states(struct process *proc);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_fs_states
 Input		:struct process *to
 		 < copy to >
 		 struct process *from
 		 < copy from >
 Output		:void
 Return		:void
 Description	:copy fs states
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void copy_fs_states(struct process *to, struct process *from);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_root_sb
 Input		:struct process *proc
 		 < process to get its root directory >
 Output		:void
 Return		:struct super_block*
 		 < super block of root >
 Description	:get root super block
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
#define get_root_sb(proc)	(vfs_get_root(proc)->dentry->d_sb)

/*
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getcwd
 Input		:char *buf
 		 < user space buffer to put cwd >
 		 unsigned long size
 		 < size of a buffer >
 Output		:char *buf
 		 < user space buffer to put cwd >
 Return		:long
 		 < result >
 Description	:get current working directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL long getcwd(char *buf, unsigned long size);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:chdir
 Input		:const char *path
 		 < path name >
 Output		:void
 Return		:int
 		 < result >
 Description	:change working directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int chdir(const char *path);

/*
----------------------------------------------------------------------------------
	vfs operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_get_cwd
 Input		:struct proces *proc
 		 < process to get its cwd >
 Output		:void
 Return		:struct path*
 		 < path of cwd >
 Description	:get current working directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct path* vfs_get_cwd(struct process *proc);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:set_cwd
 Input		:struct process *proc
 		 < process to set its cwd >
 		 struct vfsmount *mnt_cwd
 		 < vfs mount information to which cwd belongs >
 		 struct dentry *d_cwd
 		 < directory entry object of cwd >
 Output		:void
 Return		:void
 Description	:set current working directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void
vfs_set_cwd(struct process *proc, struct vfsmount *mnt_cwd, struct dentry *d_cwd);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_root
 Input		:struct process *proc
 		 < process to get its root directory >
 Output		:void
 Return		:struct path*
 		 < path of root >
 Description	:get root directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct path* vfs_get_root(struct process *proc);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:set_root
 Input		:struct process *proc
 		 < process to set its root path >
 		 struct vfsmount *mnt_root
 		 < vfs mount information to which root belongs >
 		 struct dentry *d_root
 		 < root director entry object >
 Output		:void
 Return		:void
 Description	:set root directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void vfs_set_root(struct process *proc,
				struct vfsmount *mnt_root, struct dentry *d_root);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_absolute_path
 Input		:struct dentry *dentry
 		 < dentry to get its absolute path >
 		 char *buf
 		 < buffer to store resolved absolute path >
 		 unsigned long size
 		 < the maximum size of the buffer >
 Output		:void
 Return		:long
 		 < result or the number of characters stored in buf on success >
 Description	:get absolute path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT long
vfs_absolute_path(struct dentry *dentry, char *buf, unsigned long size);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_chdir
 Input		:struct vfsmount *mnt
 		 < vfs mount information >
 		 struct dentry *dentry
 		 < directory dentry >
 		 struct vnode *dir
 		 < directory vnode >
 Output		:void
 Return		:int
 		 < result >
 Description	:vfs change current directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int
vfs_chdir(struct vfsmount *mnt, struct dentry *dentry, struct vnode *dir);

#endif	// __BK_FS_STATES_H__
