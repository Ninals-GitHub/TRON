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
 *	@(#)strercd_r.c
 *
 */

#include <basic.h>
#include <string.h>
#include <errno.h>
#include <tk/errno.h>

LOCAL	const struct	{
	int	mer;
	const char	*msg;
} e_tbl[] = {
	{MERCD(E_OK), "E_OK: Completed successfully"},
	{MERCD(E_SYS), "E_SYS: System error"},
	{MERCD(E_NOCOP), "E_NOCOP: Coprocessor disable"},
	{MERCD(E_NOSPT), "E_NOSPT: Unsupported function"},
	{MERCD(E_RSFN), "E_RSFN: Reserved function code number"},
	{MERCD(E_RSATR), "E_RSATR: Reserved attribute"},
	{MERCD(E_PAR), "E_PAR: Parameter error"},
	{MERCD(E_ID), "E_ID: Incorrect ID number"},
	{MERCD(E_CTX), "E_CTX: Context error"},
	{MERCD(E_MACV), "E_MACV: Inaccessible memory/access violation"},
	{MERCD(E_OACV), "E_OACV: Object access violation"},
	{MERCD(E_ILUSE), "E_ILUSE: Incorrect system call use"},
	{MERCD(E_NOMEM), "E_NOMEM: Insufficient memory"},
	{MERCD(E_LIMIT), "E_LIMIT: Exceed system limits"},
	{MERCD(E_OBJ), "E_OBJ: Incorrect object state"},
	{MERCD(E_NOEXS), "E_NOEXS: Object does not exist"},
	{MERCD(E_QOVR), "E_QOVR: Queuing overflow"},
	{MERCD(E_RLWAI), "E_RLWAI: Forcibly release wait state"},
	{MERCD(E_TMOUT), "E_TMOUT: Polling fail/time out"},
	{MERCD(E_DLT), "E_DLT: Waited object was deleted"},
	{MERCD(E_DISWAI), "E_DISWAI: Release wait caused by wait disable"},
	{MERCD(E_IO), "E_IO: Output/input error"},
	{MERCD(E_NOMDA), "E_NOMDA: No media"},
	{MERCD(E_BUSY), "E_BUSY: Busy state"},
	{MERCD(E_ABORT), "E_ABORT: Aborted"},
	{MERCD(E_RONLY), "E_RONLY: Write protected"},
};

LOCAL	const struct	{
	int	ser;
	const char	*msg;
} ex_tbl[] = {
	{EPERM, "EX_PERM: Operation not permitted"},
	{ENOENT, "EX_NOENT: No such file or directory"},
	{ESRCH, "EX_SRCH: No such process"},
	{EINTR, "EX_INTR: Interrupted system call"},
	{EIO, "EX_IO: Input/output error"},
	{ENXIO, "EX_NXIO: Device not configured"},
	{E2BIG, "EX_2BIG: Argument list too long"},
	{ENOEXEC, "EX_NOEXEC: Exec format error"},
	{EBADF, "EX_BADF: Bad file descriptor"},
	{ECHILD, "EX_CHILD: No child processes"},
	{EDEADLK, "EX_DEADLK: Resource deadlock avoided"},
	{ENOMEM, "EX_NOMEM: Cannot allocate memory"},
	{EACCES, "EX_ACCES: Permission denied"},
	{EFAULT, "EX_FAULT: Bad address"},
	{EBUSY, "EX_BUSY: Device busy"},
	{EEXIST, "EX_EXIST: File exists"},
	{EXDEV, "EX_XDEV: Cross-device link"},
	{ENODEV, "EX_NODEV: Operation not supported by device"},
	{ENOTDIR, "EX_NOTDIR: Not a directory"},
	{EISDIR, "EX_ISDIR: Is a directory"},
	{EINVAL, "EX_INVAL: Invalid argument"},
	{ENFILE, "EX_NFILE: Too many open files in system"},
	{EMFILE, "EX_MFILE: Too many open files"},
	{ENOTTY, "EX_NOTTY: Inappropriate ioctl for device"},
	{EFBIG, "EX_FBIG: File too large"},
	{ENOSPC, "EX_NOSPC: No space left on device"},
	{ESPIPE, "EX_SPIPE: Illegal seek"},
	{EROFS, "EX_ROFS: Read-only file system"},
	{EMLINK, "EX_MLINK: Too many links"},
	{EPIPE, "EX_PIPE: Broken pipe"},
	{EDOM, "EX_DOM: Numerical argument out of domain"},
	{ERANGE, "EX_RANGE: Result too large"},
	{EAGAIN, "EX_AGAIN: Resource temporarily unavailable"},
	{EINPROGRESS, "EX_INPROGRESS: Operation now in progress"},
	{EALREADY, "EX_ALREADY: Operation already in progress"},
	{ENOTSOCK, "EX_NOTSOCK: Socket operation on non-socket"},
	{EDESTADDRREQ, "EX_DESTADDRREQ: Destination address required"},
	{EMSGSIZE, "EX_MSGSIZE: Message too long"},
	{EPROTOTYPE, "EX_PROTOTYPE: Protocol wrong type for socket"},
	{ENOPROTOOPT, "EX_NOPROTOOPT: Protocol not available"},
	{EPROTONOSUPPORT, "EX_PROTONOSUPPORT: Protocol not supported"},
	{ESOCKTNOSUPPORT, "EX_SOCKTNOSUPPORT: Socket type not supported"},
	{EOPNOTSUPP, "EX_OPNOTSUPP: Operation not supported"},
	{EPFNOSUPPORT, "EX_PFNOSUPPORT: Protocol family not supported"},
	{EADDRINUSE, "EX_ADDRINUSE: Address already in use"},
	{EADDRNOTAVAIL, "EX_ADDRNOTAVAIL: Can't assign requested address"},
	{ENETDOWN, "EX_NETDOWN: Network is down"},
	{ENETUNREACH, "EX_NETUNREACH: Network is unreachable"},
	{ENETRESET, "EX_NETRESET: Network dropped connection on reset"},
	{ECONNABORTED, "EX_CONNABORTED: Software caused connection abort"},
	{ECONNRESET, "EX_CONNRESET: Connection reset by peer"},
	{ENOBUFS, "EX_NOBUFS: No buffer space available"},
	{EISCONN, "EX_ISCONN: Socket is already connected"},
	{ENOTCONN, "EX_NOTCONN: Socket is not connected"},
	{ESHUTDOWN, "EX_SHUTDOWN: Can't send after socket shutdown"},
	{ETIMEDOUT, "EX_TIMEDOUT: Operation timed out"},
	{ECONNREFUSED, "EX_CONNREFUSED: Connection refused"},
	{ELOOP, "EX_LOOP: Too many levels of symbolic links"},
	{ENAMETOOLONG, "EX_NAMETOOLONG: File name too long"},
	{EHOSTDOWN, "EX_HOSTDOWN: Host is down"},
	{EHOSTUNREACH, "EX_HOSTUNREACH: No route to host"},
	{ENOTEMPTY, "EX_NOTEMPTY: Directory not empty"},
	{EDQUOT, "EX_DQUOT: Disk quota exceeded"},
	{ENOLCK, "EX_NOLCK: No locks available"},
	{ENOSYS, "EX_NOSYS: Function not implemented"},
	{EFTYPE, "EX_FTYPE: Inappropriate file type or format"},
	{EILSEQ, "EX_ILSEQ: Illegal byte sequence"},
	{ENOTSUP, "EX_NOTSUP: Not supported"},
};

EXPORT	int	strercd_r(ER er, char *buf, size_t buflen)
{
	int	i, n, mer, ser;
	const char	*p;

	p = NULL;
	mer = MERCD(er);
	if (mer != EC_ERRNO) {
		for (i = 0; i < (int)(sizeof(e_tbl) / sizeof(*e_tbl)); i++) {
			if (e_tbl[i].mer == mer) {
				p = e_tbl[i].msg;
				break;
			}
		}
	} else {
		ser = SERCD(er);
		for (i = 0; i < (int)(sizeof(ex_tbl) / sizeof(*ex_tbl)); i++) {
			if (ex_tbl[i].ser == ser) {
				p = ex_tbl[i].msg;
				break;
			}
		}
	}

	if (p != NULL) {
		n = strlen( p );
		if (n + 1 > (int)buflen) goto e2;
		strcpy( buf, p );
		return 0;
	}

	return EINVAL;
e2:	return ERANGE;
}
