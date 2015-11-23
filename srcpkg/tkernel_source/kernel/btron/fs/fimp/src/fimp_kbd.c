/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */

#include <basic.h>
#include <tk/typedef.h>
#include <string.h>

#include <device/std_x86/kbd.h>

#include <debug/vdebug.h>

#include "fimp_kbd.h"
/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	fimp interface functions
----------------------------------------------------------------------------------
*/
LOCAL ER kbd_service(fimp_t *req);
LOCAL ER kbd_register(fimpinf_t *fimpinf, void *info);
LOCAL ER kbd_unregister(fimpinf_t *fimpinf);
LOCAL ER kbd_attach(coninf_t *coninf, void *info);
LOCAL ER kbd_detach(coninf_t *coninf);
LOCAL ER kbd_startup(coninf_t *coninf, ID resid);
LOCAL ER kbd_cleanup(coninf_t *coninf, ID resid);
/*
----------------------------------------------------------------------------------
	fimp service methods
----------------------------------------------------------------------------------
*/
LOCAL ER kbd_open(struct fimp_open *req);
LOCAL ER kbd_close(struct fimp_close *req);
LOCAL ER kbd_read64(struct fimp_read64 *req);
LOCAL ER kbd_write64(struct fimp_write64 *req);
LOCAL ER set_stat64_us(struct stat64_us *stat, W fid);
LOCAL ER kbd_stat64_us(struct fimp_stat64_us *req);
LOCAL ER kbd_fstat64_us(struct fimp_fstat64_us *req);
LOCAL ER kbd_chdir(struct fimp_chdir *req);
LOCAL ER kbd_fchdir(struct fimp_fchdir *req);
LOCAL ER kbd_getdents(struct fimp_getdents *req);


LOCAL INT get_kbd_fid(const coninf_t *coninf, const B *name);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/* Console file descriptors */
#define ROOT_FID		(0)
#define STDIN_FID		(1)
#define STDOUT_FID		(2)
#define STDERR_FID		(3)
#define	STDIN_FN		"stdin"
#define	STDOUT_FN		"stdout"
#define	STDERR_FN		"stderr"

#define CHK_AND(flags, value)		(((flags) & (value)) == (value))
#define CHK_OR(flags, value)		(((flags) & (value)) != 0)

/*
==================================================================================

	Management 

==================================================================================
*/
EXPORT const fs_fimp_t fimp_kbd_entry = {
	.reqfn		= kbd_service,
	.registfn	= kbd_register,
	.unregistfn	= kbd_unregister,
	.attachfn	= kbd_attach,
	.detachfn	= kbd_detach,
	.startupfn	= kbd_startup,
	.cleanupfn	= kbd_cleanup,
	.breakfn	= NULL,
	.flags		= FIMP_FLAG_PRIVILEGE,
	.priority	= 0,
};



/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
----------------------------------------------------------------------------------
	fimp interface functionss
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:kbd_service
 Input		:fimp_t *req
		 < request >
 Output		:void
 Return		:ER
		 < error code >
 Description	:kbd file operations
==================================================================================
*/
LOCAL ER kbd_service(fimp_t *req)
{
	ER	er;

	switch (req->com.r_code) {
	case FIMP_OPEN:
		er = kbd_open(&req->r_open);
		break;
	case FIMP_CLOSE:
		er = kbd_close(&req->r_close);
		break;
	case FIMP_READ64:
		er = kbd_read64(&req->r_read64);
		break;
	case FIMP_WRITE64:
		er = kbd_write64(&req->r_write64);
		break;
	case FIMP_STAT64_US:
		er = kbd_stat64_us(&req->r_stat64_us);
		break;
	case FIMP_FSTAT64_US:
		er = kbd_fstat64_us(&req->r_fstat64_us);
		break;
	case FIMP_GETDENTS:
		er = kbd_getdents(&req->r_getdents);
		break;
	case FIMP_CHDIR:
		er = kbd_chdir(&req->r_chdir);
		break;
	case FIMP_FCHDIR:
		er = kbd_fchdir(&req->r_fchdir);
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
==================================================================================
 Funtion	:kbd_register
 Input		:fimpinf_t *fimpinf
		 < fimp information >
		 void *info
		 < private data >
 Output		:void
 Return		:ER
		 < error code >
 Description	:fimp initialization function
==================================================================================
*/
LOCAL ER kbd_register(fimpinf_t *fimpinf, void *info)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:kbd_unregist
 Input		:fimpinf_t *fimpinf
		 < fimp information  >
 Output		:void
 Return		:ER
		 < error code >
 Description	:fimp finalization function
==================================================================================
*/
LOCAL ER kbd_unregister(fimpinf_t *fimpinf)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:kbd_attach
 Input		:coninf_t *coninf
		 < connection information >
		 void *info
		 < private data >
 Output		:void
 Return		:ER
		 < error code >
 Description	:attach kbd fs to the file system
==================================================================================
*/
LOCAL ER kbd_attach(coninf_t *coninf, void *info)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:kbd_detach
 Input		:coninf_t *coninf
		 < connection information >
 Output		:void
 Return		:ER
		 < error code >
 Description	:detach kbd fs from the file system
==================================================================================
*/
LOCAL ER kbd_detach(coninf_t *coninf)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:kbd_startup
 Input		:coninf_t *coninf
		 < connection inforamtion >
		 ID resid
		 < resource id >
 Output		:void
 Return		:ER
		 < error code >
 Description	:startup kbd fs
==================================================================================
*/
LOCAL ER kbd_startup(coninf_t *coninf, ID resid)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:kbd_cleanup
 Input		:coninf_t *coninf
		 < connection inforamtion >
		 ID resid
		 < resource id >
 Output		:void
 Return		:ER
		 < error code >
 Description	:cleanup kbd fs
==================================================================================
*/
LOCAL ER kbd_cleanup(coninf_t *coninf, ID resid)
{
	return(E_OK);
}

/*
----------------------------------------------------------------------------------
	fimp service methods
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:kbd_open
 Input		:const fimp_open *req
		 < open request >
 Output		:void
 Return		:ER
		 < error code >
 Description	:open method
==================================================================================
*/
LOCAL ER kbd_open(struct fimp_open *req)
{
	ER	er;
	INT	fid;

	er = E_OK;
	switch ((fid = get_kbd_fid(req->coninf, req->path))) {
	case ROOT_FID:
		if (CHK_OR(req->oflags, O_CREAT))		er = EX_ACCES;
		else if (CHK_OR(req->oflags, O_WRONLY))	er = EX_ISDIR;
		break;
	case STDIN_FID:
		if (CHK_OR(req->oflags, O_CREAT | O_WRONLY))	er = EX_ACCES;
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
==================================================================================
 Funtion	:kbd_close
 Input		:struct fimp_close *req
		 < close request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:kbd close method
==================================================================================
*/
LOCAL ER kbd_close(struct fimp_close *req)
{
	switch (req->fid) {
	case ROOT_FID:
	case STDIN_FID:
		break;
	default:
		return EX_BADF;
	}
	return E_OK;
}

/*
==================================================================================
 Funtion	:kbd_read64
 Input		:struct fimp_read64 *req
		 < read request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:kbd read method
==================================================================================
*/
LOCAL ER kbd_read64(struct fimp_read64 *req)
{
	ER	er;
	INT	len;

	er = EX_BADF;
	if (req->fid == STDIN_FID) {
		len = (INT)kbd_in(req->buf, *req->len);
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
==================================================================================
 Funtion	:kbd_write64
 Input		:struct fimp_write64 *req
		 < write request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:kbd write method
==================================================================================
*/
LOCAL ER kbd_write64(struct fimp_write64 *req)
{
	return EX_BADF;
}

/*
==================================================================================
 Funtion	:set_stat64_us
 Input		:struct stat64_us *stat
		 < file statistics information >
		 W fid
		 < file id >
 Output		:struct stat64_us *stat
		 < file statistics information >
 Return		:ERR
		 < error code >
 Description	:set stat method
==================================================================================
*/
LOCAL ER set_stat64_us(struct stat64_us *stat, W fid)
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
==================================================================================
 Funtion	:kbd_stat64_us
 Input		:struct fimp_stat64_us *req
		 < stat request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:kbd stat method
==================================================================================
*/
LOCAL ER kbd_stat64_us(struct fimp_stat64_us *req)
{
	return set_stat64_us(req->buf, get_kbd_fid(req->coninf, req->path));
}

/*
==================================================================================
 Funtion	:kbd_fstat64_us
 Input		:struct fimp_stat64_us *req
		 < fstat request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:kbd fstat method
==================================================================================
*/
LOCAL ER kbd_fstat64_us(struct fimp_fstat64_us *req)
{
	return set_stat64_us(req->buf, req->fid);
}

/*
==================================================================================
 Funtion	:kbd_chdir
 Input		:struct fimp_chdir *req
		 < chdir request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:kbd chdir method
==================================================================================
*/
LOCAL ER kbd_chdir(struct fimp_chdir *req)
{
	return (get_kbd_fid(req->coninf, req->path) == ROOT_FID) ?
							E_OK : EX_NOENT;
}

/*
==================================================================================
 Funtion	:kbd_fchdir
 Input		:struct fimp_fchdir *req
		 < getdents request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:kbd fchdir method
==================================================================================
*/
LOCAL ER kbd_fchdir(struct fimp_fchdir *req)
{
	if (req->fid != ROOT_FID) return EX_BADF;

	req->buf[0] = '/';
	strcpy(&req->buf[1], req->coninf->connm);
	return E_OK;
}

/*
==================================================================================
 Funtion	:kbd_getdents
 Input		:struct fimp_getdents *req
		 < getdents request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:kbd getdents method
==================================================================================
*/
LOCAL ER kbd_getdents(struct fimp_getdents *req)
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
==================================================================================
 Funtion	:get_kbd_fid
 Input		:const coninf_t *coninf
		 < connection information >
		 const B *name
		 < file name >
 Output		:void
 Return		:INT
		 < file id >
 Description	:get file id of kbd
==================================================================================
*/
LOCAL INT get_kbd_fid(const coninf_t *coninf, const B *name)
{
	INT len = strlen(coninf->connm);
	if (*name++ == '/' && strncmp(coninf->connm, name, len) == 0) {
		if (strlen(name) == len) {
			return(ROOT_FID);
		}
		if (name[len] == '/') {
			name += len + 1;
			if (strcmp(name, STDIN_FN) == 0) {
				return(STDIN_FID);
			}
			if (strcmp(name, STDERR_FN) == 0) {
				return(STDERR_FID);
			}
		}
	}
	
	return(-1);
}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
