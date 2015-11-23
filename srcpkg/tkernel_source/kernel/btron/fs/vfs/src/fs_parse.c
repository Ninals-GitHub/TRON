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
 *	@(#)fs_parse.c
 *
 */

#include "fsdefs.h"

/*
 *  Normalize the pathname : remove "/./", "/../" and "//" in the pathname.
 */
LOCAL	INT	path_normalize(B *bufs, B *bufe, B *buf, const B *path)
{
	B	c, *p, *pp, *cp;

	/* When the first character isn't '/', add '/' at the top. */
	if ((c = *path++) != '/') {
		c = '/';
		path--;
	}

	/* Normalize pathname */
	for ( ; c != '\0'; c = *path++) {
		if (c == '/') {
			if (*(p = (B*)path) == '.' && *++p == '.') p++;
			if (*p == '/' || *p == '\0') {
				/* "/../" "/./" or "//" */
				if (p - path == 2) {	/* "/../" */
					/* Back to the parent directory */
					for (cp = pp = bufs; cp < buf; cp++) {
						if (*cp == '/' ) pp = cp;
					}
					buf = pp;
				}
				/* Remove "/.." "/." "/" */
				path = p;
				continue;
			}
		}
		if (buf >= bufe) break;	/* Buffer overflow */
		*buf++ = c;
	}
	if (c != '\0') return -1;	/* Buffer overflow : too long path */

	if (buf == bufs) *buf++ = '/';	/* Root only */
	*buf = '\0';
	return 0;
}

/*
 *  Parses the file pathname and returns the path structure
 */
EXPORT	INT	fs_path_parse(fs_env_t *env, const B *name,
						fs_path_t *path, BOOL write)
{
	B	*buf, *str;
	INT	sts, len;

	/* Get pathname work buffer */
	buf = fs_ctx_get_pathbuf(env->t_ctx);
	if (buf == NULL) {
		sts = EX_NOMEM;
		goto exit0;
	}

	/* Check empty name */
	if (name[0] == '\0') {
		sts = EX_NOENT;
		goto exit0;
	}

	len = 0;
	if (name[0] != '/') {
		/* Relative pathname, get current directory in buf[] */
		fs_lock_lock(LOCKNUM_CDIR);
		str = env->t_ctx->x_curdir;
		if (str[1] != '\0') {
			len = strlen(str);
			(void)memcpy(buf, str, len);
			/* The last character of buf[] should not be '/' */
		}
		fs_lock_unlock(LOCKNUM_CDIR);
	}

	/* Make absolute and normalized pathname */
	if (path_normalize(buf, &buf[PATH_MAX - 1], &buf[len], name) < 0) {
		sts = EX_NAMETOOLONG;
		goto exit0;
	}

	/* Check connection name */
	str = strchr(&buf[1], '/');
	if (str != NULL) *str = '\0';		/* Set terminator */

	/* buf[1] == '\0' is specal case of "root" only */
	sts = fs_con_find((buf[1] == '\0') ? buf : &buf[1], &path->p_con);
	if (str != NULL) *str = '/';		/* Recover terminator */
	if (sts != 0) goto exit0;

	/* Check writable file system */
	if (write != FALSE &&
		(path->p_con->c_coninf.dflags & DEV_FLAG_READONLY) != 0) {
		fs_con_release(path->p_con);
		sts = EX_ROFS;
		goto exit0;
	}
	path->p_name = buf;
	return 0;
exit0:
	if (buf != NULL) {
		/* Free pathname buffer */
		fs_ctx_free_pathbuf(env->t_ctx, buf);
	}
	path->p_con = NULL;
	path->p_name = NULL;
	return sts;
}

/*
 *  Destroy the path structure
 */
EXPORT	void	fs_path_release(fs_env_t *env, fs_path_t * const path)
{
	if (path->p_name != NULL) {
		fs_ctx_free_pathbuf(env->t_ctx, path->p_name);
		path->p_name = NULL;
	}
	if (path->p_con != NULL) {
		fs_con_release(path->p_con);
		path->p_con = NULL;
	}
}

