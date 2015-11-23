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
 *	@(#)fimp_console.c
 *
 */

#include "fimp_console.h"
#include <sys/consio.h>
#include <string.h>

/*
 *  FIMP entry for "console" file system
 */
LOCAL	ER	console_service(fimp_t *req);
LOCAL	ER	console_registfn(fimpinf_t *fimpinf, void *info);
LOCAL	ER	console_unregistfn(fimpinf_t *fimpinf);
LOCAL	ER	console_attachfn(coninf_t *coninf, void *info);
LOCAL	ER	console_detachfn(coninf_t *coninf);

EXPORT	const	fs_fimp_t	fimp_consolefs_entry = {
	console_service,	/* reqfn	*/
	console_registfn,	/* registfn	*/
	console_unregistfn,	/* unregistfn	*/
	console_attachfn,	/* attachfn	*/
	console_detachfn,	/* detachfn	*/
	0,			/* startupfn	*/
	0,			/* cleanupfn	*/
	0,			/* breakfn	*/
	FIMP_FLAG_PRIVILEGE,	/* flags	*/
	0,			/* priority	*/
};

/* Console file descriptors */
#define ROOT_FID		(0)
#define STDIN_FID		(1)
#define STDOUT_FID		(2)
#define STDERR_FID		(3)
#define	STDIN_FN		"stdin"
#define	STDOUT_FN		"stdout"
#define	STDERR_FN		"stderr"

/* Macros for flag check */
#define CHK_AND(flags, value)		(((flags) & (value)) == (value))
#define CHK_OR(flags, value)		(((flags) & (value)) != 0)

/* Console I/O driver entry */
IMPORT	ER	ConsoleIO(INT ac, UB *av[]);

/*
 *  Get console file id
 */
LOCAL	INT	get_con_fid(const coninf_t *coninf, const B *name)
{
	INT	len;

	len = strlen(coninf->connm);
	if (*name++ == '/' && strncmp(coninf->connm, name, len) == 0) {
		/* connm is matched */
		if (strlen(name) == len) return ROOT_FID;
		if (name[len] == '/') {
			name += len + 1;
			if (strcmp(name, STDIN_FN) == 0)  return STDIN_FID;
			if (strcmp(name, STDOUT_FN) == 0) return STDOUT_FID;
			if (strcmp(name, STDERR_FN) == 0) return STDERR_FID;
		}
	}
	return -1;
}

/*
 *  FIMP_OPEN service
 */
LOCAL	ER	console_open(struct fimp_open *req)
{
	ER	er;
	INT	fid;

	er = E_OK;
	switch (fid = get_con_fid(req->coninf, req->path)) {
	case ROOT_FID:
		if (CHK_OR(req->oflags, O_CREAT))		er = EX_ACCES;
		else if (CHK_OR(req->oflags, O_WRONLY))	er = EX_ISDIR;
		break;
	case STDIN_FID:
		if (CHK_OR(req->oflags, O_CREAT | O_WRONLY))	er = EX_ACCES;
		else if (CHK_OR(req->oflags, O_DIRECTORY))	er = EX_NOTDIR;
		break;
	case STDOUT_FID:
	case STDERR_FID:
		if (CHK_OR(req->oflags, O_CREAT | O_RDONLY))	er = EX_ACCES;
		else if (CHK_OR(req->oflags, O_DIRECTORY))	er = EX_NOTDIR;
		break;
	default:
		if (CHK_OR(req->oflags, O_CREAT))		er = EX_ACCES;
		else						er = EX_NOENT;
	}

	if (er == E_OK) {
		if (CHK_AND(req->oflags, O_CREAT | O_EXCL)) {
			er = EX_EXIST;
		} else {
			*req->fid = fid;
		}
	}
	return er;
}

/*
 *  FIMP_CLOSE service
 */
LOCAL	ER	console_close(struct fimp_close *req)
{
	switch (req->fid) {
	case ROOT_FID:
	case STDIN_FID:
	case STDOUT_FID:
	case STDERR_FID:
		break;
	default:
		return EX_BADF;
	}
	return E_OK;
}

/*
 *  FIMP_READ64 service
 */
LOCAL	ER	console_read64(struct fimp_read64 *req)
{
	ER	er;
	INT	len;

	er = EX_BADF;
	if (req->fid == STDIN_FID) {
		/* Input from "console" */
		len = console_in((UW)req->coninf->consd, req->buf, *req->len);
		if (len >= 0) {
			*req->len = len;
			er = E_OK;
		} else {
			*req->len = 0;
			er = EX_INTR;
		}
		*req->retoff = 0;
	}
	return er;
}

/*
 *  FIMP_WRITE64 service
 */
LOCAL	ER	console_write64(struct fimp_write64 *req)
{
	ER	er;
	W	len;

	er = EX_BADF;
	if (req->fid == STDOUT_FID || req->fid == STDERR_FID) {
		/* Output to "console" */
		len = console_out((UW)req->coninf->consd, req->buf, *req->len);
		*req->len = len;
		*req->retoff = 0;
		er = E_OK;
	}
	return er;
}

/*
 *  Get stat64_us
 */
LOCAL	ER	set_stat64_us(struct stat64_us *stat, W fid)
{
	INT	size;
	mode_t	mode;

	switch(fid) {
	case ROOT_FID:
		size = 3;
		mode = S_IFDIR | S_IRUSR | S_IXUSR;
		break;
	case STDIN_FID:
		size = 0;
		mode = S_IFCHR | S_IRUSR;
		break;
	case STDOUT_FID:
	case STDERR_FID:
		size = 0;
		mode = S_IFCHR | S_IWUSR;
		break;
	default:
		return EX_BADF;
	}
	memset(stat, 0, sizeof(struct stat64_us));
	stat->st_ino = fid;
	stat->st_size = size * sizeof(struct dirent);
	stat->st_mode = mode;
	return E_OK;
}

/*
 *  FIMP_STAT64_US service
 */
LOCAL	ER	console_stat64_us(struct fimp_stat64_us *req)
{
	return set_stat64_us(req->buf, get_con_fid(req->coninf, req->path));
}

/*
 *  FIMP_FSTAT64_US service
 */
LOCAL	ER	console_fstat64_us(struct fimp_fstat64_us *req)
{
	return set_stat64_us(req->buf, req->fid);
}

/*
 *  FIMP_CHDIR service
 */
LOCAL	ER	console_chdir(struct fimp_chdir *req)
{
	return (get_con_fid(req->coninf, req->path) == ROOT_FID) ?
							E_OK : EX_NOENT;
}

/*
 *  FIMP_FCHDIR service
 */
LOCAL	ER	console_fchdir(struct fimp_fchdir *req)
{
	if (req->fid != ROOT_FID) return EX_BADF;

	req->buf[0] = '/';
	strcpy(&req->buf[1], req->coninf->connm);
	return E_OK;
}

/*
 *  FIMP_GETDENTS service
 */
LOCAL	ER	console_getdents(struct fimp_getdents *req)
{
	ino_t		ino;
	const	B	*fname;
	struct dirent	*dir;
	off64_t		off;
	INT		rlen, len, nlen, reclen;
	ER		er;

	if (req->fid != ROOT_FID) return EX_BADF;

	off = *req->off;
	if ((off % sizeof(struct dirent)) != 0) return EX_NOENT;

	er = E_OK;
	rlen = *req->len;
	for (dir = req->buf, len = 0; len + RECLEN_DIRENT(1) <= rlen; ) {
		switch (off / sizeof(struct dirent)) {
		case 0:
			ino = STDIN_FID;
			fname = STDIN_FN;
			break;
		case 1:
			ino = STDOUT_FID;
			fname = STDOUT_FN;
			break;
		case 2:
			ino = STDERR_FID;
			fname = STDERR_FN;
			break;
		default:
			fname = NULL;
			break;
		}
		if (fname == NULL) break;		/* End of entry */

		/* Calcurate actual entry length and check buffer space */
		if ((nlen = strlen(fname)) > NAME_MAX) nlen = NAME_MAX;
		reclen = RECLEN_DIRENT(nlen);
		if (len + reclen > rlen) {
			if (len == 0) er = EX_INVAL;
			break;	/* No more space */
		}

		/* Set one entry to dir buffer */
		strncpy(dir->d_name, fname, nlen);
		dir->d_name[nlen] = '\0';
		dir->d_ino = ino;
		dir->d_reclen = reclen;
		len += reclen;

		/* Setup for next entry */
		off += sizeof(struct dirent);
		dir = (struct dirent *)((UB*)dir + reclen);

		/* Special case, get only one entry */
		if (rlen == sizeof(struct dirent)) break;
	}
	*req->retoff = off;
	*req->len = len;
	return er;
}

/*
 *  Console FIMP service function
 */
LOCAL	ER	console_service(fimp_t *req)
{
	ER	er;

	switch (req->com.r_code) {
	case FIMP_OPEN:
		er = console_open(&req->r_open);
		break;
	case FIMP_CLOSE:
		er = console_close(&req->r_close);
		break;
	case FIMP_READ64:
		er = console_read64(&req->r_read64);
		break;
	case FIMP_WRITE64:
		er = console_write64(&req->r_write64);
		break;
	case FIMP_STAT64_US:
		er = console_stat64_us(&req->r_stat64_us);
		break;
	case FIMP_FSTAT64_US:
		er = console_fstat64_us(&req->r_fstat64_us);
		break;
	case FIMP_GETDENTS:
		er = console_getdents(&req->r_getdents);
		break;
	case FIMP_CHDIR:
		er = console_chdir(&req->r_chdir);
		break;
	case FIMP_FCHDIR:
		er = console_fchdir(&req->r_fchdir);
		break;
	case FIMP_FSYNC:
	case FIMP_SYNC:
		er = E_OK;
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
 *  Console FIMP initialize function
 */
LOCAL	ER	console_registfn(fimpinf_t *fimpinf, void *info)
{
	/* Nothing to do */
	return E_OK;
}

/*
 *  Console FIMP finalize function
 */
LOCAL	ER	console_unregistfn(fimpinf_t *fimpinf)
{
	/* Never unregister */
	return E_OK;
}

/*
 *  Console FIMP attach function
 */
LOCAL	ER	console_attachfn(coninf_t *coninf, void *info)
{
	ER	er;
	UW	port;

	/* Has the console driver initialized ? */
	er = console_conf(CS_GETPORT, &port);
	if (er < E_OK) {
		if (er == E_RSFN) {
			/* No, initialize console driver now */
#ifdef DRV_CONSOLE
			if (ConsoleIO(0, NULL) >= E_OK) {
				er = console_conf(CS_GETPORT, &port);
			}
#endif
		}
		if (er < E_OK) return EX_IO;
	}
	coninf->consd = (void *)port;
	return E_OK;
}

/*
 *  Console FIMP detach function
 */
LOCAL	ER	console_detachfn(coninf_t *coninf)
{
	/* Never detach */
	return E_OK;
}

