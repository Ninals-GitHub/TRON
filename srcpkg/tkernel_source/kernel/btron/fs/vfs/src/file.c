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
#include <bk/uapi/sys/stat.h>

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
LOCAL ALWAYS_INLINE struct file* get_open_file(int fd);

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
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(!count)) {
		return(0);
	}
	
	if (UNLIKELY(get_soft_limit(RLIMIT_NOFILE) < fd)) {
		return(-EBADF);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	return(vfs_read(filp, buf, count, &filp->f_pos));
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
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(!count)) {
		return(0);
	}
	
	if (UNLIKELY(get_soft_limit(RLIMIT_NOFILE) < fd)) {
		return(-EBADF);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	return(vfs_write(filp, buf, count, &filp->f_pos));
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
	struct file_name *fname;
	struct file *filp;
	struct process *proc;
	struct dentry *dentry;
	int err;
	int fd;
	
	err = vfs_lookup(pathname, &fname, 0);
	
	if (err) {
		goto failed_vfs_lookup;
	}
	
	dentry = fname->dentry;
	
	put_file_name(fname);
	
	if (!dentry->d_vnode) {
		vd_printf("open:negative dentry?\n");
		return(-ENOENT);
	}
	
	proc = get_current();
	
	filp = file_cache_alloc();
	
	if (UNLIKELY(!filp)) {
		err = -ENOENT;
		goto failed_vfs_lookup;
	}
	
	err = vfs_open(dentry->d_vnode, filp);
	
	if (err) {
		vd_printf("open:failed vfs_open\n");
		err = -ENOENT;
		goto failed_vfs_open;
	}
	
	fd = install_file(proc, filp);
	
	if (fd) {
		vd_printf("open:failed install_file\n");
		err = -ENOENT;
		goto failed_install_file;
	}
	
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
EXPORT ssize_t vfs_read(struct file *filp, char *buf, size_t len, loff_t *ppos)
{
	if (filp->f_fops && filp->f_fops->read) {
		return(filp->f_fops->read(filp, buf, len, ppos));
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
	} else if (filp->f_fops->open) {
		err = filp->f_fops->open(vnode, filp);
	}
	
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
	
	old = tstdlib_bittest(fdtable->open_fds, found_fd);
	
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
	
	fdtable->next_fd = fd;
	
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
 Funtion	:get_open_file
 Input		:int fd
 		 < open file descriptor >
 Output		:void
 Return		:struct file*
 		 < open file object >
 Description	:get opened file from a current process
==================================================================================
*/
LOCAL ALWAYS_INLINE struct file* get_open_file(int fd)
{
	struct process *proc = get_current();
	struct fdtable *files = proc->fs.files;
	
	return(files->fd[fd]);
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
