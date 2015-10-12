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
 *	@(#)strerror_r.c
 *
 */

#include <basic.h>
#include <string.h>
#include <errno.h>

LOCAL	const struct	{
	int	er;
	const	char	*msg;
} er_tbl[] = {
	{EPERM, "EPERM: Operation not permitted"},
	{ENOENT, "ENOENT: No such file or directory"},
	{ESRCH, "ESRCH: No such process"},
	{EINTR, "EINTR: Interrupted system call"},
	{EIO, "EIO: Input/output error"},
	{ENXIO, "ENXIO: Device not configured"},
	{E2BIG, "E2BIG: Argument list too long"},
	{ENOEXEC, "ENOEXEC: Exec format error"},
	{EBADF, "EBADF: Bad file descriptor"},
	{ECHILD, "ECHILD: No child processes"},
	{EDEADLK, "EDEADLK: Resource deadlock avoided"},
	{ENOMEM, "ENOMEM: Cannot allocate memory"},
	{EACCES, "EACCES: Permission denied"},
	{EFAULT, "EFAULT: Bad address"},
	{EBUSY, "EBUSY: Device busy"},
	{EEXIST, "EEXIST: File exists"},
	{EXDEV, "EXDEV: Cross-device link"},
	{ENODEV, "ENODEV: Operation not supported by device"},
	{ENOTDIR, "ENOTDIR: Not a directory"},
	{EISDIR, "EISDIR: Is a directory"},
	{EINVAL, "EINVAL: Invalid argument"},
	{ENFILE, "ENFILE: Too many open files in system"},
	{EMFILE, "EMFILE: Too many open files"},
	{ENOTTY, "ENOTTY: Inappropriate ioctl for device"},
	{EFBIG, "EFBIG: File too large"},
	{ENOSPC, "ENOSPC: No space left on device"},
	{ESPIPE, "ESPIPE: Illegal seek"},
	{EROFS, "EROFS: Read-only file system"},
	{EMLINK, "EMLINK: Too many links"},
	{EPIPE, "EPIPE: Broken pipe"},
	{EDOM, "EDOM: Numerical argument out of domain"},
	{ERANGE, "ERANGE: Result too large"},
	{EAGAIN, "EAGAIN: Resource temporarily unavailable"},
	{EINPROGRESS, "EINPROGRESS: Operation now in progress"},
	{EALREADY, "EALREADY: Operation already in progress"},
	{ENOTSOCK, "ENOTSOCK: Socket operation on non-socket"},
	{EDESTADDRREQ, "EDESTADDRREQ: Destination address required"},
	{EMSGSIZE, "EMSGSIZE: Message too long"},
	{EPROTOTYPE, "EPROTOTYPE: Protocol wrong type for socket"},
	{ENOPROTOOPT, "ENOPROTOOPT: Protocol not available"},
	{EPROTONOSUPPORT, "EPROTONOSUPPORT: Protocol not supported"},
	{ESOCKTNOSUPPORT, "ESOCKTNOSUPPORT: Socket type not supported"},
	{EOPNOTSUPP, "EOPNOTSUPP: Operation not supported"},
	{EPFNOSUPPORT, "EPFNOSUPPORT: Protocol family not supported"},
	{EADDRINUSE, "EADDRINUSE: Address already in use"},
	{EADDRNOTAVAIL, "EADDRNOTAVAIL: Can't assign requested address"},
	{ENETDOWN, "ENETDOWN: Network is down"},
	{ENETUNREACH, "ENETUNREACH: Network is unreachable"},
	{ENETRESET, "ENETRESET: Network dropped connection on reset"},
	{ECONNABORTED, "ECONNABORTED: Software caused connection abort"},
	{ECONNRESET, "ECONNRESET: Connection reset by peer"},
	{ENOBUFS, "ENOBUFS: No buffer space available"},
	{EISCONN, "EISCONN: Socket is already connected"},
	{ENOTCONN, "ENOTCONN: Socket is not connected"},
	{ESHUTDOWN, "ESHUTDOWN: Can't send after socket shutdown"},
	{ETIMEDOUT, "ETIMEDOUT: Operation timed out"},
	{ECONNREFUSED, "ECONNREFUSED: Connection refused"},
	{ELOOP, "ELOOP: Too many levels of symbolic links"},
	{ENAMETOOLONG, "ENAMETOOLONG: File name too long"},
	{EHOSTDOWN, "EHOSTDOWN: Host is down"},
	{EHOSTUNREACH, "EHOSTUNREACH: No route to host"},
	{ENOTEMPTY, "ENOTEMPTY: Directory not empty"},
	{EDQUOT, "EDQUOT: Disk quota exceeded"},
	{ENOLCK, "ENOLCK: No locks available"},
	{ENOSYS, "ENOSYS: Function not implemented"},
	{EFTYPE, "EFTYPE: Inappropriate file type or format"},
	{EILSEQ, "EILSEQ: Illegal byte sequence"},
	{ENOTSUP, "ENOTSUP: Not supported"},
};

EXPORT	int	strerror_r(errno_t errnum, char *buf, size_t buflen)
{
	int	i, n;

	for (i = 0; i < (int)(sizeof(er_tbl) / sizeof(*er_tbl)); i++) {
		if (er_tbl[i].er == errnum) {
			n = strlen( er_tbl[i].msg );
			if (n + 1 > (int)buflen) goto e2;
			strcpy( buf, er_tbl[i].msg );
			return 0;
		}
	}

	return EINVAL;
e2:	return ERANGE;
}
