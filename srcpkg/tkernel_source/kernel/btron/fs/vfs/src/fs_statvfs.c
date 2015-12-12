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
 *    Modified by Nina Petipa at 2015/12/05
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
 *	@(#)fs_statvfs.c
 *
 */

#include <bk/kernel.h>
#include <bk/uapi/sys/stat.h>

#include "fsdefs.h"

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/

/*
==================================================================================

	Management 

==================================================================================
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
 *  fs_statvfs() - Get FIMP status by file name
 */
EXPORT	INT	xfs_statvfs(fs_env_t *env, const B *name, struct statvfs *ptr)
{
	fs_path_t	path;
	INT		sts;

	/* Parse the pathname & get path structure */
	sts = fs_path_parse(env, name, &path, FALSE);
	if (sts == 0) {
		/* Call FIMP STATVFS service */
		env->t_cmd.r_statvfs.path = path.p_name;
		env->t_cmd.r_statvfs.buf = ptr;
		sts = fs_callfimp(env, path.p_con, FIMP_STATVFS);

		/* Release path structure */
		fs_path_release(env, &path);
	}
	if (sts == 0) return 0; 
	env->t_errno = sts;
	return -1;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fstat64
 Input		:int fd
 		 < open file descriptor to get status >
 		 struct stat64 *buf
 		 < file status information buffer >
 Output		:struct stat64 *buf
 		 < file status information buffer >
 Return		:void
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fstat64(int fd, struct stat64 *buf)
{
	int err;
	
	//printf("fstat64[fd=%d]:\n", fd);
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	err = ChkSpaceRW(buf, sizeof(struct stat64));
	
	if (err) {
		return(-EFAULT);
	}
	
	err = fs_fstat64(fd, buf);
#if 0
	if (err) printf("err:fstat64\n");
	printf("st_dev=%d, ", buf->st_dev);
	printf("st_ino=%d, ", buf->st_ino);
	printf("st_mode=0x%04X, ", buf->st_mode);
	printf("st_nlink=%d, ", buf->st_nlink);
	printf("st_uid=%d, ", buf->st_uid);
	printf("st_gid=%d, ", buf->st_gid);
	printf("st_rdev=%d, ", buf->st_rdev);
	printf("st_size=%ld, ", buf->st_size);
	printf("st_atime=%d, ", buf->st_atime);
	printf("st_mtime=%d, ", buf->st_mtime);
	printf("st_ctime=%d, ", buf->st_ctime);
	printf("st_blksize=%d, ", buf->st_blksize);
	printf("st_blocks=%d\n", buf->st_blocks);
#endif
	return(err);
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
