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
 *	@(#)fs_fcntl.c
 *
 */

#include "fsdefs.h"

/*
 *  F_DUPFD function
 */
LOCAL	INT	dupfd(fs_env_t *env, fs_file_t *file, INT arg, INT *retv)
{
	INT	sts;

	if (arg >= env->t_ctx->x_maxfile) {
		sts = EX_INVAL;
	} else {
		/* Alloaction new file number */
		sts = fs_ctx_allocfile(env->t_ctx, arg);
		if (sts < 0) {
			sts = EX_MFILE;
		} else {
			fs_ctx_setfile(env->t_ctx, sts, file);
			*retv = sts;
			sts = 0;
		}
	}
	return sts;
}

/*
 *  F_GETFD function
 */
LOCAL	INT	getfd(fs_env_t *env, INT fno, fs_file_t *file, INT *retv)
{
	*retv = (INT)(env->t_ctx->x_fileids[fno] & FD_CLOEXEC);
	return 0;
}

/*
 *  F_SETFD function
 */
LOCAL	INT	setfd(fs_env_t *env, INT fno, fs_file_t *file, INT arg,
							INT *retv)
{
	UINT	*ip;

	ip = &env->t_ctx->x_fileids[fno];

	fs_lock_lock(LOCKNUM_CTX);

	if (((*retv = (INT)*ip) | FD_CLOEXEC) == (INT)file->f_desc.d_ident) {
		*ip = (*retv & ~FD_CLOEXEC) | (arg & FD_CLOEXEC);
		ip = NULL;
	}
	fs_lock_unlock(LOCKNUM_CTX);

	*retv &= FD_CLOEXEC;
	return (ip != NULL) ? EX_BADF : 0;
}

/*
 *  F_GETFL function
 */
LOCAL	INT	getfl(fs_env_t *env, fs_file_t *file, INT *retv)
{
	*retv = file->f_desc.d_flags &
		(O_ACCMODE|O_NONBLOCK|O_APPEND|O_DSYNC|O_RSYNC|O_SYNC);
	return 0;
}

/*
 *  F_SETFL function
 */
LOCAL	INT	setfl(fs_env_t *env, fs_file_t *file, INT arg, INT *retv)
{
	fs_lock_lock(LOCKNUM_FIL);
	*retv = file->f_desc.d_flags &
		(O_ACCMODE|O_NONBLOCK|O_APPEND|O_DSYNC|O_RSYNC|O_SYNC);
	file->f_desc.d_flags = (UH)(file->f_desc.d_flags &
			~(O_NONBLOCK|O_APPEND|O_DSYNC|O_RSYNC|O_SYNC))
		| (arg & (O_NONBLOCK|O_APPEND|O_DSYNC|O_RSYNC|O_SYNC));
	fs_lock_unlock(LOCKNUM_FIL);
	return 0;
}

/*
 *  fs_fcntl() : Generic file control function
 */
LOCAL	INT	xfs_fcntl_fs(fs_env_t *env, INT cmd, INT fno,
						void *arg, INT *retv)
{
	fs_file_t	*file;
	INT		sts;

	/* Get file descriptor */
	sts = fs_file_get(env, fno, &file, NULL);
	if (sts != 0) goto exit0;

	switch (cmd) {
	case F_DUPFD:
		sts = dupfd(env, file, (INT)arg, retv);
		if (sts == 0) goto exit0; /* Don't release file descriptor */
		break;
	case F_GETFD:
		sts = getfd(env, fno, file, retv);
		break;
	case F_SETFD:
		sts = setfd(env, fno, file, (INT)arg, retv);
		break;
	case F_GETFL:
		sts = getfl(env, file, retv);
		break;
	case F_SETFL:
		sts = setfl(env, file, (INT)arg, retv);
		break;
	default:
		sts = EX_INVAL;
		break;
	}
	/* Release file descriptor */
	(void)fs_file_release(env, file, NULL);
exit0:
	return sts;
}

/*
 *  fs_fcntl() : FIMP depend file control function
 */
LOCAL	INT	xfs_fcntl_fimp(fs_env_t *env, INT fcmd, INT fno,
						void *arg, INT *retv)
{
	fs_file_t	*file;
	fs_cond_t	*con;
	fimp_t		*cmd;
	INT		sts;

	/* Get file & connection descriptor */
	sts = fs_file_get(env, fno, &file, &con);
	if (sts != 0) goto exit0;

	/* Call FIMP FCNTL64 service */
	env->t_misc.t_rw.t_flags = file->f_desc.d_flags;
	env->t_misc.t_rw.t_off64 = FILE_OFFSET(file);

	cmd = &env->t_cmd;
	cmd->r_fcntl64.fcmd = fcmd;
	cmd->r_fcntl64.fid = file->f.f_fid;
	cmd->r_fcntl64.oflags = &env->t_misc.t_rw.t_flags;
	cmd->r_fcntl64.off = (off64_t *)&env->t_misc.t_rw.t_off64;
	cmd->r_fcntl64.arg = arg;
	cmd->r_fcntl64.retval = retv;
	sts = fs_callfimp(env, con, FIMP_FCNTL64);
	if (sts == 0) {
		file->f_desc.d_flags = env->t_misc.t_rw.t_flags;
		FILE_OFFSET(file) = env->t_misc.t_rw.t_off64;
	}
	/* Release file & connection descriptor */
	(void)fs_file_release(env, file, con);
exit0:
	return sts;
}

/*
 *  fs_fcntl() - File control
 */
EXPORT	INT	xfs_fcntl(fs_env_t *env, INT fno, INT cmd, void *arg)
{
	INT	sts, retv;

	retv = 0;

	if (((cmd >> FCTLGROUP_SHIFT) & FCTLGROUP_MASK) == 'f') {/* Generic */
		sts = xfs_fcntl_fs(env, cmd, fno, arg, &retv);
	} else {	/* FIMP depend */
		sts = xfs_fcntl_fimp(env, cmd, fno, arg, &retv);
	}
	if (sts == 0) return retv;
	env->t_errno = sts;
	return -1;
}

