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
 *	@(#)fsdefs.h
 *
 */

#ifndef __FSDEFS_H__
#define __FSDEFS_H__

#include <basic.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <tk/util.h>
#include <sys/queue.h>
#include <t2ex/fs.h>
#include <t2ex/datetime.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* Offset check specification */
#define SIGNED_OFFSET		(1)
#define LSEEK_WITHIN_FILE_SIZE	(1)

/* Config: Maximum number of files that can be opened */
#define BASE_FS_OPEN_MAX	(3)	/* stdin, stdout, stderr */
#define MIN_FS_OPEN_MAX		(BASE_FS_OPEN_MAX + 2)
#define DEF_FS_OPEN_MAX		(BASE_FS_OPEN_MAX + 8)

/* Config: Maximum number of the FIMP that can be registered */
#define BASE_FS_FIMP_MAX	(2)	/* root, console */
#define MIN_FS_FIMP_MAX		(BASE_FS_FIMP_MAX + 1)
#define DEF_FS_FIMP_MAX		(BASE_FS_FIMP_MAX + 1)

/* Config: Maximum number of the Connection that can be attached */
#define BASE_FS_CON_MAX		(2)	/* root, console */
#define MIN_FS_CON_MAX		(BASE_FS_CON_MAX + 1)
#define DEF_FS_CON_MAX		(BASE_FS_CON_MAX + 2)

/* Max name length for FS filesystems, devices and FIFOs (POSIX min-value) */
#define FS_NAME_MAX		(14)

/*
 *  Forward references
 */
typedef struct fs_cond	fs_cond_t;	/* Connection descriptor	*/
typedef struct fs_env	fs_env_t;	/* Current environment	*/

/*
 *  FS path structure
 */
typedef struct fs_path	fs_path_t;
struct fs_path {
	fs_cond_t	*p_con;		/* Connecter descriptor		*/
	B		*p_name;	/* Relative path name		*/
};
IMPORT	INT	fs_path_parse(fs_env_t *env, const B *name,
					fs_path_t * path, BOOL write);
IMPORT	void	fs_path_release(fs_env_t *env, fs_path_t * const path);

/*
 *  Exclusive lock number
 */
#define	LOCKNUM_CTX	(0)		/* lock number for ctx		*/
#define	LOCKNUM_FIL	(1)		/* lock number for file		*/
#define	LOCKNUM_CDIR	(2)		/* lock number for curdir	*/
#define	LOCKNUM_FIMP	(3)		/* lock number for FIMP list	*/
#define	LOCKNUM_ROOT	(4)		/* lock number for root list	*/
#define	LOCKNUM_DESC	(5)		/* lock number for desc		*/

IMPORT	void	fs_lock_lock(INT locknum);
IMPORT	void	fs_lock_unlock(INT locknum);

/*
 *  Registration structure
 */
typedef struct fs_regis	fs_regis_t;
struct fs_regis {
	QUEUE		r_list;		/* List of registed entry	*/
	INT		r_count;	/* Number of enqueued lists	*/
};

/*
 *  Generic descriptor
 */
typedef struct fs_desc	fs_desc_t;
struct fs_desc {
	QUEUE		d_list;		/* Descriptor list structure	*/
	UINT		d_ident;	/* Descriptor identifier	*/
	UH		d_flags;	/* Implimentation dependent flags */
	UH		d_refc;		/* Reference counter		*/
};

/* Descriptor types */
#define DESC_FILESYS	0x01		/* FIMP 			*/
#define DESC_CON	0x02		/* Connection			*/
#define DESC_FILE	0x03		/* File 			*/
		/* Note: ( DESC_FILE & FD_CLOEXEC ) should be not zero	*/

/*
 *  Descriptor manager
 */
typedef struct	fs_descmgr	fs_descmgr_t;
struct	fs_descmgr {
	H		m_descmax;	/* Max number of descriptors	*/
	H		m_descsize;	/* Size per descriptor		*/
	H		m_type;		/* Descriptor type		*/
	H		m_usecnt;	/* Number of used descriptors	*/
	fs_desc_t	*m_freelist;	/* Free descriptor list 	*/
	fs_desc_t	*m_descpool;	/* Descriptor pool		*/
};

IMPORT	INT	fs_descmgr_init(fs_descmgr_t * const mgr, UINT max,
						UINT size, UINT type);
IMPORT	INT	fs_descmgr_fini(fs_descmgr_t * const mgr);
IMPORT	fs_desc_t *fs_descmgr_alloc(fs_descmgr_t * const mgr);
IMPORT	void	fs_descmgr_free(fs_descmgr_t * const mgr, fs_desc_t *desc);
IMPORT	fs_desc_t *fs_descmgr_get(fs_descmgr_t * const mgr, UINT id);
IMPORT	void	fs_descmgr_release(fs_descmgr_t * const mgr, fs_desc_t *desc);
IMPORT	UINT	fs_descmgr_validate(fs_descmgr_t * const mgr, fs_desc_t *desc);
IMPORT	void	fs_descmgr_invalidate(fs_descmgr_t * const mgr,
						fs_desc_t *desc);
IMPORT	INT	fs_descmgr_ident2index(UINT ident);

/*
 *  File descriptor
 */
typedef union fs_file	fs_file_t;
union fs_file {
	fs_desc_t	f_desc;		/* Basic descriptor		*/
	struct {
		/* f_desc.d_list isn't used, so is used as f_fid and f_cid */
		UINT	f_fid;		/* FIMP file ID 		*/
		UINT	f_cid;		/* Connection id		*/
		UINT	d_ident;	/* Descriptor identifier	*/
		UH	d_flags;	/* Implimentation dependent flags */
		UH	d_refc;		/* Reference counter		*/
		/* additions to f_desc */
		off64_t f_off;		/* File offset			*/
	} f;
};
#define	FILE_OFFSET(file)	((file)->f.f_off)

IMPORT	INT	fs_file_init(INT max);
IMPORT	INT	fs_file_fini(void);
IMPORT	fs_file_t *fs_file_alloc(void);
IMPORT	void	fs_file_free(fs_file_t *file);
IMPORT	INT	fs_file_get(fs_env_t *env, INT fn, fs_file_t **fp,
							fs_cond_t **conp);
IMPORT	INT	fs_file_release(fs_env_t *env, fs_file_t *file, fs_cond_t *con);
IMPORT	void	fs_file_setup(fs_file_t * const file, INT fid,
							INT cid, INT flags);

/*
 *  FIMP descriptor structure
 */
typedef struct fs_fimpd	fs_fimpd_t;
struct fs_fimpd {
	fs_desc_t	p_desc;		/* Basic descriptor		*/
	fs_fimp_t	p_fimp;		/* FIMP 			*/
	fimpinf_t	p_fimpinf;	/* FIMP information		*/
};
#define FIMP_BUSY	(1)		/* p_desc.d_flags		*/

IMPORT	INT	fs_fimp_init(INT max);
IMPORT	INT	fs_fimp_fini(void);
IMPORT	INT	fs_fimp_find(const B *fimpnm, fs_fimpd_t **fimpd);
IMPORT	void	fs_fimp_release(fs_fimpd_t *fimpd);
IMPORT	INT	fs_fimp_regist(const B *fimpnm, fs_fimpd_t **fimpd);
IMPORT	void	fs_fimp_end_regist(fs_fimpd_t *fimpd, INT err);
IMPORT	INT	fs_fimp_unregist(const B *fimpnm, fs_fimpd_t **fimpd);
IMPORT	void	fs_fimp_end_unregist(fs_fimpd_t *fimpd, INT err);

/*
 *  Connection descriptor structure
 */
struct fs_cond {
	fs_desc_t	c_desc;		/* Basic descriptor		*/
	fs_fimpd_t	*c_fimpsd;	/* FIMP descriptor		*/
	coninf_t	c_coninf;	/* Connection information	*/
	fs_cond_t	*c_sync;	/* Sync list pointer		*/
};
#define CON_BUSY	(1)		/* c_desc.d_flags		*/

IMPORT	INT	fs_con_init(INT max);
IMPORT	INT	fs_con_fini(void);
IMPORT	INT	fs_con_get(UINT ident, fs_cond_t **cond);
IMPORT	INT	fs_con_find(const B *connm, fs_cond_t **cond);
IMPORT	void	fs_con_release(fs_cond_t *cond);
IMPORT	INT	fs_con_regist(const B *connm, fs_cond_t **cond);
IMPORT	void	fs_con_end_regist(fs_cond_t *cond, INT err);
IMPORT	INT	fs_con_unregist(const B *connm, fs_cond_t **cond, INT forced);
IMPORT	void	fs_con_end_unregist(fs_cond_t *cond, INT err);

/*
 *  File system context
 */
typedef struct fs_ctx	fs_ctx_t;
struct fs_ctx {
	B		*x_curdir;	/* Current directory string	*/
	B		*x_pathbuf;	/* Path work buffer		*/
	mode_t		x_umask;	/* File creation mask		*/
	INT		x_maxfile;	/* Max open files		*/
	UINT		x_fileids[0];	/* File identifiers [Maxfile]	*/
};
#define	FS_CTX_SIZE(n)		(sizeof(fs_ctx_t) + sizeof(UINT) * (n))

IMPORT	INT	fs_ctx_initialize(fs_ctx_t * const ctx, INT maxfile);
IMPORT	void	fs_ctx_finalize(fs_ctx_t * const ctx, fs_env_t *env);
IMPORT	B	*fs_ctx_get_pathbuf(fs_ctx_t * const ctx);
IMPORT	void	fs_ctx_free_pathbuf(fs_ctx_t * const ctx, B *buf);
IMPORT	INT	fs_ctx_allocfile(fs_ctx_t * const ctx, INT fn);
IMPORT	void	fs_ctx_releasefile(fs_ctx_t * const ctx, INT fn);
IMPORT	void	fs_ctx_freefile(fs_ctx_t * const ctx, INT fn, fs_file_t *file);
IMPORT	void	fs_ctx_setfile(fs_ctx_t * const ctx, INT fn, fs_file_t *fp);

/*
 *  Task specific (local) data
 */
typedef struct fs_tsd	fs_tsd_t;
struct fs_tsd {
	union {			/* Stat flags for fs_break()	*/
		_UW	all;
		struct {		/* for atmoic operation		*/
			_UB	exec_fs;	/* in exec of fs	*/
			_UB	exec_fimp;	/* in exec of fimp	*/
			_UB	break_called;	/* fimp_break() is called */
			_UB	break_done;	/* fimp_break() is doone */
		} c;
	} t_stat;
	fs_cond_t	*t_con;		/* Connection descriptor	*/
};

IMPORT	fs_tsd_t	*fs_tsd_get(ID tskid);

/*
 *  Current environment for execution of file system functions
 */
struct	fs_env {
	fimp_t		t_cmd;		/* FIMP command			*/
	INT		t_errno;	/* errno			*/
	ID		t_tid;		/* Task ID			*/
	fs_ctx_t	*t_ctx;		/* File system context		*/
	fs_tsd_t	*t_tsd;		/* Task local context		*/
	union {			/* Work data			*/
		UINT		t_fid;		/* for open		*/
		struct stat	t_stat;		/* for stat/fstat	*/
		struct stat64_us t_stat64_u;	/* for stat/fstat	*/
		struct timeval t_times[2];	/* for utimes		*/
		SYSTIM_U	t_times_u[2];	/* for utimes_us	*/
		struct {
			size_t	t_len;		/* Length	*/
			off_t		t_off;		/* Offset(32bit) */
			off64_t		t_off64;	/* Offset(64bit) */
			INT		t_flags;	/* Open flags	*/
		} t_rw;				/* for read/write	*/
	} t_misc;
};

/*
 *  FS Configurations
 */
typedef struct fs_config	fs_config_t;
struct fs_config {
	INT	c_maxfile;		/* Maximum number of files	*/
	INT	c_maxfimp;		/* Maximum number of FIMPs	*/
	INT	c_maxcon;		/* Maximum number of Connections */
};

/*
 *  FS initializer/finalizer
 */
IMPORT	INT	fs_tk_is_initialized(void);
IMPORT	INT	fs_tk_initialize(void);
IMPORT	INT	fs_tk_finalize(void);

/*
 *  OS-dependent function prototypes (kernel)
 */

/* Memory allocation & free */
#define	fs_malloc(sz)		Kmalloc(sz)
#define	fs_free(p)		Kfree(p)

/* Mmeory space lock & unlock operations */
#define	fs_lockspace(buf, num)		LockSpace((VP)buf, (INT)num)
#define	fs_unlockspace(buf, num)	UnlockSpace((VP)buf, (INT)num)

IMPORT	INT	fs_task_exist(ID tid);
IMPORT	INT	fs_callfimp(fs_env_t *env, fs_cond_t *con, INT rcode);

/*
 *  Generic variables & functions
 */
IMPORT	fs_regis_t fs_root;			/* Root of the FS	*/

IMPORT	INT	fs_init(fs_env_t *env, const fs_config_t *config);
IMPORT	void	fs_fini(fs_env_t *env);
IMPORT	INT	fs_rootinit(fs_env_t *env);
IMPORT	INT	fs_rootfini(fs_env_t *env);

/*
 *  File system functions
 */
IMPORT	INT	fs_is_dir(fs_env_t *env, const fs_path_t *path);
IMPORT	INT	fs_is_fdir(fs_env_t *env, fs_cond_t *con, fs_file_t *file);
IMPORT	INT	fs_check_fimpnm(const B *fimpnm);
IMPORT	INT	fs_check_connm(const B *fimpnm);
IMPORT	INT	fs_check_64bits(fs_cond_t *con, off64_t len);

IMPORT	INT	xfs_fstat64_us_impl(fs_env_t *env, fs_cond_t *cond,
			fs_file_t *file);
IMPORT	INT	xfs_stat64_us_impl(fs_env_t *env, const fs_path_t *path);

IMPORT	INT	xfs_chdir(fs_env_t *env, const B *path);
IMPORT	INT	xfs_chmod(fs_env_t *env, const B *path, mode_t mode);
IMPORT	INT	xfs_close(fs_env_t *env, INT fno);
IMPORT	INT	xfs_ioctl(fs_env_t *env, INT fno, INT request, void *arg);
IMPORT	INT	xfs_fchdir(fs_env_t *env, INT fno);
IMPORT	INT	xfs_fchmod(fs_env_t *env, INT fno, mode_t mode);
IMPORT	INT	xfs_fcntl(fs_env_t *env, INT fn, INT cmd, void *arg);
IMPORT	INT	xfs_fdatasync(fs_env_t *env, INT fno);
IMPORT	INT	xfs_fsync(fs_env_t *env, INT fno);
IMPORT	INT	xfs_fstat(fs_env_t *env, INT fno, struct stat *ptr);
IMPORT	INT	xfs_fstat_us(fs_env_t *env, INT fno, struct stat_us *ptr);
IMPORT	INT	xfs_fstat_ms(fs_env_t *env, INT fno, struct stat_ms *ptr);
IMPORT	INT	xfs_fstat64(fs_env_t *env, INT fno, struct stat64 *ptr);
IMPORT	INT	xfs_fstat64_us(fs_env_t *env, INT fno, struct stat64_us *ptr);
IMPORT	INT	xfs_fstat64_ms(fs_env_t *env, INT fno, struct stat64_ms *ptr);
IMPORT	INT	xfs_fstatvfs(fs_env_t *env, INT fno, struct statvfs *ptr);
IMPORT	INT	xfs_ftruncate64(fs_env_t *env, INT fno, off64_t len);
IMPORT	INT	xfs_truncate64(fs_env_t *env, const B *name, off64_t len);
IMPORT	INT	xfs_getcwd(fs_env_t *env, B *buf, size_t size);
IMPORT	INT	xfs_lseek(fs_env_t *env, INT fno, off_t off,
			INT whence, off_t *newoff);
IMPORT	INT	xfs_lseek64(fs_env_t *env, INT fno, off64_t off, INT whence,
			off64_t *newoff);
IMPORT	INT	xfs_mkdir(fs_env_t *env, const B *path, mode_t mode);
IMPORT	INT	fs_mkfifo(fs_env_t *env, const B *path, mode_t mode);
IMPORT	INT	xfs_open(fs_env_t *env, const B *path, INT flags, mode_t mode);
IMPORT	INT	xfs_read(fs_env_t *env, INT fno, void *buf, size_t num);
IMPORT	INT	xfs_getdents(fs_env_t *env, INT fno, struct dirent *buf,
			INT bufsz);
IMPORT	INT	xfs_rename(fs_env_t *env, const B *oldn, const B *newn);
IMPORT	INT	xfs_rmdir(fs_env_t *env, const B *path);
IMPORT	INT	xfs_stat(fs_env_t *env, const B *path, struct stat *ptr);
IMPORT	INT	xfs_stat_us(fs_env_t *env, const B *path,
			struct stat_us *ptr);
IMPORT	INT	xfs_stat_ms(fs_env_t *env, const B *path,
			struct stat_ms *ptr);
IMPORT	INT	xfs_stat64(fs_env_t *env, const B *path, struct stat64 *ptr);
IMPORT	INT	xfs_stat64_us(fs_env_t *env, const B *path,
			struct stat64_us *ptr);
IMPORT	INT	xfs_stat64_ms(fs_env_t *env, const B *path,
			struct stat64_ms *ptr);
IMPORT	INT	xfs_statvfs(fs_env_t *env, const B *path,
			struct statvfs *ptr);
IMPORT	INT	xfs_unlink(fs_env_t *env, const B *path);
IMPORT	INT	xfs_utimes(fs_env_t *env, const B *path,
			const struct timeval *tim);
IMPORT	INT	xfs_utimes_us(fs_env_t *env, const B *path,
			const SYSTIM_U *tim_u);
IMPORT	INT	xfs_utimes_ms(fs_env_t *env, const B *path,
			const SYSTIM *tim);
IMPORT	INT	xfs_write(fs_env_t *env, INT fno, const void *buf,
			size_t num);

IMPORT	INT	xfs_regist(fs_env_t *env, const B *fimpnm,
			const fs_fimp_t *fimp, void *exinf, void *tsd);
IMPORT	INT	xfs_unregist(fs_env_t *env, const B *fimpnm);
IMPORT	INT	xfs_attach(fs_env_t *env, const B *devnm, const B *connm,
			const B *fimpnm, INT flags, void *exinf);
IMPORT	INT	xfs_detach(fs_env_t *env, const B *connm, INT forced);

IMPORT	INT	xfs_sync(fs_env_t *env);

IMPORT	INT	xfs_break(ID tskid);
IMPORT	void	xfs_break_impl(fs_cond_t *con, ID tskid, BOOL set);

#ifdef	__cplusplus
}
#endif

#endif /* __FSDEFS_H__ */

