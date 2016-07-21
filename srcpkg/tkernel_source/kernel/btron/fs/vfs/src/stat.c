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
#include <bk/uapi/sys/stat.h>

#include <tstdlib/round.h>

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
 Funtion	:stat64
 Input		:const char *path
 		 < file path to get its status >
 		 struct stat64_i386 *buf
 		 < stat structure >
 Output		:struct stat64_i386 *buf
 		 < stat structure >
 Return		:int
 		 < result >
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int stat64(const char *path, struct stat64_i386 *buf)
{
	struct file_name *fname;
	struct vnode *vnode;
	int err;
	
	//printf("stat64:path[%s] ", path);
	
	if (UNLIKELY(!path)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)buf, sizeof(struct stat64_i386), PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(path, &fname, LOOKUP_ENTRY | LOOKUP_FOLLOW_LINK);
	
	if (err) {
		//printf("err:%d\n", -err);
		return(err);
	} else {
		//printf("\n");
	}
	
	vnode = fname->dentry->d_vnode;
	
	put_file_name(fname);
	
	return(vfs_stat64(vnode, buf));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:lstat64
 Input		:const char *path
 		 < file path to get its status >
 		 struct stat64_i386 *buf
 		 < stat structure >
 Output		:struct stat64_i386 *buf
 		 < stat structure >
 Return		:int
 		 < result >
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int lstat64(const char *path, struct stat64_i386 *buf)
{
	struct file_name *fname;
	struct vnode *vnode;
	int err;
	
	//printf("lstat64:path[%s] ", path);
	
	if (UNLIKELY(!path)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)buf, sizeof(struct stat64_i386), PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(path, &fname, LOOKUP_ENTRY);
	
	if (err) {
		//printf("err:%d\n", -err);
		return(err);
	}
	
	vnode = fname->dentry->d_vnode;
	
	put_file_name(fname);
	
	err = vfs_stat64(vnode, buf);
	
	return(err);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fstat64
 Input		:int fd
 		 < open file descriptor >
 		 struct stat64_i386 *buf
 		 < stat structure >
 Output		:struct stat64_i386 *buf
 		 < stat structure >
 Return		:int
 		 < result >
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fstat64(int fd, struct stat64_i386 *buf)
{
	struct file *filp;
	struct vnode *vnode;
	int err;
	
	//printf("fstat64:fd=%d\n", fd);
	
	err = vm_check_access((void*)buf, sizeof(struct stat64_i386), PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(get_soft_limit(RLIMIT_NOFILE) < fd)) {
		return(-EBADF);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	vnode = filp->f_vnode;
	
	return(vfs_stat64(vnode, buf));
}

/*
----------------------------------------------------------------------------------
	vfs stat operations
----------------------------------------------------------------------------------
*/

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_stat64
 Input		:struct vnode *vnode
 		 < vnode to get its file status >
 		 struct stat64_i386 *buf
 		 < file status buffer to outpu >
 Output		:struct stat64_i386 *buf
 		 < file status buffer to outpu >
 Return		:int
 		 < result >
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vfs_stat64(struct vnode *vnode, struct stat64_i386 *buf)
{
	struct super_block *sb = vnode->v_sb;
	
	buf->st_dev		= sb->s_dev;
	buf->st_ino		= (ino_t)vnode->v_ino;
	buf->_st_ino		= (ino_t)vnode->v_ino;
	buf->st_mode		= vnode->v_mode;
	buf->st_nlink		= vnode->v_nlink;
	buf->st_uid		= vnode->v_uid;
	buf->st_gid		= vnode->v_gid;
	buf->st_rdev		= vnode->v_rdev;
	buf->st_size		= (off64_t)vnode->v_size;
	buf->st_atime		= vnode->v_atime.tv_sec;
	buf->st_atime_nsec	= 0;
	buf->st_mtime		= vnode->v_mtime.tv_sec;
	buf->st_mtime_nsec	= 0;
	buf->st_ctime		= vnode->v_ctime.tv_sec;
	buf->st_ctime_nsec	= 0;
	buf->st_blocks		= DIV_ROUNDUP(vnode->v_size, S_BLKSIZE);
	buf->st_blksize		= buf->st_blocks * S_BLKSIZE;

#if 0
	printf("st_dev:%d ", buf->st_dev);
	printf("st_ino:%d ", buf->st_ino);

	printf("st_mode:0x%08X ", buf->st_mode);
	printf("st_nlink:%d\n", buf->st_nlink);
	printf("st_uid:%d ", buf->st_uid);
	printf("st_gid:%d ", buf->st_gid);
	printf("st_rdev:%d\n", buf->st_rdev);
	printf("st_size:%d ", buf->st_size);
	printf("st_atime:%d ", buf->st_atime);
	printf("st_mtime:%d\n", buf->st_mtime);
	printf("st_ctime:%d ", buf->st_ctime);
	printf("st_blksize:%d ", buf->st_blksize);
	printf("st_blocks:%d\n", buf->st_blocks);

#endif
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
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
