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
 *	@(#)fs_trunc.c
 *
 */

#include "fsdefs.h"

/*
 *  Check 64 bits length / offset
 */
EXPORT	INT	fs_check_64bits(fs_cond_t *con, off64_t len)
{
	if ((con->c_fimpsd->p_fimp.flags & FIMP_FLAG_64BIT) == 0) {
		if (len > ULONG_MAX)	return EX_OVERFLOW;
#if	SIGNED_OFFSET
		if (len > LONG_MAX)	return EX_INVAL;
#endif
	}
	return 0;
}

/*
 *  fs_ftruncate(), fs_ftruncate64() - Truncate file by file number
 */
EXPORT	INT	xfs_ftruncate64(fs_env_t *env, INT fno, off64_t len)
{
	fs_file_t	*file;
	fs_cond_t	*con;
	INT		sts;

	/* Get file & connection descriptor */
	sts = fs_file_get(env, fno, &file, &con);
	if (sts != 0) goto exit0;

	/* Check writable */
	if ((con->c_coninf.dflags & DEV_FLAG_READONLY) != 0) {
		sts = EX_ROFS;
		goto exit1;
	}
	if ((file->f_desc.d_flags & O_ACCMODE) == O_RDONLY) {
		sts = EX_BADF;
		goto exit1;
	}

	/* Check directory or not */
	sts = fs_is_fdir(env, con, file);
	if (sts != 0) {
		if (sts == 1) sts = EX_ISDIR;
		goto exit1;
	}

	/* Check 64 bit offset */
	sts = fs_check_64bits(con, len);
	if (sts == 0) {
		/* Call FIMP FTRUNCATE64 service */
		env->t_cmd.r_ftruncate64.fid = file->f.f_fid;
		env->t_cmd.r_ftruncate64.len = len;
		sts = fs_callfimp(env, con, FIMP_FTRUNCATE64);
	}
	if (sts == 0) {
		/* Update file status */
		if (len < FILE_OFFSET(file)) {
			FILE_OFFSET(file) = len;
		}
	}

exit1:
	/* Release file & connection descriptor */
	(void)fs_file_release(env, file, con);
	if (sts == 0) return 0;
exit0:
	env->t_errno = sts;
	return -1;
}

/*
 *  fs_truncate(), fs_truncate64() - Truncate file by name
 */
EXPORT	INT	xfs_truncate64(fs_env_t *env, const B *name, off64_t len)
{
	fs_path_t	path;
	INT		sts;

	/* Parse the pathname & get path structure */
	sts = fs_path_parse(env, name, &path, TRUE);
	if (sts != 0) goto exit0;

	/* Check directory or not */
	sts = fs_is_dir(env, &path);
	if (sts != 0) {
		if (sts == 1) sts = EX_ISDIR;
		goto exit1;
	}

	/* Check 64 bit offset */
	sts = fs_check_64bits(path.p_con, len);
	if (sts == 0) {
		/* Call FIMP TRUNCATE64 service */
		env->t_cmd.r_truncate64.path = path.p_name;
		env->t_cmd.r_truncate64.len = len;
		sts = fs_callfimp(env, path.p_con, FIMP_TRUNCATE64);
	}
exit1:
	/* Release path structure */
	fs_path_release(env, &path);
	if (sts == 0) return 0; 
exit0:
	env->t_errno = sts;
	return -1;
}

