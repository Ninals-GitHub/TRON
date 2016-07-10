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

#ifndef	__BK_FS_FILE_H__
#define	__BK_FS_FILE_H__

#include <bk/kernel.h>
#include <bk/fs/vfs.h>
#include <bk/uio.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct file;
struct iocb;
struct dir_context;
struct poll_table;
struct file_lock;
struct page;
struct pipe_vnode_info;
struct seq_file;
struct iov_iter;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
typedef void *fl_owner_t;
/*
----------------------------------------------------------------------------------
	opne file operations
----------------------------------------------------------------------------------
*/
struct file_operations {
	loff_t (*llseek)(struct file *filp, loff_t offset, int whence);
	ssize_t (*read)(struct file *filp, char *buf, size_t len, loff_t *ppos);
	ssize_t (*write)(struct file *filp, const char *buf, size_t len, loff_t *ppos);
	ssize_t (*read_iter)(struct iocb *iocb, struct iov_iter *iter);
	ssize_t (*write_iter)(struct iocb *iocb, struct iov_iter *from);
	int (*iterate)(struct file *filp, struct dir_context *ctx);
	unsigned int (*poll)(struct file *filp, struct poll_table *ptable);
	long (*unlocked_ioctl)(struct file *filp, unsigned int cmd, unsigned long arg);
	long (*compat_ioctl)(struct file *filp, unsigned int cmd, unsigned long arg);
	long (*mmap)(struct file *filp, struct vm *vm);
	int (*open)(struct vnode *vnode, struct file *filp);
	int (*flush)(struct file *filp, fl_owner_t id);
	int (*release)(struct vnode *vnode, struct file *filp);
	int (*fsync)(struct file *filp, loff_t start, loff_t end, int datasync);
	int (*aio_fsync)(struct iocb *iocb, int datasync);
	int (*lock)(struct file *filp, int cmd, struct file_lock *fl);
	ssize_t (*sendpage)(struct file *filp, struct page *page,
				int off, size_t len, loff_t *pos, int more);
	unsigned long (*get_unmapped_area)(struct file *filp, unsigned long addr,
						unsigned long len,
						unsigned long pgoff,
						unsigned long flags);
	int (*check_flags)(int flags);
	int (*flock)(struct file *filp, int cmd, struct file_lock *fl);
	ssize_t (*splice_write)(struct pipe_vnode_info *pipe, struct file *out,
				loff_t *ppos, size_t len, unsigned long flags);
	ssize_t (*splice_read)(struct file *filp, loff_t *ppos,
				struct pipe_vnode_info *pipe, size_t len,
				unsigned long flags);
	int (*setlease)(struct file *filp, long arg,
				struct file_lock **lease, void **priv);
	long (*fallocate)(struct file *filp, int mode, loff_t offset, loff_t len);
	int (*show_fdinfo)(struct seq_file *m, struct file *f);
};

/*
----------------------------------------------------------------------------------
	file owner
----------------------------------------------------------------------------------
*/
struct file_owner {
	pid_t		pid;
	uid_t		uid;
	uid_t		euid;
	int		signum;
};

/*
----------------------------------------------------------------------------------
	open file object
----------------------------------------------------------------------------------
*/
struct file {
	struct path			f_path;
	struct vnode			*f_vnode;
	const struct file_operations	*f_fops;
	atomic_long_t			f_count;
	unsigned int			f_flags;
	loff_t				f_pos;
	struct file_owner		f_owner;
	void				*f_private;
};

/*
----------------------------------------------------------------------------------
	open file descriptor table
----------------------------------------------------------------------------------
*/
#define	DEFAULT_NR_FDS			32
#define	INC_NR_FDS			32

struct fdtable {
	int				next_fd;
	unsigned int			max_fds;
	struct file			**fd;
	unsigned long			*close_on_exec;
	unsigned long			*open_fds;
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
 Funtion	:init_files
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize file object management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_files(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_files_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a file cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void destroy_files_cache(void);

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
IMPORT int extend_fdtable(struct process *proc);

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
IMPORT void free_fdtable(struct fdtable *fdtable);

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
IMPORT void copy_fdtable(struct fdtable *to, struct fdtable *from);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_file
 Input		:struct process *proc
 		 < process to be allocated a file >
 Output		:void
 Return		:struct file*
 		 < an open file >
 Description	:allocate a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct file* alloc_file(struct process *proc);

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
IMPORT void free_file(struct process *proc, int fd);

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
IMPORT struct file* get_open_file(int fd);

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
IMPORT int get_dirfd_path(int dirfd, struct path **dir_path);

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
IMPORT int is_open_file(int fd);

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
IMPORT int is_open_dir(int fd);

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
IMPORT int has_fdtable(struct process *proc);

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
IMPORT int open_exe_file(const char *filename);

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
IMPORT int open_vdso_file(void);

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
IMPORT ssize_t kwrite(int fd, const void *buf, size_t count);

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
IMPORT ssize_t kpread(int fd, void *buf, size_t count, loff_t offset);

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
IMPORT int kopen(const char *pathname, int flags, mode_t mode);

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
 Description	:read a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL ssize_t read(int fd, void *buf, size_t count);

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
SYSCALL ssize_t pread(int fd, void *buf, size_t count, loff_t offset);

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
SYSCALL ssize_t write(int fd, const void *buf, size_t count);

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
SYSCALL int open(const char *pathname, int flags, mode_t mode);

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
SYSCALL ssize_t writev(int fd, const struct iovec *iov, int iovcnt);

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
SYSCALL int close(int fd);

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
SYSCALL loff_t lseek64(int fd, loff_t offset64, int whence);

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
IMPORT ssize_t vfs_read(struct file *filp, char *buf, size_t len, loff_t *ppos);

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
IMPORT ssize_t
vfs_write(struct file *filp, const char *buf, size_t len, loff_t *ppos);

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
IMPORT int vfs_open(struct vnode *vnode, struct file *filp);

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
IMPORT int generic_open(struct vnode *vnode, struct file *filp);

#endif	// __BK_FS_FILE_H__
