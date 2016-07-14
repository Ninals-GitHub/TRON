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
#include <bk/fs/dir.h>

#include <bk/uapi/sys/stat.h>

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
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:dentry_file_type
 Input		:struct dentry *dentry
 		 < dentry to get its file type >
 Output		:void
 Return		:unsigned char
 		 < a file type >
 Description	:get file type from a dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT unsigned char dentry_file_type(struct dentry *dentry)
{
	struct vnode *vnode = dentry->d_vnode;
	unsigned char file_type;
	
	if (UNLIKELY(!vnode)) {
		return(DT_UNKNOWN);
	}
	
	file_type = (unsigned char)((vnode->v_mode & S_IFMT) >> S_IF_SHIFT);
	
	return(file_type);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:show_dir_path
 Input		:const char *pathname
 		 < directory path name to show >
 Output		:void
 Return		:void
 Description	:show sub directory of the pathname
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void show_dir_path(const char *pathname)
{
	struct file_name *filename;
	int err;
	
	struct dentry *dentry;
	
	err = vfs_lookup(pathname, &filename, 0);
	
	if (err) {
		return;
	}
	
	dentry = filename->parent;
	
	put_file_name(filename);

	show_subdirs(dentry);
}

/*
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getdents64
 Input		:int fd
 		 < open file descriptor >
 		 struct linux_dirent64 *dirp
 		 < directory entries for linux >
 		 unsigned int count
 		 < size of a buffer >
 Output		:struct linux_dirent *dirp
 		 < directory entries for linux >
 Return		:int
 		 < result >
 Description	:get directory entries
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int getdents64(int fd, struct linux_dirent64 *dirp, unsigned int count)
{
	struct dentry *dir;
	struct dentry *dentry;
	struct dentry *temp;
	struct file *filp;
	loff_t offset = 0;
	int write_len = 0;
	unsigned short d_reclen;
	char dot_name[] = ".\0\0";
	char dot_dot_name[] = "..\0";
	int err;
	
	//printf("getdents64\n");
	
	if (UNLIKELY(!dirp)) {
		printf("1:error:%s\n", __func__);
		return(-EFAULT);
	}
	
	if (UNLIKELY(!count)) {
		printf("2:error:%s\n", __func__);
		return(0);
	}
	
	if (count < (dent64_reclen(sizeof(dot_name)) +
				dent64_reclen(sizeof(dot_dot_name)))) {
		printf("3:error:%s\n", __func__);
		return(0);
	}
	
	err = is_open_file(fd);
	
	if (UNLIKELY(err)) {
		printf("4:error:%s\n", __func__);
		return(err);
	}
	
	err = vm_check_access((void*)dirp, count, PROT_WRITE);
	
	if (UNLIKELY(err)) {
		printf("5:error:%s\n", __func__);
		return(err);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		printf("6:error:%s\n", __func__);
		return(-EBADF);
	}
	
	dir = filp->f_path.dentry;
	
	if (UNLIKELY(filp->f_private == (void*)dir)) {
		/* ------------------------------------------------------------ */
		/* reach to eof							*/
		/* ------------------------------------------------------------ */
		//printf("is over!!!!!!!!!!! dir num = %d\n", (int)filp->f_pos);
		return(0);
	}
	
	/* -------------------------------------------------------------------- */
	/* make dot entry							*/
	/* -------------------------------------------------------------------- */
	if (filp->f_pos == 0) {
		dirp->d_ino = (ino64_t)dir->d_vnode->v_ino;
		dirp->d_reclen = dent64_reclen(sizeof(dot_name));
		offset += dent64_offset(offset, dirp->d_reclen);
		dirp->d_off = offset;
		dirp->d_type = dentry_file_type(dir);
		memcpy(dirp->d_name, dot_name, sizeof(dot_name));
		
		write_len += dirp->d_reclen;
		filp->f_pos++;
		
		dirp = dent64_next(dirp, dirp->d_reclen);
	}
	
	/* -------------------------------------------------------------------- */
	/* make dot dot entry							*/
	/* -------------------------------------------------------------------- */
	if (filp->f_pos == 1) {
		dirp->d_ino = (ino64_t)dir->d_parent->d_vnode->v_ino;
		dirp->d_reclen = dent64_reclen(sizeof(dot_dot_name));
		offset += dent64_offset(offset, dirp->d_reclen);
		dirp->d_off = offset;
		dirp->d_type = dentry_file_type(dir);
		memcpy(dirp->d_name, dot_dot_name, sizeof(dot_dot_name));
		
		write_len += dirp->d_reclen;
		filp->f_pos++;
		
		dirp = dent64_next(dirp, dirp->d_reclen);
	}
	
	if (UNLIKELY(count <= write_len)) {
		goto out;
	}
	
	/* -------------------------------------------------------------------- */
	/* make entries for children						*/
	/* -------------------------------------------------------------------- */
	if (UNLIKELY(filp->f_private)) {
		/* ------------------------------------------------------------ */
		/* get a cookie							*/
		/* ------------------------------------------------------------ */
		dentry = (struct dentry*)filp->f_private;
	} else {
		if (UNLIKELY(is_empty_list(&dir->d_subdirs))) {
			/* ---------------------------------------------------- */
			/* finish on next getdents				*/
			/* ---------------------------------------------------- */
			filp->f_private = (void*)dir;
			goto out;
		} else  {
			dentry = get_entry(dir->d_subdirs.next,
						typeof(*dentry),
						d_child);
		}
	}
	
	/* ------------------------------------------------------------ */
	/* make entries for children					*/
	/* ------------------------------------------------------------ */
	list_for_each_entry_safe_from(dentry, temp,
					&dir->d_subdirs, d_child) {
		if (dentry->d_vnode) {
			d_reclen = dent64_reclen(dentry->d_name.len);
			
			if (UNLIKELY(count <= (write_len + d_reclen))) {
				filp->f_private = (void*)dentry;
				goto out;
			}
			
			dirp->d_ino = (ino64_t)dentry->d_vnode->v_ino;
			dirp->d_reclen = d_reclen;
			offset += dent64_offset(offset, dirp->d_reclen);
			dirp->d_off = offset;
			dirp->d_type = dentry_file_type(dentry);
			memcpy(dirp->d_name, dentry_name(dentry),
					dentry->d_name.len);
			dirp->d_name[dentry->d_name.len] = '\0';
			
			write_len += dirp->d_reclen;
			filp->f_pos++;
			
			dirp = dent64_next(dirp, dirp->d_reclen);
		}
		
		
		if (UNLIKELY(dentry->d_child.next == &dir->d_subdirs)) {
			/* ---------------------------------------------------- */
			/* finish on next getdents				*/
			/* ---------------------------------------------------- */
			filp->f_private = (void*)dir;
			goto out;
		}
	}
	
out:
	return(write_len);
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
