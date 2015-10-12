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
 *	@(#)fs_root.c
 *
 */

#include "fsdefs.h"

EXPORT	fs_regis_t	fs_root;		/* Root directory	*/

#define	FS_ROOT_RID	((UINT)&fs_root)

/*
 *  Definition of FIMP for Root File System
 */
LOCAL	ER	root_service(fimp_t *req);
LOCAL	ER	root_registfn(fimpinf_t *fimpinf, void *exinf);
LOCAL	ER	root_unregistfn(fimpinf_t *fimpinf);
LOCAL	ER	root_attachfn(coninf_t *coninf, void *exinf);
LOCAL	ER	root_detachfn(coninf_t *coninf);

LOCAL	const	fs_fimp_t	rootfs_entry = {
	root_service,		/* reqfn       */
	root_registfn,		/* registfn    */
	root_unregistfn,	/* unregistfn  */
	root_attachfn,		/* attachfn    */
	root_detachfn,		/* detachfn    */
	0,			/* startupfn   */
	0,			/* cleanupfn   */
	0,			/* breakfn     */
	FIMP_FLAG_PRIVILEGE,	/* flags       */
	0,			/* priority    */
};

/*
 *  FIMP for Root File System
 */
LOCAL	const	B	root_name[] = "rootfs";	/* FIMP name		*/
LOCAL	const	B	root_con_name[] = "/";	/* Connection name	*/

/*
 *  Root FIMP initialization
 */
EXPORT	INT	fs_rootinit(fs_env_t *env)
{
	INT	sts;

	QueInit(&fs_root.r_list);
	fs_root.r_count = 0;

	/* Regist "root" FIMP */
	sts = xfs_regist(env, root_name, &rootfs_entry, NULL, NULL);
	if (sts == 0) {
		/* Attach "root" */
		sts = xfs_attach(env, NULL, root_con_name, root_name, 0, NULL);
	}
	return sts;
}

/*
 *  Root FIMP finalize
 */
EXPORT	INT	fs_rootfini(fs_env_t *env)
{
	INT	sts;

	/* Detach "root" */
	sts = xfs_detach(env, root_con_name, 1);
	if (sts == 0) {
		/* Unregist "root" */
		sts = xfs_unregist(env, root_name);
	}
	return sts;
}

/*----------------------------------------------------------------------------
	FIMP of "Root" File System
-----------------------------------------------------------------------------*/

/*
 *  FIMP_OPEN: Open file/directory service
 */
LOCAL	ER	root_open(struct fimp_open *req)
{
	if (strcmp(req->path, root_con_name) != 0)	return EX_NOENT;
	if ((req->oflags & O_ACCMODE) != O_RDONLY)	return EX_ROFS;

	*req->fid = FS_ROOT_RID;
	return E_OK;
}

/*
 *  FIMP_CLOSE: Close file/directory service
 */
LOCAL	ER	root_close(struct fimp_close *req)
{
	return (req->fid != FS_ROOT_RID) ? EX_BADF : E_OK;
}

/*
 *  FIMP_STAT64_US:  Get file/directory status by name service
 *  FIMP_FSTAT64_US: Get file/directory by file number service
 */
LOCAL	ER	set_stat64_us(struct stat64_us *stat)
{
	memset(stat, 0, sizeof(struct stat64_us));
	stat->st_size = (fs_root.r_count - 1) * sizeof(struct dirent);
	stat->st_mode = S_IFDIR | S_IRUSR | S_IXUSR;
	return E_OK;
}
LOCAL	ER	root_stat64_us(struct fimp_stat64_us *req)
{
	return (strcmp(req->path, root_con_name) != 0) ?
					EX_NOENT : set_stat64_us(req->buf);
}
LOCAL	ER	root_fstat64_us(struct fimp_fstat64_us *req)
{
	return (req->fid != FS_ROOT_RID) ? EX_BADF : set_stat64_us(req->buf);
}

/*
 *  FIMP_CHDIR: Change directory by name service
 */
LOCAL	ER	root_chdir(struct fimp_chdir *req)
{
	return (strcmp(req->path, root_con_name) != 0) ? EX_NOTDIR : E_OK;
}

/*
 *  FIMP_HCHDIR: Change directory by file number service
 */
LOCAL	ER	root_fchdir(struct fimp_fchdir *req)
{
	if (req->fid != FS_ROOT_RID)		return EX_BADF;
	if (req->len < sizeof(root_con_name))	return EX_RANGE;

	strncpy(req->buf, root_con_name, sizeof(root_con_name));
	return E_OK;
}

/*
 *  FIMP_GETDENTS: Get directory entries service
 */
LOCAL	ER	root_getdents(struct fimp_getdents *req)
{
	struct dirent	*dir;
	fs_cond_t	*con;
	INT		len, nlen, reclen;
	off64_t		off;
	ER		er;

	if (req->fid != FS_ROOT_RID) return EX_BADF;

	dir = req->buf;
	er = E_OK;

	fs_lock_lock(LOCKNUM_ROOT);

	/* skip 1st entry "root" */
	for (len = 0, off = 0, con = (fs_cond_t *)fs_root.r_list.next->next;
			(QUEUE*)con != &fs_root.r_list;
				con = (fs_cond_t *)con->c_desc.d_list.next,
				off += sizeof(struct dirent)) {
		if (off < *(req->off)) continue;	/* Skip */

		/* Calcurate actual entry length and check buffer space */
		nlen = strlen(con->c_coninf.connm);
		if (nlen > NAME_MAX) nlen = NAME_MAX;

		reclen = RECLEN_DIRENT(nlen);
		if (len + reclen > *(req->len)) {
			if (len == 0) er = EX_INVAL;
			break; /* No more space */
		}

		/* Set one entry to dir buffer */
		strncpy(dir->d_name, con->c_coninf.connm, nlen);
		dir->d_name[nlen] = '\0';
		dir->d_ino = 0;
		dir->d_reclen = reclen;
		len += reclen;

		if (*(req->len) == sizeof(struct dirent)) {
			/* Special case, get only one entry */
			off += sizeof(struct dirent);
			break;
		}
		/* Next buffer pointer */
		dir = (struct dirent *)((UB*)dir + reclen);
	}
	fs_lock_unlock(LOCKNUM_ROOT);

	*req->retoff = off;
	*req->len = len;
	return er;
}

/*
 *  Root FIMP service function
 */
LOCAL	ER	root_service(fimp_t *req)
{
	ER	er;

	switch (req->com.r_code) {
	case FIMP_OPEN:
		er = root_open(&req->r_open);
		break;
	case FIMP_CLOSE:
		er = root_close(&req->r_close);
		break;
	case FIMP_STAT64_US:
		er = root_stat64_us(&req->r_stat64_us);
		break;
	case FIMP_CHDIR:
		er = root_chdir(&req->r_chdir);
		break;
	case FIMP_FSTAT64_US:
		er = root_fstat64_us(&req->r_fstat64_us);
		break;
	case FIMP_FCHDIR:
		er = root_fchdir(&req->r_fchdir);
		break;
	case FIMP_GETDENTS:
		er = root_getdents(&req->r_getdents);
		break;
	case FIMP_RMDIR:
		er = EX_BUSY;
		break;
	default:
		er = EX_NOTSUP;
	}
	return er;
}

/*
 *  Root FIMP initialize function
 */
LOCAL	ER	root_registfn(fimpinf_t *fimpinf, void *exinf)
{
	return E_OK;
}

/*
 *  Root FIMP finalize function
 */
LOCAL	ER	root_unregistfn(fimpinf_t *fimpinf)
{
	return E_OK;
}

/*
 *  Root FIMP attach function
 */
LOCAL	ER	root_attachfn(coninf_t *coninf, void *exinf)
{
	return (strcmp(coninf->connm, root_con_name) != 0) ? EX_PERM : 0;
}

/*
 *  Root FIMP detach function
 */
LOCAL	ER	root_detachfn(coninf_t *coninf)
{
	return EX_PERM;
}


