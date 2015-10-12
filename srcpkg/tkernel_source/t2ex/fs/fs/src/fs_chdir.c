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
 *	@(#)fs_chdir.c
 *
 */

#include "fsdefs.h"

/*
 *  fs_chdir() - Change current working directory by pathname
 */
EXPORT	INT	xfs_chdir(fs_env_t *env, const B *name)
{
	fs_path_t	path;
	INT		sts;

	/* Parse the pathname & get path structure */
	sts = fs_path_parse(env, name, &path, FALSE);
	if (sts != 0) goto exit0;

	/* Check if the path is directory */
	sts = fs_is_dir(env, &path);
	if (sts != 1) {
		if (sts == 0) sts = EX_NOTDIR;
		goto exit1;
	}

	/* Lock curdir */
	fs_lock_lock(LOCKNUM_CDIR);

	/* Call FIMP CHDIR service */
	env->t_cmd.r_chdir.path = path.p_name;
	sts = fs_callfimp(env, path.p_con, FIMP_CHDIR);
	if (sts == 0) {
		/* Set new curdir */
		strcpy(env->t_ctx->x_curdir, path.p_name);
	}
	/* Unlock curdir */
	fs_lock_unlock(LOCKNUM_CDIR);
exit1:
	/* Release path structure */
	fs_path_release(env, &path);
	if (sts == 0) return 0;
exit0:
	env->t_errno = sts;
	return -1;
}

