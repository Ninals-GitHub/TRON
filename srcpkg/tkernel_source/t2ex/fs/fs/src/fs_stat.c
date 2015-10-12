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
 *	@(#)fs_stat.c
 *
 */

#include "fsdefs.h"

/*
 *  Common structure for statXXX
 */
typedef	union {
	struct stat		s;
	struct stat_us	u;
	struct stat_ms	m;
	struct stat64		s64;
	struct stat64_us	u64;
	struct stat64_ms	m64;
} Ustat;

/* statXXX kind */
#define	K_STAT			0x00
#define	K_STAT_US		0x01
#define	K_STAT_MS		0x02
#define	K_STAT64		0x10
#define	K_STAT64_MS		0x11
#define	K_STAT64_US		0x12

/*
 *  Convert SYSTIM_U to time_t
 */
LOCAL	void	SYSTIM_U_to_time(SYSTIM_U tim_u, time_t *time)
{
	struct tm	tm;

	if (tim_u == 0) {
		*time = 0;
	} else {
		(void)dt_localtime_us(tim_u, NULL, &tm);
		(void)dt_mktime(&tm, NULL, time);
	}
}

/*
 *  Convert SYSTIM_U to SYSTIM
 */
LOCAL	void	SYSTIM_U_to_SYSTIM(SYSTIM_U tim_u, SYSTIM *tim)
{
	SYSTIM_U	tmp;

	if (tim_u == 0) {
		tim->hi = 0;
		tim->lo = 0;
	} else {
		tmp = tim_u / 1000;
		tim->hi = (W)((tmp >> 32) & 0x7FFFFFFF);
		tim->lo = (UW)(tmp & 0xFFFFFFFF);
	}
}

/*
 *  Set statXXX from stat64_us
 */
LOCAL	INT	set_statXXX(INT kind, struct stat64_us *st64u, Ustat *ust)
{
	if ((kind & K_STAT64) == 0) {
		/* Check 32 bits overflow */
		if ((st64u->st_ino > ULONG_MAX)
			|| ((UD)st64u->st_size > ULONG_MAX)
			|| ((UD)st64u->st_blocks > ULONG_MAX)) {
			return EX_OVERFLOW;
		}
		ust->s.st_size = (off_t)st64u->st_size;
	} else {
		ust->s64.st_size = st64u->st_size;
	}

	/* Set common fields of statXXX */
	ust->s.st_dev = st64u->st_dev;
	ust->s.st_ino = (ino_t)st64u->st_ino;
	ust->s.st_mode = st64u->st_mode;
	ust->s.st_nlink = st64u->st_nlink;
	ust->s.st_uid = st64u->st_uid;
	ust->s.st_gid = st64u->st_gid;
	ust->s.st_rdev = st64u->st_rdev;

	/* Set variant fields of statXXX */
	switch(kind) {
	case K_STAT:
		SYSTIM_U_to_time(st64u->st_atime_u, &ust->s.st_atime);
		SYSTIM_U_to_time(st64u->st_mtime_u, &ust->s.st_mtime);
		SYSTIM_U_to_time(st64u->st_ctime_u, &ust->s.st_ctime);
		ust->s.st_blksize = st64u->st_blksize;
		ust->s.st_blocks = st64u->st_blocks;
		break;
	case K_STAT_US:
		ust->u.st_atime_u = st64u->st_atime_u;
		ust->u.st_mtime_u = st64u->st_mtime_u;
		ust->u.st_ctime_u = st64u->st_ctime_u;
		ust->u.st_blksize = st64u->st_blksize;
		ust->u.st_blocks = st64u->st_blocks;
		break;
	case K_STAT_MS:
		SYSTIM_U_to_SYSTIM(st64u->st_atime_u, &ust->m.st_atime);
		SYSTIM_U_to_SYSTIM(st64u->st_mtime_u, &ust->m.st_mtime);
		SYSTIM_U_to_SYSTIM(st64u->st_ctime_u, &ust->m.st_ctime);
		ust->m.st_blksize = st64u->st_blksize;
		ust->m.st_blocks = st64u->st_blocks;
		break;
	case K_STAT64:
		SYSTIM_U_to_time(st64u->st_atime_u, &ust->s64.st_atime);
		SYSTIM_U_to_time(st64u->st_mtime_u, &ust->s64.st_mtime);
		SYSTIM_U_to_time(st64u->st_ctime_u, &ust->s64.st_ctime);
		ust->s64.st_blksize = st64u->st_blksize;
		ust->s64.st_blocks = st64u->st_blocks;
		break;
	case K_STAT64_US:
		ust->u64.st_atime_u = st64u->st_atime_u;
		ust->u64.st_mtime_u = st64u->st_mtime_u;
		ust->u64.st_ctime_u = st64u->st_ctime_u;
		ust->u64.st_blksize = st64u->st_blksize;
		ust->u64.st_blocks = st64u->st_blocks;
		break;
	case K_STAT64_MS:
		SYSTIM_U_to_SYSTIM(st64u->st_atime_u, &ust->m64.st_atime);
		SYSTIM_U_to_SYSTIM(st64u->st_mtime_u, &ust->m64.st_mtime);
		SYSTIM_U_to_SYSTIM(st64u->st_ctime_u, &ust->m64.st_ctime);
		ust->m64.st_blksize = st64u->st_blksize;
		ust->m64.st_blocks = st64u->st_blocks;
		break;
	}
	return 0;
}

/*
 *  Call FIMP STAT64_US service
 */
EXPORT	INT	xfs_stat64_us_impl(fs_env_t *env, const fs_path_t *path)
{
	env->t_cmd.r_stat64_us.path = path->p_name;
	env->t_cmd.r_stat64_us.buf = &env->t_misc.t_stat64_u;
	return fs_callfimp(env, path->p_con, FIMP_STAT64_US);
}

/*
 *  Call FIMP FSTAT64_US service
 */
EXPORT	INT	xfs_fstat64_us_impl(fs_env_t *env, fs_cond_t *con,
							fs_file_t *file)
{
	env->t_cmd.r_fstat64_us.fid = file->f.f_fid;
	env->t_cmd.r_fstat64_us.buf = &env->t_misc.t_stat64_u;
	return fs_callfimp(env, con, FIMP_FSTAT64_US);
}

/*
 *  Check directory or not : returns 1: dir, 0: file, < 0: error
 */
EXPORT	INT	fs_is_dir(fs_env_t *env, const fs_path_t *path)
{
	INT	sts;

	sts = xfs_stat64_us_impl(env, path);
	if (sts == 0) {
		sts = S_ISDIR(env->t_misc.t_stat64_u.st_mode) ? 1 : 0;
	}
	return sts;
}

/*
 *  Check directory or not : returns 1: dir, 0: file, < 0: error
 */
EXPORT	INT	fs_is_fdir(fs_env_t *env, fs_cond_t *con, fs_file_t *file)
{
	INT	sts;

	sts = xfs_fstat64_us_impl(env, con, file);
	if (sts == 0) {
		sts = S_ISDIR(env->t_misc.t_stat64_u.st_mode) ? 1 : 0;
	}
	return sts;
}

/*
 *  Get statXXX by pathname
 */
LOCAL	INT	get_stat64(fs_env_t *env, const B *name, Ustat *ust, INT kind)
{
	fs_path_t	path;
	INT		sts;

	/* Parse the pathname & get path structure */
	sts = fs_path_parse(env, name, &path, FALSE);
	if (sts != 0) goto exit0;

	/* Call FIMP STAT64_US service */
	sts = xfs_stat64_us_impl(env, &path);
	if (sts == 0) {
		/* Set result */
		sts = set_statXXX(kind, &env->t_misc.t_stat64_u, ust);
	}
	/* Release path structure */
	fs_path_release(env, &path);
exit0:
	if (sts == 0) return 0;
	env->t_errno = sts;
	return -1;
}

/*
 *  Get statXXX by file descriptor
 */
LOCAL	INT	get_fstat64(fs_env_t *env, INT fno, Ustat *ust, INT kind)
{
	fs_file_t	*file;
	fs_cond_t	*con;
	INT		sts;

	/* Get file & connection descriptor */
	sts = fs_file_get(env, fno, &file, &con);
	if (sts != 0) goto exit0;

	/* Call FIMP FSTAT64_US service */
	sts = xfs_fstat64_us_impl(env, con, file);
	if (sts == 0) {
		/* Set result */
		sts = set_statXXX(kind, &env->t_misc.t_stat64_u, ust);
	}
	/* Release file & connection descriptor */
	(void)fs_file_release(env, file, con);
	if (sts == 0) return 0;
exit0:
	env->t_errno = sts;
	return -1;
}

/*
 *  fs_stat() - Get file status by name
 */
EXPORT	INT	xfs_stat(fs_env_t *env, const B *name, struct stat *ptr)
{
	return get_stat64(env, name, (Ustat*)ptr, K_STAT);
}

/*
 *  fs_stat_us() - Get file status by name
 */
EXPORT	INT	xfs_stat_us(fs_env_t *env,
				const B *name, struct stat_us *ptr)
{
	return get_stat64(env, name, (Ustat*)ptr, K_STAT_US);
}

/*
 *  fs_stat_ms() - Get file status by name
 */
EXPORT	INT	xfs_stat_ms(fs_env_t *env,
				const B *name, struct stat_ms *ptr)
{
	return get_stat64(env, name, (Ustat*)ptr, K_STAT_MS);
}

/*
 *  fs_stat64() - Get file status by name
 */
EXPORT	INT	xfs_stat64(fs_env_t *env, const B *name, struct stat64 *ptr)
{
	return get_stat64(env, name, (Ustat*)ptr, K_STAT64);
}

/*
 *  fs_stat64_us() - Get file status by name
 */
EXPORT	INT	xfs_stat64_us(fs_env_t *env, const B *name,
						struct stat64_us *ptr)
{
	return get_stat64(env, name, (Ustat*)ptr, K_STAT64_US);
}

/*
 *  fs_stat64_ms() - Get file status by name
 */
EXPORT	INT	xfs_stat64_ms(fs_env_t *env, const B *name,
						struct stat64_ms *ptr)
{
	return get_stat64(env, name, (Ustat*)ptr, K_STAT64_MS);
}

/*
 *  fs_fstat() - Get file status by file descriptor
 */
EXPORT	INT	xfs_fstat(fs_env_t *env, INT fno, struct stat *ptr)
{
	return get_fstat64(env, fno, (Ustat*)ptr, K_STAT);
}

/*
 *  fs_fstat_us() - Get  file status by file descriptor
 */
EXPORT	INT	xfs_fstat_us(fs_env_t *env, INT fno, struct stat_us *ptr)
{
	return get_fstat64(env, fno, (Ustat*)ptr, K_STAT_US);
}

/*
 *  fs_fstat_ms() - Get file status by file descriptor
 */
EXPORT	INT	xfs_fstat_ms(fs_env_t *env, INT fno, struct stat_ms *ptr)
{
	return get_fstat64(env, fno, (Ustat*)ptr, K_STAT_MS);
}

/*
 *  fs_fstat64() - Get file status by file descriptor
 */
EXPORT	INT	xfs_fstat64(fs_env_t *env, INT fno, struct stat64 *ptr)
{
	return get_fstat64(env, fno, (Ustat*)ptr, K_STAT64);
}

/*
 *  fs_fstat64_us() - Get file status by file descriptor
 */
EXPORT	INT	xfs_fstat64_us(fs_env_t *env, INT fno, struct stat64_us *ptr)
{
	return get_fstat64(env, fno, (Ustat*)ptr, K_STAT64_US);
}

/*
 *  fs_fstat64_ms() - Get file status by file descriptor
 */
EXPORT	INT	xfs_fstat64_ms(fs_env_t *env, INT fno, struct stat64_ms *ptr)
{
	return get_fstat64(env, fno, (Ustat*)ptr, K_STAT64_MS);
}

