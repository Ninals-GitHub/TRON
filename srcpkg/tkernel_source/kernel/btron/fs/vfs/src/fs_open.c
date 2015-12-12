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
 *    Modified by Nina Petipa at 2015/11/29
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
 *	@(#)fs_open.c
 *
 */

#include <tk/kernel.h>
#include <bk/uapi/errno.h>
#include "fsdefs.h"

/*
 *  Validate file open flag
 */
LOCAL	INT	fs_validate_open_flags(INT flags, mode_t mode)
{
	if (	((flags & O_ACCMODE) != O_RDONLY &&
		 (flags & O_ACCMODE) != O_WRONLY &&
		 (flags & O_ACCMODE) != O_RDWR)
		|| ((flags & (O_CREAT | O_EXCL)) == O_EXCL)
		|| ((flags & ~(O_ACCMODE | O_APPEND | O_CREAT | O_DSYNC |
			O_EXCL | O_NOCTTY | O_NONBLOCK | O_RSYNC |
			O_SYNC | O_TRUNC | O_DIRECTORY)) != 0)
		|| (((flags & O_CREAT) != 0)
			&& ((mode & (mode_t) ~ (S_ISUID | S_ISGID | S_ISVTX
			| S_IRWXU | S_IRWXG | S_IRWXO)) != (mode_t) 0))) {
		return 0;	/* Invalid */
	}
	return 1;		/* Valid */
}

/*
 *  File open function
 */
LOCAL	INT	open_file(fs_env_t *env, fs_path_t *path, INT flags,
						mode_t mode, INT fno)
{
	fs_file_t	*file;
	INT		sts;

	/* Allocate file descriptor */
	file = fs_file_alloc();
	if (file == NULL) {
		sts = EX_NFILE;
		goto exit0;
	}

	/* Form file mode */
	if ((flags & O_CREAT) != 0) {
		mode &= ~env->t_ctx->x_umask;
	} else {
		mode = 0;
	}

	/* Call FIMP OPEN service */
	env->t_cmd.r_open.path = path->p_name;
	env->t_cmd.r_open.oflags = flags;
	env->t_cmd.r_open.mode = mode;
	env->t_cmd.r_open.fid = &env->t_misc.t_fid;
	sts = fs_callfimp(env, path->p_con, FIMP_OPEN);
	if (sts != 0) {
		fs_file_free(file);
		goto exit0;
	}

	/* Setup file descriptor and prepare open request */
	fs_file_setup(file, env->t_misc.t_fid, path->p_con->c_desc.d_ident,
			(flags & ~(O_CREAT | O_EXCL | O_TRUNC | O_NOCTTY)));

	/* Set file ident and record it */
	fs_ctx_setfile(env->t_ctx, fno, file);

	/* Avoid releasing it by fs_path_release() */
	path->p_con = NULL;
exit0:
	return sts;
}

/*
 *  fs_open() - Open file
 */
EXPORT	INT	xfs_open(fs_env_t *env, const B *name, INT flags, mode_t mode)
{
	fs_path_t	path;
	INT		fno, sts;

	fno = -1;

	/* Check flags and access permissions validity */
	if (fs_validate_open_flags(flags, mode) == 0) {
		sts = EX_INVAL;
		goto exit0;
	}

	/* Parse the pathname & get path structure */
	sts = fs_path_parse(env, name, &path, FALSE);
	if (sts != 0) goto exit0;

	if (strchr(&path.p_name[1], '/') == NULL) {
		/* Check write to filesystem root */
		if ((flags & O_ACCMODE) != O_RDONLY) {
			sts = EX_ISDIR;
			goto exit1;
		}
		/* Check existing filesystem root */
		if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
			sts = EX_EXIST;
			goto exit1;
		}
		/* Check creating or truncating filesystem root */
		if ((flags & (O_CREAT | O_TRUNC)) != 0) {
			sts = EX_ACCES;
			goto exit1;
		}
	} else {
		/* Check file system writeability */
		if ((path.p_con->c_coninf.dflags & DEV_FLAG_READONLY) != 0 &&
			( (flags & (O_CREAT | O_TRUNC)) != 0 ||
				(flags & O_ACCMODE) != O_RDONLY ) ) {
			sts = EX_ROFS;
			goto exit1;
		}
	}
	/* Reserve file number */
	fno = fs_ctx_allocfile(env->t_ctx, 0);
	if (fno < 0) {
		sts = EX_MFILE;
		goto exit1;
	}
	/* open file */
	sts = open_file(env, &path, flags, mode, fno);
	if (sts != 0) {
		/* Release allocated file number */
		fs_ctx_releasefile(env->t_ctx, fno);
	}
exit1:
	/* Release path structure */
	fs_path_release(env, &path);
	if (sts == 0) return fno;
exit0:
	env->t_errno = sts;
	return -1;
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:open
 Input		:const char *pathname
 		 < path name to open >
 		 int flags
 		 < open flags >
 		 mode_t mode
 		 < open mode >
 Output		:void
 Return		:int
 		 < open file descriptor id >
 Description	:open a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int open(const char *pathname, int flags, mode_t mode)
{
	printf("open\(pathname=%s", pathname);
	printf(", flags=0x%08X", flags);
	printf(", mode=0x%04X\)\n", mode);
	
	return 1;
}

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
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
