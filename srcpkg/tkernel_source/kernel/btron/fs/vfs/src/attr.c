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
#include <bk/unistd.h>
#include <bk/fs/vfs.h>
#include <bk/memory/access.h>
#include <bk/uapi/fcntl.h>
#include <bk/uapi/sys/stat.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int vfs_chmod(struct vnode *vnode, mode_t mode);
LOCAL int xchown(const char *pathname, uid_t owner, gid_t group, int flags);
LOCAL int vfs_chown(struct vnode *vnode, uid_t owner, gid_t group);

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


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:chmod
 Input		:const char *pathname
 		 < path name to change its permissions >
 		 mode_t mode
 		 < file permissions to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change permissions of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int chmod(const char *pathname, mode_t mode)
{
	struct file_name *fname;
	struct vnode *vnode;
	int err;
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)pathname, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(pathname, &fname, LOOKUP_ENTRY | LOOKUP_FOLLOW_LINK);
	
	if (err) {
		return(err);
	}
	
	vnode = fname->dentry->d_vnode;
	
	put_file_name(fname);
	
	err = vfs_chmod(vnode, mode);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fchmod
 Input		:int fd
 		 < open file descriptor to change its permissions >
 		 mode_t mode
 		 < file permissions to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change permissions of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fchmod(int fd, mode_t mode)
{
	struct file *filp;
	struct vnode *vnode;
	int err;
	
	if (UNLIKELY(get_soft_limit(RLIMIT_NOFILE) < fd)) {
		return(-EBADF);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	vnode = filp->f_vnode;
	
	err = vfs_chmod(vnode, mode);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fchmodat
 Input		:int dirfd
 		 < directory open file descriptor >
 		 const char *pathname
 		 < path name to change its permissions >
 		 mode_t mode
 		 < file permissions to change >
 		 int flags
 		 < flags >
 Output		:void
 Return		:int
 		 < result >
 Description	:change permissions of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags)
{
	int err;
	int lookup_flags = LOOKUP_ENTRY;
	struct path *dir_path;
	struct file_name *fname;
	struct vnode *vnode;
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)pathname, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	dir_path = NULL;
	
	err = get_dirfd_path(dirfd, &dir_path);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	if (flags & AT_SYMLINK_FOLLOW) {
		lookup_flags |= LOOKUP_FOLLOW_LINK;
	}
	
	err = vfs_lookup_at(dir_path, pathname, &fname, lookup_flags);
	
	if (UNLIKELY(err)) {
		goto failed_vfs_lookup;
	}
	
	put_file_name(fname);
	
	vnode = fname->dentry->d_vnode;
	
	err = vfs_chmod(vnode, mode);
	
	return(err);
	
failed_vfs_lookup:
	put_file_name(fname);
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:chown
 Input		:const char *pathname
 		 < path name to change its ownership >
 		 uid16_t owner
 		 < new owner >
 		 gid16_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int chown(const char *pathname, uid16_t owner, gid16_t group)
{
	uid_t owner32 = owner;
	gid_t group32 = group;
	
	return(xchown(pathname, owner32, group32,
					LOOKUP_ENTRY | LOOKUP_FOLLOW_LINK));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:chown32
 Input		:const char *pathname
 		 < path name to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int chown32(const char *pathname, uid_t owner, gid_t group)
{
	return(xchown(pathname, owner, group, LOOKUP_ENTRY | LOOKUP_FOLLOW_LINK));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fchown32
 Input		:int fd
 		 < open file descriptor >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fchown32(int fd, uid_t owner, gid_t group)
{
	struct file *filp;
	struct vnode *vnode;
	int err;
	
	if (UNLIKELY(get_soft_limit(RLIMIT_NOFILE) < fd)) {
		return(-EBADF);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	vnode = filp->f_vnode;
	
	err = vfs_chown(vnode, owner, group);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:lchown32
 Input		:const char *pathname
 		 < path name to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int lchown32(const char *pathname, uid_t owner, gid_t group)
{
	return(xchown(pathname, owner, group, LOOKUP_ENTRY));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fchownat
 Input		:int dirfd
 		 < open directory file descriptor >
 		 const char *pathname
 		 < path name to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 		 int flags
 		 < flags >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int
fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags)
{
	int err;
	int lookup_flags = LOOKUP_ENTRY | LOOKUP_FOLLOW_LINK;
	struct path *dir_path;
	struct file_name *fname;
	struct vnode *vnode;
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)pathname, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	dir_path = NULL;
	
	err = get_dirfd_path(dirfd, &dir_path);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	if (flags & AT_SYMLINK_NOFOLLOW) {
		lookup_flags &= ~LOOKUP_FOLLOW_LINK;
	}
	
	err = vfs_lookup_at(dir_path, pathname, &fname, lookup_flags);
	
	if (UNLIKELY(err)) {
		goto failed_vfs_lookup;
	}
	
	put_file_name(fname);
	
	vnode = fname->dentry->d_vnode;
	
	err = vfs_chown(vnode, owner, group);
	
	return(err);
	
failed_vfs_lookup:
	put_file_name(fname);
	return(err);
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
 Funtion	:vfs_chmod
 Input		:struct vnode *vnode
 		 < vnode to change its permissions >
 		 mode_t mode
 		 < file permissions to change >
 Output		:void
 Return		:int
 		 < result >
 Description	:change permissions of a vnode
==================================================================================
*/
LOCAL int vfs_chmod(struct vnode *vnode, mode_t mode)
{
	struct process *proc = get_current();
	mode_t change = vnode->v_mode & ~S_IFMT;
	
	change ^= mode;
	
	if (proc->euid) {
		/* ------------------------------------------------------------ */
		/* detect owner permissions					*/
		/* ------------------------------------------------------------ */
		if (vnode->v_uid != proc->euid) {
			if (vnode->v_gid != proc->egid) {
				return(-EACCES);
			}
		}
	}
	
	mode &= ~S_IFMT;
	vnode->v_mode = (vnode->v_mode & S_IFMT) | mode;
	
	return(0);
}

/*
==================================================================================
 Funtion	:xchown
 Input		:const char *pathname
 		 < path name to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 		 int flags
 		 < lookup flags >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a file
==================================================================================
*/
LOCAL int xchown(const char *pathname, uid_t owner, gid_t group, int flags)
{
	struct file_name *fname;
	struct vnode *vnode;
	int err;
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)pathname, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(pathname, &fname, flags);
	
	if (err) {
		return(err);
	}
	
	vnode = fname->dentry->d_vnode;
	
	put_file_name(fname);
	
	err = vfs_chown(vnode, owner, group);
	
	return(err);
}

/*
==================================================================================
 Funtion	:vfs_chown
 Input		:struct vnode *vnode
 		 < vnode to change its ownership >
 		 uid_t owner
 		 < new owner >
 		 gid_t group
 		 < new group >
 Output		:void
 Return		:int
 		 < result >
 Description	:change ownership of a vnode
==================================================================================
*/
LOCAL int vfs_chown(struct vnode *vnode, uid_t owner, gid_t group)
{
	struct process *proc = get_current();
	
	if (0 <= owner) {
		/* ------------------------------------------------------------ */
		/* detect owner permissions					*/
		/* ------------------------------------------------------------ */
		if (proc->euid) {
			if (vnode->v_uid != proc->euid) {
				return(-EACCES);
			}
		}
		vnode->v_uid = owner;
	}
	
	if (0 <= group) {
		/* ------------------------------------------------------------ */
		/* detect group permissions					*/
		/* ------------------------------------------------------------ */
		if (proc->egid) {
			if (vnode->v_gid != proc->egid) {
				return(-EACCES);
			}
		}
		vnode->v_gid = group;
	}
	
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
