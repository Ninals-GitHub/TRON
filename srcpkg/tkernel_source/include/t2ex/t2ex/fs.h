/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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

/*
 *	@(#)fs.h
 *
 *	T2EX: file system functions
 */

#ifndef __T2EX_FS_H__
#define __T2EX_FS_H__

#include <basic.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/fimp.h>
#include <sys/unistd.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Startup & Finish
 */
IMPORT	ER	fs_main(INT ac, UB *arg[]);

/*
 * Interface libraries (use internal SVC _fs_xxxx())
 */
IMPORT	int	fs_open(const char *path, int oflag, ... /* mode_t mode */ );
IMPORT	int	fs_creat(const char *pathname, mode_t mode);
IMPORT	off_t	fs_lseek(int fd, off_t offset, int whence);
IMPORT	off64_t	fs_lseek64(int fd, off64_t offset64, int whence);
IMPORT	int	fs_ioctl(int fd, int request, ... /* arg */ );
IMPORT	int	fs_fcntl(int fd, int cmd, ... /* arg */ );

/*
 * Definition for interface library automatic generation (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<t2ex/fs.h>

[PREFIX]
FS
***/

/* [BEGIN SYSCALLS] */
IMPORT	ER	fs_regist(const char *fimpnm, const fs_fimp_t *fimp, void *info);
IMPORT	ER	fs_unregist(const char *fimpnm);
IMPORT	ER	fs_attach(const char *devnm, const char *connm, const char *fimpnm, int flags, void *info);
IMPORT	ER	fs_detach(const char *connm);
IMPORT	int	fs_break(ID tskid);

IMPORT	ER	fs_chdir(const char *path);
IMPORT	ER	fs_fchdir(int fd);
IMPORT	ER	fs_getcwd(char *buf, size_t size);

IMPORT	ER	fs_chmod(const char *path, mode_t mode);
IMPORT	ER	fs_fchmod(int fd, mode_t mode);

IMPORT	ER	fs_fsync(int fd);
IMPORT	ER	fs_fdatasync(int fd);
IMPORT	ER	fs_sync(void);

IMPORT	ER	fs_mkdir(const char *path, mode_t mode);
IMPORT	ER	fs_rmdir(const char *path);
IMPORT	int	fs_getdents(int fd, struct dirent* buf, size_t bufsz);

IMPORT	int	fs_read(int fd, void *buf, size_t count);
IMPORT	int	fs_write(int fd, const void *buf, size_t count);
IMPORT	ER	fs_close(int fd);

IMPORT	ER	fs_rename(const char *oldpath, const char *newpath);
IMPORT	ER	fs_unlink(const char *path);

IMPORT	ER	fs_stat(const char *path, struct stat *buf);
IMPORT	ER	fs_stat_us(const char *path, struct stat_us *ubuf);
IMPORT	ER	fs_stat_ms(const char *path, struct stat_ms *mbuf);
IMPORT	ER	fs_stat64(const char *path, struct stat64 *buf64);
IMPORT	ER	fs_stat64_us(const char *path, struct stat64_us *ubuf64);
IMPORT	ER	fs_stat64_ms(const char *path, struct stat64_ms *mbuf64);

IMPORT	ER	fs_fstat(int fd, struct stat *buf);
IMPORT	ER	fs_fstat_us(int fd, struct stat_us *ubuf);
IMPORT	ER	fs_fstat_ms(int fd, struct stat_ms *mbuf);
IMPORT	ER	fs_fstat64(int fd, struct stat64 *buf64);
IMPORT	ER	fs_fstat64_us(int fd, struct stat64_us *ubuf64);
IMPORT	ER	fs_fstat64_ms(int fd, struct stat64_ms *mbuf64);

IMPORT	ER	fs_truncate(const char *path, off_t length);
IMPORT	ER	fs_truncate64(const char *path, off64_t length64);
IMPORT	ER	fs_ftruncate(int fd, off_t length);
IMPORT	ER	fs_ftruncate64(int fd, off64_t length64);

IMPORT	ER	fs_utimes(const char *path, const struct timeval tim[2]);
IMPORT	ER	fs_utimes_us(const char *path, const SYSTIM_U tim_u[2]);
IMPORT	ER	fs_utimes_ms(const char *path, const SYSTIM tim_m[2]);

IMPORT	ER	fs_statvfs(const char *path, struct statvfs *buf);
IMPORT	ER	fs_fstatvfs(int fd, struct statvfs *buf);

/* internal use */
IMPORT	int	_fs_open(const char *path, int oflag, mode_t mode);
IMPORT	ER	_fs_lseek(int fd, off_t offset, int whence, off_t *roff);
IMPORT	ER	_fs_lseek64(int fd, off64_t offset64, int whence, off64_t *roff);
IMPORT	int	_fs_ioctl(int fd, int request, void *arg);
IMPORT	int	_fs_fcntl(int fd, int cmd, void *arg);

/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif	/* __T2EX_FS_H__ */

