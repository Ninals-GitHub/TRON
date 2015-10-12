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
 *	@(#)fs_renam.c
 *
 */

#include "fsdefs.h"

/*
 *  fs_rename() - Rename file
 */
EXPORT	INT	xfs_rename(fs_env_t * env, const B *oldn, const B *newn)
{
	fs_path_t	opath, npath;
	INT		sts;

	/* Parse the old pathname & get path structure */
	sts = fs_path_parse(env, oldn, &opath, TRUE);
	if (sts != 0) goto exit0;

	/* Parse the new pathname & get path structure */
	sts = fs_path_parse(env, newn, &npath, FALSE);
	if (sts != 0) goto exit1;

	if (opath.p_con != npath.p_con) {
		/* Valid only if both connection is the same */
		sts = EX_XDEV;
	} else {
		/* Call FIMP RENAME service */
		env->t_cmd.r_rename.oldpath = opath.p_name;
		env->t_cmd.r_rename.newpath = npath.p_name;
		sts = fs_callfimp(env, opath.p_con, FIMP_RENAME);
	}
	/* Release new path structure */
	fs_path_release(env, &npath);
exit1:
	/* Release old path structure */
	fs_path_release(env, &opath);
	if (sts == 0) return 0; 
exit0:
	env->t_errno = sts;
	return -1;
}

