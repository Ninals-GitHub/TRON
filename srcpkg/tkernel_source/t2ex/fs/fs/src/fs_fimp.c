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
 *	@(#)fs_fimp.c
 *
 */

#include "fsdefs.h"

EXPORT	fs_regis_t	fimp_root;	/* FIMP descriptor list root	*/
					/* Note: "r_count" isn't used	*/

LOCAL	fs_descmgr_t	fimp_descmgr;	/* FIMP descriptor manager	*/

/*
 *  Initialize FIMP manager
 */
EXPORT	INT	fs_fimp_init(INT max)
{
	QueInit(&fimp_root.r_list);
	return fs_descmgr_init(&fimp_descmgr, max,
					sizeof(fs_fimpd_t), DESC_FILESYS);
}

/*
 *  Finalize FIMP manager
 */
EXPORT	INT	fs_fimp_fini(void)
{
	return fs_descmgr_fini(&fimp_descmgr);
}

/*
 *  Release FIMP descriptor obtained by fs_fimp_find()
 */
EXPORT	void	fs_fimp_release(fs_fimpd_t *fimpd)
{
	/* Decrement reference count */
	if (fimpd->p_desc.d_refc > 0) {
		fimpd->p_desc.d_refc--;
	}
}

/*
 *  Search FIMP descriptor - should be called with lock
 */
LOCAL	fs_fimpd_t	*fs_fimp_search(const B *fimpnm)
{
	fs_fimpd_t	*fimpd;

	for (fimpd = (fs_fimpd_t *)fimp_root.r_list.next;
		(QUEUE*)fimpd != &fimp_root.r_list;
			fimpd = (fs_fimpd_t *)fimpd->p_desc.d_list.next) {
		if (strcmp(fimpd->p_fimpinf.fimpnm, fimpnm) == 0) {
			return fimpd;	/* Found */
		}
	}
	return NULL;	/* Not found */
}

/*
 *  Find & get FIMP descriptor by name
 */
EXPORT	INT	fs_fimp_find(const B *fimpnm, fs_fimpd_t **fimpd)
{
	INT		sts;
	fs_fimpd_t	*p;

	fs_lock_lock(LOCKNUM_FIMP);

	p = fs_fimp_search(fimpnm);
	if (p == NULL) {
		sts = EX_NOENT;
	} else if ((p->p_desc.d_flags & FIMP_BUSY) != 0) {
		sts = EX_BUSY;
		p = NULL;
	} else {	/* Found, increment reference count to avoid delete */
		p->p_desc.d_refc++;
		sts = 0;
	}

	fs_lock_unlock(LOCKNUM_FIMP);
	*fimpd = p;
	return sts;
}

/*
 *  Regist FIMP descriptor by name
 */
EXPORT	INT	fs_fimp_regist(const B *fimpnm, fs_fimpd_t **fimpd)
{
	INT		sts;
	fs_fimpd_t	*p;

	fs_lock_lock(LOCKNUM_FIMP);

	p = fs_fimp_search(fimpnm);
	if (p != NULL) {
		sts = EX_EXIST;
	} else if ((p = (fs_fimpd_t *)
				fs_descmgr_alloc(&fimp_descmgr)) == NULL) {
		sts = EX_NOBUFS;
	} else {
		/* append to "root" with BUSY */
		strcpy(p->p_fimpinf.fimpnm, fimpnm);
		p->p_fimpinf.fimpsd = NULL;
		p->p_desc.d_flags |= FIMP_BUSY;
		QueInsert(&p->p_desc.d_list, &fimp_root.r_list);
		sts = 0;
	}

	fs_lock_unlock(LOCKNUM_FIMP);
	*fimpd = p;
	return sts;
}

/*
 *  End of registration of FIMP descriptor
 */
EXPORT	void	fs_fimp_end_regist(fs_fimpd_t *fimpd, INT err)
{
	fs_lock_lock(LOCKNUM_FIMP);

	if (err != 0) {	/* Error : Cancel registration */
		/* Remove from "root" & release FIMP descriptor */
		QueRemove(&fimpd->p_desc.d_list);
		fs_descmgr_free(&fimp_descmgr, &fimpd->p_desc);
	} else {		/* OK : Complete registration */
		/* Validate isn't required */
		/*(void)fs_descmgr_validate(&fimp_descmgr, &fimpd->p_desc);*/
		fimpd->p_desc.d_flags &= ~FIMP_BUSY;	/* Reset BUSY */
	}

	fs_lock_unlock(LOCKNUM_FIMP);
}

/*
 *  Unregist FIMP descriptor by name
 */
EXPORT	INT	fs_fimp_unregist(const B *fimpnm, fs_fimpd_t **fimpd)
{
	INT		sts;
	fs_fimpd_t	*p;

	fs_lock_lock(LOCKNUM_FIMP);

	p = fs_fimp_search(fimpnm);
	if (p == NULL || (p->p_desc.d_flags & FIMP_BUSY) != 0) {
		sts = EX_NOENT;
	} else if (p->p_desc.d_refc != 0) {
		sts = EX_BUSY;
	} else {
		p->p_desc.d_flags |= FIMP_BUSY;
		sts = 0;
	}

	fs_lock_unlock(LOCKNUM_FIMP);
	*fimpd = p;
	return sts;
}

/*
 *  End of unregistration of FIMP descriptor
 */
EXPORT	void	fs_fimp_end_unregist(fs_fimpd_t *fimpd, INT err)
{
	fs_lock_lock(LOCKNUM_FIMP);

	if (err != 0) {	/* Error : Cancel unregistration */
		fimpd->p_desc.d_flags &= ~FIMP_BUSY;
	} else {		/* OK : Complete unregistration */
		/* Remove from "root" */
		QueRemove(&fimpd->p_desc.d_list);
		/* Free FIMP descriptor */
		fs_descmgr_free(&fimp_descmgr, &fimpd->p_desc);
	}

	fs_lock_unlock(LOCKNUM_FIMP);
}

