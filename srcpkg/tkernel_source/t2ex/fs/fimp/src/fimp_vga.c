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

#include <device/std_x86/vga.h>

#include <debug/vdebug.h>

#include "fimp_vga.h"
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
LOCAL ER vga_service(fimp_t *req);
LOCAL ER vga_register(fimpinf_t *fimpinf, void *info);
LOCAL ER vga_unregister(fimpinf_t *fimpinf);
LOCAL ER vga_attach(coninf_t *coninf, void *info);
LOCAL ER vga_detach(coninf_t *coninf);
LOCAL ER vga_startup(coninf_t *coninf, ID resid);
LOCAL ER vga_cleanup(coninf_t *coninf, ID resid);
/*
----------------------------------------------------------------------------------
	fimp service methods
----------------------------------------------------------------------------------
*/
LOCAL ER vga_open(struct fimp_open *req);
LOCAL ER vga_close(struct fimp_close *req);
LOCAL ER vga_read64(struct fimp_read64 *req);
LOCAL ER vga_write64(struct fimp_write64 *req);
LOCAL ER set_stat64_us(struct stat64_us *stat, W fid);
LOCAL ER vga_stat64_us(struct fimp_stat64_us *req);
LOCAL ER vga_fstat64_us(struct fimp_fstat64_us *req);
LOCAL ER vga_chdir(struct fimp_chdir *req);
LOCAL ER vga_fchdir(struct fimp_fchdir *req);
LOCAL ER vga_getdents(struct fimp_getdents *req);


LOCAL INT get_vga_fid(const coninf_t *coninf, const B *name);

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
EXPORT const fs_fimp_t fimp_vga_entry = {
	.reqfn		= vga_service,
	.registfn	= vga_register,
	.unregistfn	= vga_unregister,
	.attachfn	= vga_attach,
	.detachfn	= vga_detach,
	.startupfn	= vga_startup,
	.cleanupfn	= vga_cleanup,
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
 Funtion	:vga_service
 Input		:fimp_t *req
		 < request >
 Output		:void
 Return		:ER
		 < error code >
 Description	:vga file operations
==================================================================================
*/
LOCAL ER vga_service(fimp_t *req)
{
	ER	er;

	switch (req->com.r_code) {
	case FIMP_OPEN:
		er = vga_open(&req->r_open);
		break;
	case FIMP_CLOSE:
		er = vga_close(&req->r_close);
		break;
	case FIMP_READ64:
		er = vga_read64(&req->r_read64);
		break;
	case FIMP_WRITE64:
		er = vga_write64(&req->r_write64);
		break;
	case FIMP_STAT64_US:
		er = vga_stat64_us(&req->r_stat64_us);
		break;
	case FIMP_FSTAT64_US:
		er = vga_fstat64_us(&req->r_fstat64_us);
		break;
	case FIMP_GETDENTS:
		er = vga_getdents(&req->r_getdents);
		break;
	case FIMP_CHDIR:
		er = vga_chdir(&req->r_chdir);
		break;
	case FIMP_FCHDIR:
		er = vga_fchdir(&req->r_fchdir);
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
 Funtion	:vga_register
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
LOCAL ER vga_register(fimpinf_t *fimpinf, void *info)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:vga_unregist
 Input		:fimpinf_t *fimpinf
		 < fimp information  >
 Output		:void
 Return		:ER
		 < error code >
 Description	:fimp finalization function
==================================================================================
*/
LOCAL ER vga_unregister(fimpinf_t *fimpinf)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:vga_attach
 Input		:coninf_t *coninf
		 < connection information >
		 void *info
		 < private data >
 Output		:void
 Return		:ER
		 < error code >
 Description	:attach vga fs to the file system
==================================================================================
*/
LOCAL ER vga_attach(coninf_t *coninf, void *info)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:vga_detach
 Input		:coninf_t *coninf
		 < connection information >
 Output		:void
 Return		:ER
		 < error code >
 Description	:detach vga fs from the file system
==================================================================================
*/
LOCAL ER vga_detach(coninf_t *coninf)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:vga_startup
 Input		:coninf_t *coninf
		 < connection inforamtion >
		 ID resid
		 < resource id >
 Output		:void
 Return		:ER
		 < error code >
 Description	:startup vga fs
==================================================================================
*/
LOCAL ER vga_startup(coninf_t *coninf, ID resid)
{
	return(E_OK);
}

/*
==================================================================================
 Funtion	:vga_cleanup
 Input		:coninf_t *coninf
		 < connection inforamtion >
		 ID resid
		 < resource id >
 Output		:void
 Return		:ER
		 < error code >
 Description	:cleanup vga fs
==================================================================================
*/
LOCAL ER vga_cleanup(coninf_t *coninf, ID resid)
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
 Funtion	:vga_open
 Input		:const fimp_open *req
		 < open request >
 Output		:void
 Return		:ER
		 < error code >
 Description	:open method
==================================================================================
*/
LOCAL ER vga_open(struct fimp_open *req)
{
	ER	er;
	INT	fid;

	er = E_OK;
	switch (fid = get_vga_fid(req->coninf, req->path)) {
	case ROOT_FID:
		if (CHK_OR(req->oflags, O_CREAT))		er = EX_ACCES;
		else if (CHK_OR(req->oflags, O_WRONLY))	er = EX_ISDIR;
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
==================================================================================
 Funtion	:vga_close
 Input		:struct fimp_close *req
		 < close request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:vga close method
==================================================================================
*/
LOCAL ER vga_close(struct fimp_close *req)
{
	switch (req->fid) {
	case ROOT_FID:
	case STDOUT_FID:
	case STDERR_FID:
		break;
	default:
		return EX_BADF;
	}
	return E_OK;
}

/*
==================================================================================
 Funtion	:vga_read64
 Input		:struct fimp_read64 *req
		 < read request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:vga read method
==================================================================================
*/
LOCAL ER vga_read64(struct fimp_read64 *req)
{
	return EX_BADF;
}


/*
==================================================================================
 Funtion	:vga_write64
 Input		:struct fimp_write64 *req
		 < write request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:vga write method
==================================================================================
*/
LOCAL ER vga_write64(struct fimp_write64 *req)
{
	ER	er;
	W	len;

	er = EX_BADF;
	if (req->fid == STDOUT_FID || req->fid == STDERR_FID) {
		len = vga_out(req->buf, *req->len);
		*req->len = len;
		*req->retoff = 0;
		er = E_OK;
	}
	return er;
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
==================================================================================
 Funtion	:vga_stat64_us
 Input		:struct fimp_stat64_us *req
		 < stat request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:vga stat method
==================================================================================
*/
LOCAL ER vga_stat64_us(struct fimp_stat64_us *req)
{
	return set_stat64_us(req->buf, get_vga_fid(req->coninf, req->path));
}

/*
==================================================================================
 Funtion	:vga_fstat64_us
 Input		:struct fimp_stat64_us *req
		 < fstat request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:vga fstat method
==================================================================================
*/
LOCAL ER vga_fstat64_us(struct fimp_fstat64_us *req)
{
	return set_stat64_us(req->buf, req->fid);
}

/*
==================================================================================
 Funtion	:vga_chdir
 Input		:struct fimp_chdir *req
		 < chdir request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:vga chdir method
==================================================================================
*/
LOCAL ER vga_chdir(struct fimp_chdir *req)
{
	return (get_vga_fid(req->coninf, req->path) == ROOT_FID) ?
							E_OK : EX_NOENT;
}

/*
==================================================================================
 Funtion	:vga_fchdir
 Input		:struct fimp_fchdir *req
		 < getdents request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:vga fchdir method
==================================================================================
*/
LOCAL ER vga_fchdir(struct fimp_fchdir *req)
{
	if (req->fid != ROOT_FID) return EX_BADF;

	req->buf[0] = '/';
	strcpy(&req->buf[1], req->coninf->connm);
	return E_OK;
}

/*
==================================================================================
 Funtion	:vga_getdents
 Input		:struct fimp_getdents *req
		 < getdents request >
 Output		:void
 Return		:ERR
		 < error code >
 Description	:vga getdents method
==================================================================================
*/
LOCAL ER vga_getdents(struct fimp_getdents *req)
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
==================================================================================
 Funtion	:get_vga_fid
 Input		:const coninf_t *coninf
		 < connection information >
		 const B *name
		 < file name >
 Output		:void
 Return		:INT
		 < file id >
 Description	:get file id of vga
==================================================================================
*/
LOCAL INT get_vga_fid(const coninf_t *coninf, const B *name)
{
	INT len = strlen(coninf->connm);
	if (*name++ == '/' && strncmp(coninf->connm, name, len) == 0) {
		if (strlen(name) == len) {
			return(ROOT_FID);
		}
		if (name[len] == '/') {
			name += len + 1;
			if (strcmp(name, STDOUT_FN) == 0) {
				return(STDOUT_FID);
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
