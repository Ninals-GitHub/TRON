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
 *	@(#)fimp.h
 *
 *	T2EX: file system implementaion part definitions
 */

#ifndef _SYS_FIMP_H_
#define _SYS_FIMP_H_

#include <basic.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <tk/devmgr.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*
 *  Type Definitions
 */
typedef	unsigned int	fid_t;	/* LFS & PFS file identifier */

/*
 * flags
 */
#define FIMP_READONLY	 1		/* Readonly file system 	*/
#define FIMP_REMOVABLE	 2		/* Removable file system	*/
#define FIMP_MEMORY	 4		/* Memory file system		*/
#define FIMP_NETWORK	 8		/* Network file system		*/
#define FIMP_DEVICE	(16 | FIMP_MEMORY)	/* Device file system	*/
#define FIMP_64BIT	 32		/* 64bit file supported file system */
#define FIMP_USEABORT	 64		/* Abort supported file system	*/
#define FIMP_PRIVILEGE	 128		/* Privilege file system	*/
#define FIMP_MASK	 0xff		/* PFS flags mask		*/
#define FIMP_SHIFT	 8		/* Right shift for PFS index	*/

/*
 *  regist/unregist/attach/detach
*/
/* Max name length for filesystems (POSIX min-value) */
#define L_FIMPNM 	14
#define L_CONNM 	14

/* FIMP information */
typedef struct {
	char	fimpnm[L_FIMPNM + 1];		/* FIMP name		*/
	void	*fimpsd;			/* FIMP specific data	*/
} fimpinf_t;

/* Connection information */
typedef struct {
	fimpinf_t	*fimpinf;		/* fimpinf		*/
	int		dflags;			/* Device flags 	*/
	UB		devnm[L_DEVNM + 1];	/* Device name		*/
	char		connm[L_CONNM + 1];	/* Connection name	*/
	void		*consd;			/* Connection specific data */
} coninf_t;

/* Device flags */
#define DEV_FLAG_READONLY	FIMP_READONLY	/* Readonly device	*/
#define DEV_FLAG_REMOVABLE	FIMP_REMOVABLE	/* Removable device	*/
#define DEV_FLAG_MEMORY 	FIMP_MEMORY	/* Memory device	*/
#define DEV_FLAG_MASK		(DEV_FLAG_READONLY | DEV_FLAG_REMOVABLE |\
					DEV_FLAG_MEMORY)

/*
 *  FIMP command codes
 */
#define FIMP_OPEN	 	0
#define FIMP_CLOSE	 	1
#define FIMP_IOCTL	 	2
#define FIMP_GETDENTS	 	3
#define FIMP_CHDIR	 	4
#define FIMP_FCHDIR		5
#define FIMP_FSYNC		6
#define FIMP_UNLINK		7
#define FIMP_RENAME		8
#define FIMP_CHMOD		9
#define FIMP_FCHMOD		10
#define FIMP_MKDIR		11
#define FIMP_RMDIR		12
#define FIMP_STATVFS		13
#define FIMP_FSTATVFS		14
#define FIMP_SYNC		15
#define FIMP_READ64		16
#define FIMP_WRITE64		17
#define FIMP_TRUNCATE64		18
#define FIMP_FTRUNCATE64	19
#define FIMP_FCNTL64		20
#define FIMP_STAT64_US		21
#define FIMP_FSTAT64_US		22
#define FIMP_UTIMES_US		23
#define	FIMP_COMMAND_MAX	23

/*
 *  DEV command codes
 */
#define DEV_DEVOPEN		 0
#define DEV_DEVCLOSE		 1
#define DEV_DEVREAD		 2
#define DEV_DEVWRITE		 3
#define DEV_DEVCTL		 4
#define DEV_DEVSELECT		 5

struct fimp_open {
	int		r_code;		/* Request code 		*/
	coninf_t 	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
	int		oflags;		/* Open flags			*/
	mode_t		mode;		/* Open mode			*/
	fid_t		*fid;		/* Ptr to return fid		*/
};

struct fimp_close {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	int		oflags;		/* Open flags			*/
};

struct fimp_read64 {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	int		oflags;		/* Open flags			*/
	void		*buf;		/* User buffer			*/
	size_t		*len;		/* Read and return size ptr	*/
	off64_t		*off;		/* Ptr to File offset		*/
	off64_t		*retoff;	/* Ptr to Return offset		*/
};

struct fimp_write64 {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	int		oflags;		/* Open flags			*/
	void		*buf;		/* User buffer			*/
	size_t		*len;		/* Write and return size ptr	*/
	off64_t		*off;		/* Ptr to File offset		*/
	off64_t		*retoff;	/* Ptr to Return offset		*/
};

struct fimp_ioctl {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	int		oflags;		/* Open flags			*/
	int		dcmd;		/* Command to device		*/
	void		*arg;		/* Ptr to in/out data		*/
	ER		*retval;	/* Ptr to return device info	*/
};

struct fimp_fsync {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	int		type;		/* Fsync or fdatasync		*/
	int		oflags;		/* Open flags			*/
};
#define TYPE_FSYNC	0		/* Fsync command		*/
#define TYPE_FDATASYNC	1		/* Fdatasync command		*/

struct fimp_truncate64 {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
	off64_t		len;		/* Target length		*/
};

struct fimp_ftruncate64 {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	off64_t		len;		/* Target length		*/
};

struct fimp_unlink {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
};

struct fimp_rename {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*oldpath;	/* Old file path		*/
	const char	*newpath;	/* New file path		*/
};

struct fimp_chmod {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
	mode_t		mode;		/* File mode			*/
};

struct fimp_fchmod {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	mode_t		mode;		/* File mode			*/
};

struct fimp_mkdir {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
	mode_t		mode;		/* File mode			*/
};

struct fimp_rmdir {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
};

struct fimp_chdir {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
};

struct fimp_fchdir {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	char		*buf;		/* Name buffer			*/
	int		len;		/* Buffer size			*/
};

struct fimp_getdents {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	int		oflags;		/* Open flags			*/
	struct dirent	*buf;		/* Directory entry		*/
	size_t		*len;		/* Read and return size ptr	*/
	off64_t		*off;		/* Ptr to File offset		*/
	off64_t		*retoff;	/* Ptr to Return offset		*/
};

struct fimp_fstatvfs {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	struct statvfs *buf;		/* Ptr to statvfs structure	*/
};

struct fimp_statvfs {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
	struct statvfs *buf;		/* Ptr to statvfs structure	*/
};

struct fimp_sync {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
};

struct fimp_utimes_us {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
	SYSTIM_U 	*times_u;	/* [0]atime, [1]mtime		*/
};

struct fimp_fcntl64 {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	int		*oflags;	/* Ptr to Open flags		*/
	int		fcmd;		/* Command to PFS		*/
	off64_t		*off;		/* Ptr to File offset		*/
	void 		*arg;		/* Ptr to User buffer		*/
	ER		*retval;	/* Ptr to Return code		*/
};

struct fimp_fstat64_us {
	int		r_code;		/* Request code 		*/
	coninf_t 	*coninf;	/* Connection information	*/
	fid_t		fid;		/* Open file ID			*/
	struct stat64_us *buf; 	/* Ptr to stat structure	*/
};

struct fimp_stat64_us {
	int		r_code;		/* Request code 		*/
	coninf_t	*coninf;	/* Connection information	*/
	const char	*path;		/* File path			*/
	struct stat64_us *buf; 	/* Ptr to stat structure	*/
};

/*
 *  Device flags
 */
#define DEV_BLOCK	1		/* Block device 		*/
#define DEV_CHARACTER	2		/* Character device		*/
#define DEV_TTY 	4		/* TTY device			*/
#define DEV_CONSOLE	8		/* Console device		*/
#define DEV_MEMORY	16		/* Memory device		*/
#define DEV_USEABORT	64		/* Abort supported device	*/
#define DEV_PRIVILEGE	128		/* Privilege device		*/
#define DEV_MASK	0xff		/* Device flags mask		*/
#define DEV_SHIFT	8		/* Right shift for device unit	*/

/*
 *  Request union
 */
union fimp {
	struct {				/* Common structure	*/
		int		r_code;		/* Reuest Service code	*/
		coninf_t	*coninf;	/* Connection information */
	} com;
	struct fimp_open	r_open;		/* FIMP_OPEN		*/
	struct fimp_close	r_close;	/* FIMP_CLOSE		*/
	struct fimp_read64	r_read64;	/* FIMP_READ64		*/
	struct fimp_write64	r_write64;	/* FIMP_WRITE64		*/
	struct fimp_ioctl	r_ioctl;	/* FIMP_IOCTL		*/
	struct fimp_fsync	r_fsync;	/* FIMP_FSYNC		*/
	struct fimp_truncate64 r_truncate64;	/* FIMP_TRUNCATE64	*/
	struct fimp_ftruncate64 r_ftruncate64; /* FIMP_FTRUNCATE64	*/
	struct fimp_unlink	r_unlink;	/* FIMP_UNLINK		*/
	struct fimp_rename	r_rename;	/* FIMP_RENAME		*/
	struct fimp_chmod	r_chmod;	/* FIMP_CHMOD		*/
	struct fimp_fchmod	r_fchmod;	/* FIMP_FCHMOD		*/
	struct fimp_mkdir	r_mkdir;	/* FIMP_MKDIR		*/
	struct fimp_rmdir	r_rmdir;	/* FIMP_RMDIR		*/
	struct fimp_chdir	r_chdir;	/* FIMP_CHDIR		*/
	struct fimp_fchdir	r_fchdir;	/* FIMP_FCHDIR		*/
	struct fimp_getdents	r_getdents;	/* FIMP_GETDENTS	*/
	struct fimp_fstatvfs	r_fstatvfs;	/* FIMP_FSTATVFS	*/
	struct fimp_statvfs	r_statvfs;	/* FIMP_STATVFS		*/
	struct fimp_sync	r_sync;		/* FIMP_SYNC		*/
	struct fimp_utimes_us	r_utimes_us;	/* FIMP_UTIMES_US	*/
	struct fimp_fcntl64	r_fcntl64;	/* FIMP_FCNTL64		*/
	struct fimp_stat64_us	r_stat64_us;	/* FIMP_STAT64_US	*/
	struct fimp_fstat64_us r_fstat64_us;	/* FIMP_FSTAT64_US	*/
};
typedef union fimp fimp_t;

/* FIMP function table */
typedef struct {
	/* Service function		*/
	ER	(*reqfn) (fimp_t * req);
	/* Registration function	*/
	ER	(*registfn) (fimpinf_t * fimpinf, void *info);
	/* Unregistration function	*/
	ER	(*unregistfn) (fimpinf_t * fimpinf);
	/* Attach function		*/
	ER	(*attachfn) (coninf_t * coninf, void *info);
	/* Detach function		*/
	ER	(*detachfn) (coninf_t * coninf);
	/* Startup function		*/
	ER	(*startupfn) (coninf_t * coninf, ID resid);
	/* Cleanup function		*/
	ER	(*cleanupfn) (coninf_t * coninf, ID resid);
	/* Break function		*/
	ER	(*breakfn) (coninf_t * coninf, ID tskid, BOOL set);
	int	flags;		/* flags 		*/
	int	priority;	/* Priority (1-8)	*/
} fs_fimp_t;

/* flags */
#define FIMP_FLAG_READONLY	FIMP_READONLY
#define FIMP_FLAG_MEMORY 	FIMP_MEMORY
#define FIMP_FLAG_NETWORK	FIMP_NETWORK
#define FIMP_FLAG_DEVICE 	FIMP_DEVICE
#define FIMP_FLAG_64BIT		FIMP_64BIT
#define FIMP_FLAG_USEABORT	FIMP_USEABORT
#define FIMP_FLAG_PRIVILEGE	FIMP_PRIVILEGE

#define FIMP_FLAG_MASK		(FIMP_FLAG_READONLY | FIMP_FLAG_MEMORY |\
				 FIMP_FLAG_NETWORK | FIMP_FLAG_DEVICE |\
				 FIMP_FLAG_64BIT | FIMP_FLAG_USEABORT |\
				 FIMP_FLAG_PRIVILEGE)

/* Special fimpnm */
#define FIMP_FAT 		"FIMP_FAT"
#define FIMP_AUTODETECT		"FIMP_AUTODETECT"

#ifdef	__cplusplus
}
#endif
#endif	/* _SYS_FIMP_H_ */

