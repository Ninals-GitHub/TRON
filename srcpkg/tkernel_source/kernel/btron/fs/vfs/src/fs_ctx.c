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
 *	@(#)fs_ctx.c
 *
 */

#include "fsdefs.h"

#define	TMP_FILEID	((UINT)(-1))

#define	PATH_BUF(p)	((p) + PATH_MAX)

/*
 *  Initialize file system context
 */
EXPORT	INT	fs_ctx_initialize(fs_ctx_t * const ctx, INT maxfile)
{
	B	*p;

	(void)memset(ctx, 0, FS_CTX_SIZE(maxfile));
	ctx->x_maxfile = maxfile;

	/* Set curdir and work pathname buffer */
	p = (B *)fs_malloc(PATH_MAX * 2);
	if (p == NULL) return EX_NOMEM;

	ctx->x_curdir = p;		/* Current directory buffer */
	p[0] = '/';			/* Default is "/" */
	p[1] = '\0';
	ctx->x_pathbuf = PATH_BUF(p);	/* Work pathname buffer */
	return 0;
}

/*
 *  Finalize file system context
 */
EXPORT	void	fs_ctx_finalize(fs_ctx_t * const ctx, fs_env_t *env)
{
	INT		fn;
	fs_file_t	*file;

	/* Close all opened files */
	for (fn = 0; fn < ctx->x_maxfile; fn++) {
		if (ctx->x_fileids[fn] == 0) continue;
		/* Get file descriptor */
		if (fs_file_get(env, fn, &file, NULL) == 0) {
			/* Release file descriptor */
			(void)fs_file_release(env, file, NULL);
			/* Again, release file descriptor got by fs_open() */
			(void)fs_file_release(env, file, NULL);
		}
		ctx->x_fileids[fn] = 0;
	}
	env->t_errno = 0;	/* Ignore error */

	/* Release current directory buffer */
	if (ctx->x_curdir != NULL) {
		fs_free((void *)ctx->x_curdir);
		ctx->x_curdir = NULL;
	}
	ctx->x_pathbuf = NULL;
}

/*
 *  Get work buffer for pathname
 */
EXPORT	B	*fs_ctx_get_pathbuf(fs_ctx_t * const ctx)
{
	B	*buf = NULL;

	if (ctx != NULL) {
		fs_lock_lock(LOCKNUM_CTX);
		buf = ctx->x_pathbuf;
		if (buf != NULL) {
			ctx->x_pathbuf = NULL;	/* No more available */
		}
		fs_lock_unlock(LOCKNUM_CTX);
	}
	if (buf == NULL) {
		buf = (B *)fs_malloc(PATH_MAX);
	}
	return buf;
}

/*
 *  Free work buffer for pathname
 */
EXPORT	void	fs_ctx_free_pathbuf(fs_ctx_t * const ctx, B *buf)
{
	if (ctx != NULL) {
		fs_lock_lock(LOCKNUM_CTX);
		if (buf == PATH_BUF(ctx->x_curdir)) {
			ctx->x_pathbuf = buf;	/* Again, available */
			buf = NULL;
		}
		fs_lock_unlock(LOCKNUM_CTX);
	}
	if (buf != NULL) {
		fs_free(buf);
	}
}

/*
 *  Allocate a file descriptor starting from a given file number
 */
EXPORT	INT	fs_ctx_allocfile(fs_ctx_t * const ctx, INT fn)
{
	fs_lock_lock(LOCKNUM_CTX);
	for ( ; fn < ctx->x_maxfile; fn++) {
		if (ctx->x_fileids[fn] != 0) continue;
		ctx->x_fileids[fn] = TMP_FILEID;
		break;
	}
	fs_lock_unlock(LOCKNUM_CTX);

	return (fn >= ctx->x_maxfile) ? -1 : fn;
}

/*
 *  Release file descriptor at error
 */
EXPORT	void	fs_ctx_releasefile(fs_ctx_t * const ctx, INT fn)
{
	fs_lock_lock(LOCKNUM_CTX);
	if (ctx->x_fileids[fn] == TMP_FILEID) {
		ctx->x_fileids[fn] = 0;
	}
	fs_lock_unlock(LOCKNUM_CTX);
}

/*
 *  Free file descriptor
 */
EXPORT	void	fs_ctx_freefile(fs_ctx_t * const ctx, INT fn, fs_file_t *fp)
{
	fs_lock_lock(LOCKNUM_CTX);
	if (fp->f_desc.d_ident == (ctx->x_fileids[fn] | (UINT)FD_CLOEXEC)) {
		ctx->x_fileids[fn] = 0;
	}
	fs_lock_unlock(LOCKNUM_CTX);
}

/*
 *  Set file descriptor
 */
EXPORT	void	fs_ctx_setfile(fs_ctx_t * const ctx, INT fn, fs_file_t *fp)
{
	fs_lock_lock(LOCKNUM_CTX);
	if (ctx->x_fileids[fn] == TMP_FILEID) {
		ctx->x_fileids[fn] = (fp->f_desc.d_ident & (UINT)~FD_CLOEXEC);
	}
	fs_lock_unlock(LOCKNUM_CTX);
}

