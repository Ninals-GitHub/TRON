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
 *	@(#)fs_sync.c
 *
 */

#include "fsdefs.h"

/*
 *  fs_sync() - Synchronization
 */
EXPORT	INT	xfs_sync(fs_env_t * env)
{
	fs_cond_t	*sync, *con;
	INT		sts, err;

	/* Make connection list */
	fs_lock_lock(LOCKNUM_ROOT);

	/* Skip 1st entry "root" */
	for (sync = NULL, con = (fs_cond_t *)fs_root.r_list.next->next;
		(QUEUE*)con != &fs_root.r_list;
			con = (fs_cond_t *)con->c_desc.d_list.next) {
		if ((con->c_coninf.dflags & DEV_FLAG_READONLY) == 0) {
			con->c_sync = sync;	/* Lint to sync list */
			sync = con;
			sync->c_desc.d_refc++;	/* Avoid detach */
		}
	}
	fs_lock_unlock(LOCKNUM_ROOT);

	/* Sync each connection */
	for (sts = 0; sync != NULL; sync = sync->c_sync) {
		/* Call FIMP SYNC service */
		err = fs_callfimp(env, sync, FIMP_SYNC);
		if (err != 0) sts = err;
		sync->c_desc.d_refc--;		/* Permit detach */
	}

	if (sts == 0) return 0;
	env->t_errno = sts;
	return -1;
}

