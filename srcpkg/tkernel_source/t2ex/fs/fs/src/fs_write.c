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
 *	@(#)fs_write.c
 *
 */

#include "fsdefs.h"

/*
 *  Write data to file
 */
LOCAL	INT	xfs_write_impl(fs_env_t *env, fs_cond_t *con, fs_file_t *file,
				const void *buf, size_t num, off64_t off)
{
	fimp_t	*cmd;
	INT	sts;

	sts = 0;
	if ((INT)num < 0) {
		sts = EX_INVAL;
		goto exit0;
	}
	if (fs_lockspace((void *)buf, num) != 0) {
		sts = EX_ACCES;
		goto exit0;
	}

	/* Check 64 bits offset */
	sts = fs_check_64bits(con, off);
	if (sts == 0) {
		/* Call FIMP WRITE64 service */
		env->t_misc.t_rw.t_len = num;
		env->t_misc.t_rw.t_off64 = off;

		cmd = &env->t_cmd;
		cmd->r_write64.fid = file->f.f_fid;
		cmd->r_write64.oflags = file->f_desc.d_flags;
		cmd->r_write64.buf = (void *)buf;
		cmd->r_write64.len = &env->t_misc.t_rw.t_len;
		cmd->r_write64.off = (off64_t *)&env->t_misc.t_rw.t_off64;
		cmd->r_write64.retoff = (off64_t *)&env->t_misc.t_rw.t_off64;
		sts = fs_callfimp(env, con, FIMP_WRITE64);
		FILE_OFFSET(file) = env->t_misc.t_rw.t_off64;
	}
	(void)fs_unlockspace((void *)buf, num);
exit0:
	return sts;
}

/*
 *  fs_write() - Write data to file
 */
EXPORT	INT	xfs_write(fs_env_t *env, INT fno,
					const void *buf, size_t num)
{
	fs_file_t	*file;
	fs_cond_t	*con;
	INT		sts;

	env->t_misc.t_rw.t_len = 0;

	/* Get file and connection descriptor */
	sts = fs_file_get(env, fno, &file, &con);
	if (sts != 0) goto exit0;

	/* Check open mode */
	if ((file->f_desc.d_flags & O_ACCMODE) == O_RDONLY) {
		sts = EX_BADF;
		goto exit1;
	}

	/* Write data */
	if (num != 0) {
		sts = xfs_write_impl(env, con, file,
					buf, num, FILE_OFFSET(file));
	}
exit1:
	/* Release file and connection descriptor */
	(void)fs_file_release(env, file, con);
	if (sts == 0) return (INT)env->t_misc.t_rw.t_len;
exit0:
	env->t_errno = sts;
	return -1;
}

