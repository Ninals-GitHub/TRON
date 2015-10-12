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
 *	@(#)fs_con.c
 *
 */

#include "fsdefs.h"

LOCAL	fs_descmgr_t	con_descmgr;		/* Connection manager */

/*
 *  Initialize connection manager
 */
EXPORT	INT	fs_con_init(INT max)
{
	return fs_descmgr_init(&con_descmgr, max,
					sizeof(fs_cond_t), DESC_CON);
}

/*
 *  Finalize connection manager
 */
EXPORT	INT	fs_con_fini(void)
{
	return fs_descmgr_fini(&con_descmgr);
}

/*
 *  Release connection descriptor obtained by fs_con_find() or fs_con_get()
 */
EXPORT	void	fs_con_release(fs_cond_t *cond)
{
	fs_descmgr_release(&con_descmgr, &cond->c_desc);
}

/*
 *  Get connection descriptor by connection id
 */
EXPORT	INT	fs_con_get(UINT ident, fs_cond_t **cond)
{
	INT		sts;
	fs_cond_t	*p;

	fs_lock_lock(LOCKNUM_ROOT);

	p = (fs_cond_t *)fs_descmgr_get(&con_descmgr, ident);
	if (p == NULL) {
		sts = EX_BADF;
	} else if ((p->c_desc.d_flags & CON_BUSY) != 0) {
		fs_descmgr_release(&con_descmgr, &p->c_desc);
		sts = EX_BUSY;
		p = NULL;
	} else {
		sts = 0;
	}

	fs_lock_unlock(LOCKNUM_ROOT);
	*cond = p;
	return sts;
}

/*
 *  Search connection descriptor -  should be called with lock
 */
LOCAL	fs_cond_t	*fs_con_search(const B *connm)
{
	fs_cond_t	*cond;

	for (cond = (fs_cond_t *)fs_root.r_list.next;
		(QUEUE*)cond != &fs_root.r_list;
			cond = (fs_cond_t *)cond->c_desc.d_list.next) {
		if (strcmp(cond->c_coninf.connm, connm) == 0) {
			return cond;	/* Found */
		}
	}
	return NULL;	/* Not found */
}

/*
 *  Find Connection by the connection name
 */
EXPORT	INT	fs_con_find(const B *connm, fs_cond_t **cond)
{
	INT		sts;
	fs_cond_t	*p;

	fs_lock_lock(LOCKNUM_ROOT);

	p = fs_con_search(connm);
	if (p == NULL) {
		sts = EX_NOENT;
	} else if ((p->c_desc.d_flags & CON_BUSY) != 0) {
		sts = EX_BUSY;
		p = NULL;
	} else {	/* Found, increment reference count to avoid delete */
		p->c_desc.d_refc++;
		sts = 0;
	}

	fs_lock_unlock(LOCKNUM_ROOT);
	*cond = p;
	return sts;
}

/*
 *  Regist connection descriptor by name
 */
EXPORT	INT	fs_con_regist(const B *connm, fs_cond_t **cond)
{
	INT		sts;
	fs_cond_t	*p;

	fs_lock_lock(LOCKNUM_ROOT);

	p = fs_con_search(connm);
	if (p != NULL) {
		sts = EX_EXIST;
	} else if ((p = (fs_cond_t *)
				fs_descmgr_alloc(&con_descmgr)) == NULL) {
		sts = EX_NOBUFS;
	} else {
		/* Append to "root" with BUSY */
		strcpy(p->c_coninf.connm, connm);
		p->c_desc.d_flags |= CON_BUSY;
		QueInsert(&p->c_desc.d_list, &fs_root.r_list);
		fs_root.r_count++;
		sts = 0;
	}

	fs_lock_unlock(LOCKNUM_ROOT);
	*cond = p;
	return sts;
}

/*
 *  End of registration of connection descriptor
 */
EXPORT	void	fs_con_end_regist(fs_cond_t *cond, INT err)
{
	fs_lock_lock(LOCKNUM_ROOT);

	if (err != 0) {	/* Error : Cancel registration */
		/* Remove from "root" & release connection descriptor */
		QueRemove(&cond->c_desc.d_list);
		fs_root.r_count--;
		fs_descmgr_free(&con_descmgr, &cond->c_desc);
	} else {		/* OK : Complete registration */
		(void)fs_descmgr_validate(&con_descmgr, &cond->c_desc);
		cond->c_desc.d_flags &= ~CON_BUSY;	/* Reset BUSY */
	}

	fs_lock_unlock(LOCKNUM_ROOT);
}

/*
 *  Unregist connection descriptor by name
 */
EXPORT	INT	fs_con_unregist(const B *connm, fs_cond_t **cond, INT forced)
{
	INT		sts;
	fs_cond_t	*p;

	fs_lock_lock(LOCKNUM_ROOT);

	p = fs_con_search(connm);
	if (p == NULL || (p->c_desc.d_flags & CON_BUSY) != 0) {
		sts = EX_NOENT;
	} else if (forced == 0 && p->c_desc.d_refc != 0) {
		sts = EX_BUSY;
	} else {
		p->c_desc.d_flags |= CON_BUSY;
		sts = 0;
	}

	fs_lock_unlock(LOCKNUM_ROOT);
	*cond = p;
	return sts;
}

/*
 *  End of unregistration of connection descriptor
 */
EXPORT	void	fs_con_end_unregist(fs_cond_t *cond, INT err)
{
	fs_lock_lock(LOCKNUM_ROOT);

	if (err != 0) {	/* Error : Cancel unregistration */
		cond->c_desc.d_flags &= ~CON_BUSY;
	} else {		/* OK : Complete unregistration */
		/* Remove from "root" */
		QueRemove(&cond->c_desc.d_list);
		fs_root.r_count--;
		/* Invalidate & free connection descriptor */
		fs_descmgr_invalidate(&con_descmgr, &cond->c_desc);
		fs_descmgr_free(&con_descmgr, &cond->c_desc);
	}

	fs_lock_unlock(LOCKNUM_ROOT);
}

