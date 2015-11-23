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
 *	@(#)fs_init.c
 *
 */

#include "fsdefs.h"

IMPORT	fs_regis_t	fimp_root;		/* FIMP "root" */

/*
 *  Detach and unregist all file systems
 */
LOCAL	INT	fs_all_unregist(fs_env_t *env)
{
	QUEUE	*q, *p;
	INT	sts, err;

	sts = 0;

	/* Detach all cpnnections : Skip 1st entry "root" */
	for (q = fs_root.r_list.next->next; q != &fs_root.r_list; q = p) {
		p = q->next;
		err = xfs_detach(env, ((fs_cond_t *)q)->c_coninf.connm, 1);
		if (sts == 0) sts = err;
	}

	/* Unregist all FIMPs : Skip 1st entry "rootfs" */
	for (q = fimp_root.r_list.next->next; q != &fimp_root.r_list; q = p) {
		p = q->next;
		err = xfs_unregist(env, ((fs_fimpd_t *)q)->p_fimpinf.fimpnm);
		if (sts == 0) sts = err;
	}
	return sts;
}

/*
 *  File system initialization
 */
EXPORT	INT	fs_init(fs_env_t *env, const fs_config_t *config)
{
	INT	sts;

	sts = fs_file_init(config->c_maxfile);
	if (sts == 0) {
		sts = fs_fimp_init(config->c_maxfimp);
		if (sts == 0) {
			sts = fs_con_init(config->c_maxcon);
			if (sts == 0) {
				sts = fs_rootinit(env);
			}
		}
	}
	return sts;
}

/*
 *  File system finalization
 */
EXPORT	void	fs_fini(fs_env_t *env)
{
	(void)fs_all_unregist(env);
	(void)fs_rootfini(env);
	(void)fs_file_fini();
	(void)fs_con_fini();
	(void)fs_fimp_fini();
}

