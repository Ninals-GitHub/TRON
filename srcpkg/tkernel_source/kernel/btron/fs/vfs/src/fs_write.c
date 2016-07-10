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
 *	@(#)fs_write.c
 *
 */

#include <typedef.h>
#include <bk/kernel.h>
#include <bk/uapi/errno.h>
#include <bk/uio.h>
#include <tk/kernel.h>

#include "fsdefs.h"

/*
 *  Write data to file
 */
LOCAL	INT	xfs_write_impl(fs_env_t *env, fs_cond_t *con, fs_file_t *file,
				const void *buf, size_t num, off64_t off)
{
	fimp_t	*cmd;
	INT	sts;

	sts = 0;
	if ((INT)num < 0) {
		sts = EX_INVAL;
		goto exit0;
	}
	if (fs_lockspace((void *)buf, num) != 0) {
		sts = EX_ACCES;
		goto exit0;
	}

	/* Check 64 bits offset */
	sts = fs_check_64bits(con, off);
	if (sts == 0) {
		/* Call FIMP WRITE64 service */
		env->t_misc.t_rw.t_len = num;
		env->t_misc.t_rw.t_off64 = off;

		cmd = &env->t_cmd;
		cmd->r_write64.fid = file->f.f_fid;
		cmd->r_write64.oflags = file->f_desc.d_flags;
		cmd->r_write64.buf = (void *)buf;
		cmd->r_write64.len = &env->t_misc.t_rw.t_len;
		cmd->r_write64.off = (off64_t *)&env->t_misc.t_rw.t_off64;
		cmd->r_write64.retoff = (off64_t *)&env->t_misc.t_rw.t_off64;
		sts = fs_callfimp(env, con, FIMP_WRITE64);
		FILE_OFFSET(file) = env->t_misc.t_rw.t_off64;
	}
	(void)fs_unlockspace((void *)buf, num);
exit0:
	return sts;
}

/*
 *  fs_write() - Write data to file
 */
EXPORT	INT	xfs_write(fs_env_t *env, INT fno,
					const void *buf, size_t num)
{
	fs_file_t	*file;
	fs_cond_t	*con;
	INT		sts;

	env->t_misc.t_rw.t_len = 0;

	/* Get file and connection descriptor */
	sts = fs_file_get(env, fno, &file, &con);
	if (sts != 0) goto exit0;

	/* Check open mode */
	if ((file->f_desc.d_flags & O_ACCMODE) == O_RDONLY) {
		sts = EX_BADF;
		goto exit1;
	}

	/* Write data */
	if (num != 0) {
		sts = xfs_write_impl(env, con, file,
					buf, num, FILE_OFFSET(file));
	}
exit1:
	/* Release file and connection descriptor */
	(void)fs_file_release(env, file, con);
	if (sts == 0) return (INT)env->t_misc.t_rw.t_len;
exit0:
	env->t_errno = sts;
	return -1;
}


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
#if 0
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:write
 Input		:int fd
 		 < open file descriptor >
 		 const void *buf
 		 < user buffer >
 		 size_t count
 		 < size of user buffer >
 Output		:void
 Return		:ssize_t
 		 < size of written bytes >
 Description	:write a buffer to an open file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL ssize_t write(int fd, const void *buf, size_t count)
{
	int err;
	
	if (UNLIKELY(!buf)) {
		return(-EFAULT);
	}
	
	err = ChkSpace((CONST void*)buf, count, MA_READ, TMF_PPL(USER_RPL));
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
#if 0
	printf("write[fd=%d", fd);
	printf(", *buf=%s", buf);
	printf(", count=%d]\n", count);
#endif
	return(fs_write(fd, buf, count));
}
#endif
#if 0
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:writev
 Input		:int fd
 		 < open file descriptor >
 		 const struct iovec *iov
 		 < io vector to write >
 		 int iovcnt
 		 < number of io buffer >
 Output		:void
 Return		:ssize_t
 		 < size of written bytes > 
 Description	:write buffers to an open file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
	int i;
	ssize_t len = 0;

	if (UNLIKELY(!iov)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(iovcnt < 0 || UIO_MAXIOV < iovcnt)) {
		return(-EINVAL);
	}
	
	printf("writev start-------\n");
	
	for (i = 0;i < iovcnt;i++) {
		ssize_t write_len;
		printf("fd:%d iov_base:0x%08X iov_len:0x%08X\n", fd, iov[i].iov_base, iov[i].iov_len);
		//write_len = fs_write(fd, iov->iov_base, iov->iov_len);
		if (UNLIKELY(write_len < 0)) {
			return(write_len);
		}
		
		len += write_len;
		
		if (UNLIKELY(len < 0)) {
			return(-EINVAL);
		}
	}
	
	printf("writev end---------\n");
	
	return(len);
}
#endif
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
