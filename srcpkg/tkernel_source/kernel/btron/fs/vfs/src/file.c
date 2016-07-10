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
#include <bk/fs/vdso.h>
#include <bk/memory/slab.h>
#include <bk/memory/vm.h>
#include <bk/uapi/sys/stat.h>
#include <bk/uapi/fcntl.h>

#include <t2ex/limits.h>
//#include <t2ex/sys/fcntl.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL struct file* file_cache_alloc(void);
LOCAL void file_cache_free(struct file *file);
LOCAL struct fdtable* fdtable_cache_alloc(void);
LOCAL void fdtable_cache_free(struct fdtable *fdtable);
LOCAL struct fdtable* alloc_fdtable(size_t new_size);
LOCAL ALWAYS_INLINE struct fdtable* get_fdtable(struct process *proc);
LOCAL ALWAYS_INLINE void put_fdtable(struct process *proc);
LOCAL ALWAYS_INLINE
void install_fdtable(struct process *proc, struct fdtable *fdtable);
LOCAL int alloc_fd(struct process *proc);
LOCAL void free_fd(struct process *proc, int fd);
LOCAL ALWAYS_INLINE int
install_file(struct process *proc, struct file *file);
LOCAL int
xopenat(struct path *dir_path, const char *pathname, int flags, mode_t mode);

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
LOCAL struct kmem_cache *file_cache;
LOCAL const char file_cache_name[] = "file_cache";

LOCAL struct kmem_cache *fdtable_cache;
LOCAL const char fdtable_cache_name[] = "fdtable_cache";

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_files
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize file object management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_files(void)
{
	file_cache = kmem_cache_create(file_cache_name,
					sizeof(struct file), 0, 0, NULL);
	
	if (UNLIKELY(!file_cache)) {
		vd_printf("error:file_cache\n");
		return(-ENOMEM);
	}
	
	fdtable_cache = kmem_cache_create(fdtable_cache_name,
					sizeof(struct fdtable), 0, 0, NULL);
	
	if (UNLIKELY(!fdtable_cache)) {
		vd_printf("error:fdtable_cache\n");
		return(-ENOMEM);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_files_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a file cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_files_cache(void)
{
	kmem_cache_destroy(file_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:extend_fdtable
 Input		:struct process *proc
 		 < process to extend its fd table >
 Output		:void
 Return		:int
 		 < result >
 Description	:extend a fd table or allocate a fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int extend_fdtable(struct process *proc)
{
	struct fdtable *new_table;
	struct fdtable *old_table;
	size_t new_size;
	unsigned long max_nofile;
	
	/* -------------------------------------------------------------------- */
	/* :begin critical section						*/
	/* -------------------------------------------------------------------- */
	BEGIN_CRITICAL_SECTION;
	if (UNLIKELY(!has_fdtable(proc))) {
		/* ------------------------------------------------------------ */
		/* allocate a new fd table					*/
		/* ------------------------------------------------------------ */
		new_table = alloc_fdtable(DEFAULT_NR_FDS);
		
		if (UNLIKELY(!new_table)) {
			goto error_out;
		}
		
		goto success_out;
	}
	
	max_nofile = get_soft_limit(RLIMIT_NOFILE);
	
	old_table = get_fdtable(proc);
	
	if (UNLIKELY(max_nofile < old_table->max_fds)) {
		return(-EMFILE);
	} else {
		new_size = old_table->max_fds + INC_NR_FDS;
	}
	
	if (UNLIKELY(max_nofile < new_size)) {
		new_size = max_nofile;
	}

	new_table = alloc_fdtable(new_size);

	if (!new_table) {
error_out:
		put_fdtable(proc);
		OUT_CRITICAL_SECTION;
		return(-ENOMEM);
	}
	
	copy_fdtable(new_table, old_table);
	free_fdtable(old_table);
success_out:
	install_fdtable(proc, new_table);
	put_fdtable(proc);
	/* -------------------------------------------------------------------- */
	/* :end critical section						*/
	/* -------------------------------------------------------------------- */
	END_CRITICAL_SECTION;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_fdtable
 Input		:struct fdtable *fdtable
 		 < a fd table to free >
 Output		:void
 Return		:void
 Description	:free a fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_fdtable(struct fdtable *fdtable)
{
	if (UNLIKELY(!fdtable)) {
		return;
	}
	
	kfree(fdtable->close_on_exec);
	kfree(fdtable->open_fds);
	kfree(fdtable->fd);
	fdtable_cache_free(fdtable);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_fdtable
 Input		:struct fdtable *to
 		 < copy to >
 		 struct fdtable *from
 		 < copy form >
 Output		:void
 Return		:void
 Description	:copy fd table
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void copy_fdtable(struct fdtable *to, struct fdtable *from)
{
	memcpy(to->fd, from->fd, sizeof(struct file*) * from->max_fds);
	memcpy(to->close_on_exec, from->close_on_exec,
		sizeof(unsigned long) * from->max_fds / 8);
	memcpy(to->open_fds, from->open_fds,
		sizeof(unsigned long) * from->max_fds / 8);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_file
 Input		:struct process *proc
 		 < process to be freed a file >
 		 int fd
 		 < file descriptor corresponds to a open file object to free >
 Output		:void
 Return		:void
 Description	:free file and fd
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void free_file(struct process *proc, int fd)
{
	struct fdtable *fdtable;
	struct file *file;
	
	free_fd(proc, fd);
	
	/* -------------------------------------------------------------------- */
	/* :begin critical section						*/
	/* -------------------------------------------------------------------- */
	BEGIN_CRITICAL_SECTION;
	fdtable = get_fdtable(proc);
	
	file = fdtable->fd[fd];
	
	fdtable->fd[fd] = NULL;
	
	atomic_long_dec(&file->f_count);
	
	if (!atomic_long_read(&file->f_count)) {
		file_cache_free(file);
	}
	
	fdtable->next_fd = fd;
	
	put_fdtable(proc);
	/* -------------------------------------------------------------------- */
	/* :end critical section						*/
	/* -------------------------------------------------------------------- */
	END_CRITICAL_SECTION;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_open_file
 Input		:int fd
 		 < open file descriptor >
 Output		:void
 Return		:struct file*
 		 < open file object >
 Description	:get opened file from a current process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct file* get_open_file(int fd)
{
	struct process *proc = get_current();
	struct fdtable *files = proc->fs.files;
	
	return(files->fd[fd]);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_dirfd_path
 Input		:int dirfd
 		 < open directory file descriptor >
 		 struct path **dir_path
 		 < directory path >
 Output		:struct path **dir_path
 		 < directory path >
 Return		:int
 		 < result >
 Description	:get directory path from directory file descriptor
 		 Before calling the function, must be set NULL to *dir_path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int get_dirfd_path(int dirfd, struct path **dir_path)
{
	struct vnode *dir_vnode;
	struct file *filp;
	int err;
	
	*dir_path = NULL;
	
	if (dirfd != AT_FDCWD) {
		err = is_open_file(dirfd);
		
		if (UNLIKELY(err)) {
			return(-EBADF);
		}
		
		filp = get_open_file(dirfd);
		
		if (UNLIKELY(!filp)) {
			return(-EBADF);
		}
		
		dir_vnode = filp->f_vnode;
		
		if (UNLIKELY(!S_ISDIR(dir_vnode->v_mode))) {
			return(-ENOTDIR);
		}
		
		*dir_path = &filp->f_path;
	}
	
	return(0);
}



/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_open_file
 Input		:int fd
 		 < open file descriptor to be checked >
 Output		:void
 Return		:int
 		 < result >
 Description	:check whether fd is a opened file descriptor or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int is_open_file(int fd)
{
	struct process *proc = get_current();
	struct fdtable *files = proc->fs.files;
	
	if (fd < 0) {
		return(-EBADF);
	}
	
	if (UNLIKELY(get_soft_limit(RLIMIT_NOFILE) < fd)) {
		return(-EBADF);
	}
	
	if (files->max_fds < fd) {
		return(-EBADF);
	}
	
	return(!files->fd[fd]);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_open_dir
 Input		:int dirfd
 		 < open directory file descriptor to be checked >
 Output		:void
 Return		:int
 		 < result >
 Description	:check whether fd is a opened directory file descriptor or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int is_open_dir(int fd)
{
	if (fd == AT_FDCWD) {
		return(0);
	}
	
	return(is_open_file(fd));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:has_fdtable
 Input		:struct process *proc
 		 < process to test whether it has already a fd table >
 Output		:void
 Return		:int
 		 < boolean >
 Description	:test whether the process has already a fd table or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int has_fdtable(struct process *proc)
{
	struct fs_states *fs = &proc->fs;
	return((int)fs->files);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:open_exe_file
 Input		:const char *filename
 		 < a file to open >
 Output		:void
 Return		:int
 		 < result or fd >
 Description	:open a executable file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int open_exe_file(const char *filename)
{
	struct file_name *fname;
	struct dentry *dentry;
	struct file *filp;
	struct process *proc;
	int err;
	int fd;
	
	err = vfs_lookup(filename, &fname, LOOKUP_ENTRY);
	
	if (err) {
		return(err);
	}
	
	dentry = fname->dentry;
	
	put_file_name(fname);
	
	if (!dentry->d_vnode) {
		return(-ENOENT);
	}
	
	if (S_ISDIR(dentry->d_vnode->v_mode)) {
		return(-EISDIR);
	}
	
	if (!S_ISREG(dentry->d_vnode->v_mode)) {
		return(-EACCES);
	}
	
	proc = get_current();
	
	filp = file_cache_alloc();
	
	if (UNLIKELY(!filp)) {
		err = -ENOMEM;
		panic("unexpected error at %s\n", __func__);
		goto failed_file_cache_alloc;
	}
	
	filp->f_vnode = dentry->d_vnode;
	filp->f_fops = dentry->d_vnode->v_fops;
	filp->f_path.mnt = vfs_get_cwd(proc)->mnt;
	filp->f_path.dentry = dentry;
	
	//vd_printf("open_exe_file:%s\n", filename);
	
	err = vfs_open(dentry->d_vnode, filp);
	
	if (err) {
		vd_printf("open:failed vfs_open\n");
		err = -ENOENT;
		goto failed_vfs_open;
	}
	
	fd = install_file(proc, filp);
	
	vd_printf("install_file:%s fd:%d\n", filename, fd);
	
	if (fd < 0) {
		vd_printf("open:failed install_file\n");
		err = fd;
		goto failed_install_file;
	}
	
	return(fd);
	
failed_file_cache_alloc:
	put_file_name(fname);
	return(err);
failed_vfs_open:
failed_install_file:
	file_cache_free(filp);
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:open_vdso_file
 Input		:void
 Output		:void
 Return		:int
 		 < result or fd >
 Description	:open a vdso file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int open_vdso_file(void)
{
	struct file *filp;
	struct vnode *vdso_vnode = get_vdso_vnode();
	struct process *proc = get_current();
	int fd;
	int err;
	
	filp = file_cache_alloc();
	
	if (UNLIKELY(!filp)) {
		err = -ENOMEM;
		panic("unexpected error at %s\n", __func__);
		goto failed_file_cache_alloc;
	}
	
	filp->f_vnode = vdso_vnode;
	filp->f_fops = vdso_vnode->v_fops;
	filp->f_path.mnt = vfs_get_cwd(proc)->mnt;
	filp->f_path.dentry = vfs_get_cwd(proc)->dentry;
	
	printf("open_vdso_file\n");
	
	err = vfs_open(vdso_vnode, filp);
	
	if (err) {
		printf("open:failed vfs_open\n");
		err = -ENOENT;
		goto failed_vfs_open;
	}
	
	fd = install_file(proc, filp);
	
	if (fd < 0) {
		vd_printf("open:failed install_file at %s\n", __func__);
		err = fd;
		goto failed_install_file;
	}
	
	return(fd);

failed_install_file:
failed_vfs_open:
	file_cache_free(filp);
failed_file_cache_alloc:
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kwrite
 Input		:int fd
 		 < open file descriptor >
 		 const void *buf
 		 < buffer to write >
 		 size_t count
 		 < size of buffer >
 Output		:void
 Return		:ssize_t
 		 < result >
 Description	:write data to a file in kernel space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ssize_t kwrite(int fd, const void *buf, size_t count)
{
	struct file *filp;
	int err;
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(!count)) {
		return(0);
	}
	
	err = is_open_file(fd);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	return(vfs_write(filp, buf, count, &filp->f_pos));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kpread
 Input		:int fd
 		 < open file descriptor >
 		 void *buf
 		 < buffer to output >
 		 size_t count
 		 < read length >
 		 loff_t offset
 		 < file offset >
 Output		:void *buf
 		 < buffer to output >
 Return		:ssize_t
 		 < result >
 Description	:read data from a file at offset in kernel space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ssize_t kpread(int fd, void *buf, size_t count, loff_t offset)
{
	struct file *filp;
	loff_t new_offset = offset;
	int err;
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(!count)) {
		return(0);
	}
	
	err = is_open_file(fd);
	
	if (UNLIKELY(err)) {
		vd_printf("error:kpread:%d\n", err);
		return(err);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	return(vfs_read(filp, buf, count, &new_offset));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kopen
 Input		:const char *pathname
 		 < path name to open >
 		 int falgs
 		 < file open flags >
 		 mode_t mode
 		 < file open mode >
 Output		:void
 Return		:int
 		 < open file descriptor >
 Description	:open a file in kernel space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int kopen(const char *pathname, int flags, mode_t mode)
{
	return(xopenat(NULL, pathname, flags, mode));
}

/*
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:read
 Input		:int fd
 		 < open file descriptor >
 		 void *buf
 		 < buffer to output >
 		 size_t count
 		 < read length >
 Output		:void *buf
 		 < buffer to output >
 Return		:ssize_t
 		 < result >
 Description	:read data from a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL ssize_t read(int fd, void *buf, size_t count)
{
	struct file *filp;
	int err;
	
//	printf("fd=%d, ", fd);
//	printf("*buf=0x%08X, ", buf);
//	printf("count = 0x%08X\n", count);
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(!count)) {
		return(0);
	}
	
	err = vm_check_access((void*)buf, count, PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = is_open_file(fd);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	return(vfs_read(filp, buf, count, &filp->f_pos));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:pread
 Input		:int fd
 		 < open file descriptor >
 		 void *buf
 		 < buffer to output >
 		 size_t count
 		 < read length >
 		 loff_t offset
 		 < file offset >
 Output		:void *buf
 		 < buffer to output >
 Return		:ssize_t
 		 < result >
 Description	:read data from a file at offset
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL ssize_t pread(int fd, void *buf, size_t count, loff_t offset)
{
	struct file *filp;
	loff_t new_offset = offset;
	int err;
	
//	printf("pread:fd=%d, ", fd);
//	printf("*buf=0x%08X, ", buf);
//	printf("count=0x%08X, ", count);
//	printf("offset=0x%08X\n", offset);
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(!count)) {
		return(0);
	}
	
	err = is_open_file(fd);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	err = vm_check_access((void*)buf, count, PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	return(vfs_read(filp, buf, count, &new_offset));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:write
 Input		:int fd
 		 < open file descriptor >
 		 const void *buf
 		 < buffer to write >
 		 size_t count
 		 < size of buffer >
 Output		:void
 Return		:ssize_t
 		 < result >
 Description	:write data to a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL ssize_t write(int fd, const void *buf, size_t count)
{
	struct file *filp;
	int err;
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(!count)) {
		return(0);
	}
	//printf("write buf:0x%08X count:%d\n", buf, count);
	
	err = is_open_file(fd);
	
	if (UNLIKELY(err)) {
		vd_printf("error:write\n");
		return(err);
	}
	
	err = vm_check_access((void*)buf, count, PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	return(vfs_write(filp, buf, count, &filp->f_pos));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:writev
 Input		:int fd
 		 < open file descriptor >
 		 const struct iovec *iov
 		 < io vector to write >
 		 int iovcnt
 		 < number of io buffer >
 Output		:void
 Return		:ssize_t
 		 < size of written bytes > 
 Description	:write buffers to an open file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
	struct file *filp;
	int i;
	int err;
	ssize_t len = 0;

	//printf("writev start-------[%d]\n", iovcnt);
	
	if (UNLIKELY(!iov)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(iovcnt < 0 || UIO_MAXIOV < iovcnt)) {
		return(-EINVAL);
	}
	
	err = vm_check_access((void*)iov,
				sizeof(struct iovec*) * iovcnt, PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = is_open_file(fd);
	
	if (UNLIKELY(err)) {
		return(err);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	for (i = 0;i < iovcnt;i++) {
		ssize_t write_len;
		//printf("fd:%d iov_base:0x%08X iov_len:0x%08X\n", fd, iov[i].iov_base, iov[i].iov_len);
		//write_len = vfs_write(filp, iov[i].iov_base, iov[i].iov_len, &filp->f_pos);
		
		err = vm_check_access((void*)iov[i].iov_base,
					iov[i].iov_len, PROT_READ);
		
		if (UNLIKELY(err)) {
			return(-EFAULT);
		}
		
		write_len = write(fd, iov[i].iov_base, iov[i].iov_len);
		if (UNLIKELY(write_len < 0)) {
			return(write_len);
		}
		
		len += write_len;
		
		if (UNLIKELY(len < 0)) {
			return(-EINVAL);
		}
	}
	
	//printf("writev end---------\n");
	
	return(len);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:open
 Input		:const char *pathname
 		 < path name to open >
 		 int falgs
 		 < file open flags >
 		 mode_t mode
 		 < file open mode >
 Output		:void
 Return		:int
 		 < open file descriptor >
 Description	:open a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int open(const char *pathname, int flags, mode_t mode)
{
	int err;
	
	//printf("open:%s\n", pathname);
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)pathname, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	return(xopenat(NULL, pathname, flags, mode));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:openat
 Input		:int dirfd
 		 < open directory file descriptor >
 		 const char *pathname
 		 < path name to open >
 		 int falgs
 		 < file open flags >
 		 mode_t mode
 		 < file open mode >
 Output		:void
 Return		:int
 		 < open file descriptor >
 Description	:open a file relative to a directory file descriptor
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int openat(int dirfd, const char *pathname, int flags, mode_t mode)
{
	int err;
	struct path *dir_path;
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = is_open_dir(dirfd);
	
	if (UNLIKELY(err)) {
		return(err);
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
	
	
	err = xopenat(dir_path, pathname, flags, mode);
	
	//printf("openat:%s [%d]\n", pathname, err);
	
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:close
 Input		:int fd
 		 < open file descriptor to close >
 Output		:void
 Return		:int
 		 < result >
 Description	:close a file descriptor
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int close(int fd)
{
	struct process *proc = get_current();
	struct fdtable *fdtable = get_fdtable(proc);
	struct file *filp;
	int err = 0;
	
	//printf("close:%d\n", fd);
	
	if (UNLIKELY(!fdtable)) {
		printf("close:error[1]\n");
		err = -EBADF;
		goto error_out;
	}
	
	if (UNLIKELY(fdtable->max_fds < fd)) {
		printf("close:error[2]\n");
		err = -EBADF;
		goto error_out;
	}
	
	/* -------------------------------------------------------------------- */
	/* :begin critical section						*/
	/* -------------------------------------------------------------------- */
	BEGIN_CRITICAL_SECTION {
	free_fd(proc, fd);
	
	filp = fdtable->fd[fd];
	fdtable->fd[fd] = NULL;
	
	file_cache_free(filp);
	/* -------------------------------------------------------------------- */
	/* :end critical section						*/
	/* -------------------------------------------------------------------- */
	} END_CRITICAL_SECTION;
	
error_out:
	put_fdtable(proc);
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:lseek64
 Input		:int fd
 		 < open file descriptor >
 		 loff_t offset64
 		 < file offset >
 		 int whence
 		 < seek operation >
 Output		:void
 Return		:loff_t
 		 < offset after seeking >
 Description	:seek a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL loff_t lseek64(int fd, loff_t offset64, int whence)
{
	panic("%s is not implemented\n", __func__);
	return(offset64);
}

/*
----------------------------------------------------------------------------------
	vfs open file operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_read
 Input		:struct file *filp
 		 < open file object >
 		 char *buf
 		 < user buffer to output >
 		 size_t len
 		 < read length >
 		 loff_t *ppos
 		 < file read offset >
 Output		:char *buf
 		 < user buffer to output >
 		 loff_t *ppos
 		 < file read offset >
 Return		:ssize_t
 		 < actual read length >
 Description	:vfs file read method
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ssize_t
vfs_read(struct file *filp, char *buf, size_t len, loff_t *ppos)
{
//	printf("*ppos = %ld\n", *ppos);
	if (UNLIKELY(filp->f_vnode->v_size < *ppos)) {
		return(0);
	}
	
	if (UNLIKELY(filp->f_vnode->v_size < (*ppos + len))) {
		len = filp->f_vnode->v_size - *ppos;
	}
	
	if (filp->f_fops && filp->f_fops->read) {
		ssize_t read_len;
		
		read_len = filp->f_fops->read(filp, buf, len, ppos);
		
		//vd_printf("read_len:%d\n", read_len);
		
		return(read_len);
	}
	
	return(-EINVAL);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_write
 Input		:struct file *filp
 		 < open file object >
 		 const char *buf
 		 < user buffer to write >
 		 size_t len
 		 < write length >
 		 loff_t *ppos
 		 < file write offset >
 Output		:loff_t *ppos
 		 < file write offset >
 Return		:ssize_t
 		 < actual write length >
 Description	:vfs file write method
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ssize_t
vfs_write(struct file *filp, const char *buf, size_t len, loff_t *ppos)
{
	if (filp->f_fops && filp->f_fops->write) {
		return(filp->f_fops->write(filp, buf, len, ppos));
	}
	
	return(-EINVAL);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_open
 Input		:struct vnode *vnode
 		 < vnode of file to open >
 		 struct file *filp
 		 < open file object >
 Output		:void
 Return		:int
 		 < result >
 Description	:vfs file open method
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vfs_open(struct vnode *vnode, struct file *filp)
{
	int err = -ENXIO;
	
	if (UNLIKELY(S_ISCHR(vnode->v_mode))) {
		err = open_char_device(vnode, filp);
	} else if (UNLIKELY(S_ISBLK(vnode->v_mode))) {
	} else if (filp->f_fops && filp->f_fops->open) {
		err = filp->f_fops->open(vnode, filp);
	}
	
	return(err);
}


/*
----------------------------------------------------------------------------------
	generic file operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:generic_open
 Input		:struct vnode *vnode
 		 < vnode to open >
 		 struct file *filp
 		 < open file object >
 Output		:void
 Return		:int
 		 < result >
 Description	:generic file open
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int generic_open(struct vnode *vnode, struct file *filp)
{
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
 Funtion	:file_cache_alloc
 Input		:void
 Output		:void
 Return		:struct file*
 		 < file object >
 Description	:allocate a fie object
==================================================================================
*/
LOCAL struct file* file_cache_alloc(void)
{
	struct file *file = (struct file*)kmem_cache_alloc(file_cache, 0);
	
	if (UNLIKELY(!file)) {
		return(NULL);
	}
	
	memset((void*)file, 0x00, sizeof(struct file));
	
	atomic_long_init(&file->f_count, 1);
	
	return(file);
}

/*
==================================================================================
 Funtion	:file_cache_free
 Input		:struct file *file
 		 < file object to free >
 Output		:void
 Return		:void
 Description	:free a file object
==================================================================================
*/
LOCAL void file_cache_free(struct file *file)
{
	kmem_cache_free(file_cache, file);
}

/*
==================================================================================
 Funtion	:fdtable_cache_alloc
 Input		:void
 Output		:void
 Return		:struct fdtable*
 		 < fdtable object >
 Description	:allocate a fdtable object
==================================================================================
*/
LOCAL struct fdtable* fdtable_cache_alloc(void)
{
	struct fdtable *fdtable;
	
	fdtable = (struct fdtable*)kmem_cache_alloc(fdtable_cache, 0);
	
	memset((void*)fdtable, 0x00, sizeof(struct fdtable));
	
	return(fdtable);
}

/*
==================================================================================
 Funtion	:fdtable_cache_free
 Input		:struct fdtable *fdtable
 		 < fdtable object to free >
 Output		:void
 Return		:void
 Description	:free a fdtable object
==================================================================================
*/
LOCAL void fdtable_cache_free(struct fdtable *fdtable)
{
	kmem_cache_free(fdtable_cache, fdtable);
}

/*
==================================================================================
 Funtion	:alloc_fdtable
 Input		:size_t new_size
 		 < size of new fd table >
 Output		:void
 Return		:struct fdtable*
 		 < allocated fd table >
 Description	:allocate a fd table
==================================================================================
*/
LOCAL struct fdtable* alloc_fdtable(size_t new_size)
{
	struct fdtable *fdtable;
	
	fdtable = fdtable_cache_alloc();
	
	if (UNLIKELY(!fdtable)) {
		
		return(NULL);
	}
	
	fdtable->fd = (struct file**)kcalloc(sizeof(struct file*), new_size, 0);
	
	if (UNLIKELY(!fdtable->fd)) {
		goto failed_alloc_fd;
	}
	
	fdtable->open_fds = (unsigned long*)kcalloc(sizeof(unsigned long),
					new_size / (sizeof(unsigned long) * 8), 0);
	
	if (UNLIKELY(!fdtable->open_fds)) {
		goto failed_alloc_open_fds;
	}
	
	fdtable->close_on_exec = (unsigned long*)kcalloc(sizeof(unsigned long),
					new_size / (sizeof(unsigned long) * 8), 0);
	
	if (UNLIKELY(!fdtable->close_on_exec)) {
		goto failed_alloc_close_on_exec;
	}
	
	fdtable->max_fds = new_size;
	
	return(fdtable);
	
failed_alloc_close_on_exec:
	kfree(fdtable->open_fds);
failed_alloc_open_fds:
	kfree(fdtable->fd);
failed_alloc_fd:
	fdtable_cache_free(fdtable);
	
	return(NULL);
}

/*
==================================================================================
 Funtion	:get_fdtable
 Input		:struct process *proc
 		 < process to get its fd table >
 Output		:void
 Return		:struct fdtable*
 		 < fd table >
 Description	:get fd table
==================================================================================
*/
LOCAL ALWAYS_INLINE struct fdtable* get_fdtable(struct process *proc)
{
	if (UNLIKELY(!proc->fs.files)) {
		return(NULL);
	}
	
	atomic_inc(&proc->fs.fd_count);
	
	return(proc->fs.files);
}

/*
==================================================================================
 Funtion	:put_fdtable
 Input		:struct process *proc
 		 < process to put its fd table >
 Output		:void
 Return		:void
 Description	:put fd table
==================================================================================
*/
LOCAL ALWAYS_INLINE void put_fdtable(struct process *proc)
{
	if (atomic_read(&proc->fs.fd_count)) {
		atomic_dec(&proc->fs.fd_count);
	}
}

/*
==================================================================================
 Funtion	:install_fdtable
 Input		:struct process *proc
 		 < process to be installed >
 		 struct fdtable *fdtable
 		 < a fd table to install >
 Output		:void
 Return		:void
 Description	:install a fd table to a process
==================================================================================
*/
LOCAL ALWAYS_INLINE
void install_fdtable(struct process *proc, struct fdtable *fdtable)
{
	if (UNLIKELY(!atomic_read(&proc->fs.fd_count))) {
		atomic_inc(&proc->fs.fd_count);
	}
	
	proc->fs.files = fdtable;
}

/*
==================================================================================
 Funtion	:alloc_fd
 Input		:struct process *proc
 		 < process to be allocated a fd  >
 Output		:void
 Return		:int
 		 < allocated fd >
 Description	:allocate a fd from fd table of a process
==================================================================================
*/
LOCAL int alloc_fd(struct process *proc)
{
	struct fdtable *fdtable;
	int next_fd;
	int found_fd;
	int err;
	int old;
	
	/* -------------------------------------------------------------------- */
	/* :begin critical section						*/
	/* -------------------------------------------------------------------- */
	BEGIN_CRITICAL_SECTION {
	fdtable = get_fdtable(proc);
	
	next_fd = fdtable->next_fd;
	
	if (UNLIKELY(fdtable->max_fds <= next_fd)) {
		err = extend_fdtable(proc);
		
		if (UNLIKELY(err)) {
			put_fdtable(proc);
			OUT_CRITICAL_SECTION;
			return(err);
		}
	}
	
retry_find_fd:
	found_fd =(int)tstdlib_bitsearch0(fdtable->open_fds, next_fd,
						fdtable->max_fds);
	
	if (UNLIKELY(found_fd < 0)) {
		if (UNLIKELY(!next_fd)) {
			next_fd = fdtable->max_fds;
			err = extend_fdtable(proc);
			
			if (UNLIKELY(err)) {
				put_fdtable(proc);
				OUT_CRITICAL_SECTION;
				return(err);
			}
		} else {
			next_fd = 0;
		}
		goto retry_find_fd;
	}
	
	old = tstdlib_bitset(fdtable->open_fds, found_fd);
	
	if (UNLIKELY(old)) {
		goto retry_find_fd;
	}
	
	put_fdtable(proc);
	/* -------------------------------------------------------------------- */
	/* :end critical section						*/
	/* -------------------------------------------------------------------- */
	} END_CRITICAL_SECTION;
	return(found_fd);
}


/*
==================================================================================
 Funtion	:free_fd
 Input		:struct process *proc
 		 < process to be freed a fd  >
 		 int fd
 		 < file descriptor to free >
 Output		:void
 Return		:void
 Description	:free a fd
==================================================================================
*/
LOCAL void free_fd(struct process *proc, int fd)
{
	struct fdtable *fdtable;
	
	/* -------------------------------------------------------------------- */
	/* :begin critical section						*/
	/* -------------------------------------------------------------------- */
	BEGIN_CRITICAL_SECTION {
	fdtable = get_fdtable(proc);
	
	tstdlib_bitclr(fdtable->open_fds, fd);
	
	if (fd <= fdtable->next_fd) {
		fdtable->next_fd = fd;
	}
	
	put_fdtable(proc);
	/* -------------------------------------------------------------------- */
	/* :end critical section						*/
	/* -------------------------------------------------------------------- */
	} END_CRITICAL_SECTION;
}

/*
==================================================================================
 Funtion	:install_file
 Input		:struct process *proc
 		 < process to be installed a file to its fd table >
 		 int fd
 		 < file descriptor >
 		 struct file *file
 		 < file to install >
 Output		:void
 Return		:int
 		 < installed file descriptor >
 Description	:install a open file
==================================================================================
*/
LOCAL ALWAYS_INLINE int
install_file(struct process *proc, struct file *file)
{
	struct fdtable *fdtable;
	int fd;
	/* -------------------------------------------------------------------- */
	/* :begin critical section						*/
	/* -------------------------------------------------------------------- */
	BEGIN_CRITICAL_SECTION {
	
	fd = alloc_fd(proc);
	
	if (UNLIKELY(fd < 0)) {
		OUT_CRITICAL_SECTION;
		return(-EMFILE);
	}
	
	fdtable = get_fdtable(proc);
	
	fdtable->fd[fd] = file;
	
	put_fdtable(proc);
	/* -------------------------------------------------------------------- */
	/* :end critical section						*/
	/* -------------------------------------------------------------------- */
	} END_CRITICAL_SECTION;
	
	return(fd);
}


/*
==================================================================================
 Funtion	:xopenat
 Input		:struct path *dir_path
 		 < directory path >
 		 const char *pathname
 		 < path name to open >
 		 int falgs
 		 < file open flags >
 		 mode_t mode
 		 < file open mode >
 Output		:void
 Return		:int
 		 < open file descriptor >
 Description	:actual file open process relative to a directory file descriptor
==================================================================================
*/
LOCAL int
xopenat(struct path *dir_path, const char *pathname, int flags, mode_t mode)
{
	struct file_name *fname;
	struct file *filp;
	struct process *proc;
	struct dentry *dentry;
	int err;
	int fd;
	
	//printf("open [%s]\n", pathname);
	
	err = vfs_lookup_at(dir_path, pathname, &fname, LOOKUP_ENTRY);
	//err = vfs_lookup(pathname, &fname, LOOKUP_TEST);
	
	if (err) {
		//printf("failed vfs lookup at %s\n", __func__);
		goto failed_vfs_lookup;
	}
	
	dentry = fname->dentry;
	
	put_file_name(fname);
	
	if (!dentry->d_vnode) {
		//printf("open:negative dentry?\n");
		return(-ENOENT);
	}
	
	proc = get_current();
	
	filp = file_cache_alloc();
	
	if (UNLIKELY(!filp)) {
		err = -ENOMEM;
		panic("unexpected error at %s\n", __func__);
		goto failed_vfs_lookup;
	}
	
	filp->f_vnode = dentry->d_vnode;
	filp->f_fops = dentry->d_vnode->v_fops;
	filp->f_path.mnt = vfs_get_cwd(proc)->mnt;
	filp->f_path.dentry = dentry;
	
	//vd_printf("vfs_open:%s\n", pathname);
	
	err = vfs_open(dentry->d_vnode, filp);
	
	if (err) {
		printf("open:failed vfs_open\n");
		err = -ENOENT;
		goto failed_vfs_open;
	}
	
	fd = install_file(proc, filp);
	
	//printf("install_file:%s fd:%d\n", pathname, fd);
	
	if (fd < 0) {
		printf("open:failed install_file\n");
		err = fd;
		goto failed_install_file;
	}
	
	//printf("open file:%s at [%d]\n", dentry->d_iname, fd);

	
	//printf("opened:fd=%d\n", fd);
	
	return(fd);

failed_vfs_lookup:
	put_file_name(fname);
	return(err);
failed_vfs_open:
failed_install_file:
	file_cache_free(filp);
	return(err);
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
