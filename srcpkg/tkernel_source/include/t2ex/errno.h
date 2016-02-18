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
 *	@(#)errno.h
 *
 */

#ifndef __ERRNO_H__
#define __ERRNO_H__

#include <tk/errno.h>

#ifdef _BTRON_
	#include <bk/uapi/btron/errno.h>
	#include <bk/uapi/errcode.h>
#endif

#ifdef	__cplusplus
extern	"C" {
#endif

/* Symbol to specify all tasks */
#define TSK_ALL			(-1)

/* Error number type */
typedef	int	errno_t;

/* Convert main and sub error code to TK2/T2EX error code */
#ifndef ERCD
#define ERCD(mer, ser)		( ((mer) << 16) | ((ser) & 0xffff) )
#endif

/* Main error code for T2EX extended error code */
#ifndef EC_ERRNO
#define EC_ERRNO		(-73)
#endif

/* Convert T2EX extended error code to error number */
#ifndef ERRNO
#define ERRNO(er)		(MERCD(er) == EC_ERRNO ? SERCD(er) : 0)
#endif

/* Convert error number to T2EX extended error code */
#ifndef ERRNOtoER
#define ERRNOtoER(eno)		(ERCD(EC_ERRNO, (eno)))
#endif

/* 
 *  Error numbers:
 *	(*) will not occur in T2EX implementation
 */
#ifndef _BTRON_
#define EPERM		1	/* Operation not permitted		*/
#define ENOENT		2	/* No such file or directory		*/
#define ESRCH		3	/* No such process(*)			*/
#define EINTR		4	/* Interrupted function			*/
#define EIO		5	/* I/O error				*/
#define ENXIO		6	/* No such device or address		*/
#define E2BIG		7	/* Argument list too long		*/
#define ENOEXEC 	8	/* Executable file format error		*/
#define EBADF		9	/* Bad file descriptor			*/
#define ECHILD		10	/* No child processes(*)		*/
#define EDEADLK 	11	/* Resource deadlock would occur	*/
#define ENOMEM		12	/* Not enough space			*/
#define EACCES		13	/* Permission denied			*/
#define EFAULT		14	/* Bad address				*/

#define EBUSY		16	/* Device or resource busy		*/
#define EEXIST		17	/* File exists				*/
#define EXDEV		18	/* Cross-device link			*/
#define ENODEV		19	/* No such device			*/
#define ENOTDIR 	20	/* Not a director			*/
#define EISDIR		21	/* Is a directory			*/
#define EINVAL		22	/* Invalid argument			*/
#define ENFILE		23	/* Too many files open in system	*/
#define EMFILE		24	/* File descriptor value too large(*)	*/
#define ENOTTY		25	/* Inappropriate I/O control operation	*/

#define EFBIG		27	/* File too large			*/
#define ENOSPC		28	/* No space left on device		*/
#define ESPIPE		29	/* Invalid seek				*/
#define EROFS		30	/* Read-only file system		*/
#define EMLINK		31	/* Too many links(*)			*/
#define EPIPE		32	/* Broken pipe(*)			*/
#define EDOM		33	/* Mathematics argument
					out of domain of function	*/
#define ERANGE		34	/* Result too large			*/
#define EAGAIN		35	/* Resource unavailable, try again	*/
#define EWOULDBLOCK	EAGAIN	/* Operation would block		*/
#define EINPROGRESS	36	/* Operation in progress		*/
#define EALREADY	37	/* Connection already in progress	*/
#define ENOTSOCK	38	/* Not a socket				*/
#define EDESTADDRREQ	39	/* Destination address required		*/
#define EMSGSIZE	40	/* Message too large			*/
#define EPROTOTYPE	41	/* Protocol wrong type for socket	*/
#define ENOPROTOOPT	42	/* Protocol not available		*/
#define EPROTONOSUPPORT 43	/* Unsupported protocol			*/
#define ESOCKTNOSUPPORT 44	/* Unsupported socket type		*/
#define EOPNOTSUPP	45	/* Operation unsupported on socket	*/
#define EPFNOSUPPORT	46	/* Unsupported protocol family		*/
#define EAFNOSUPPORT	47	/* Address family unsupported by protocol */
#define EADDRINUSE	48	/* Address in use			*/
#define EADDRNOTAVAIL	49	/* Address not available		*/
#define ENETDOWN	50	/* Network is down			*/
#define ENETUNREACH	51	/* Network unreachable			*/
#define ENETRESET	52	/* Connection aborted by network	*/
#define ECONNABORTED	53	/* Connection aborted
				   (The problem of the local host side) */
#define ECONNRESET	54	/* Connection reset(Reset by peer)	*/
#define ENOBUFS 	55	/* No buffer space available		*/
#define EISCONN 	56	/* Socket is connected			*/
#define ENOTCONN	57	/* The socket is not connected		*/
#define ESHUTDOWN	58	/* Cannot send after transport
						endpoint shutdown	*/

#define ETIMEDOUT	60	/* Connection timed out			*/
#define ECONNREFUSED	61	/* Connection refused(Rejected by peer) */
#define ELOOP		62	/* Too many levels of symbolic links(*)	*/
#define ENAMETOOLONG	63	/* Filename too long			*/
#define EHOSTDOWN	64	/* Remote host is down			*/
#define EHOSTUNREACH	65	/* Host is unreachable			*/
#define ENOTEMPTY	66	/* Directory not empty			*/

#define EDQUOT		69	/* Disk quota exceeded(*)		*/

#define ENOLCK		77	/* No locks available(*)		*/
#define ENOSYS		78	/* Unsupported function			*/
#define EFTYPE		79	/* Inappropriate file type or format	*/

#define EOVERFLOW	84	/* Value too large to be stored in data type */
#define EILSEQ		85	/* Illegal byte sequence		*/
#endif
#define ENOTSUP 	86	/* Unsupported				*/

#define	ELAST		86	/* The last error number		*/

#define EFTYPE		79	/* Inappropriate file type or format	*/


/* 
 *  Error numbers for so_getaddrinfo() and so_getnameinfo()
 */
#define EAI_OFFSET	256
#define	EAI_AGAIN	(EAI_OFFSET +  2)	/* temporary failure in name resolution */
#define	EAI_BADFLAGS	(EAI_OFFSET +  3)	/* invalid value for ai_flags */
#define	EAI_FAIL	(EAI_OFFSET +  4)	/* non-recoverable failure in name resolution */
#define	EAI_FAMILY	(EAI_OFFSET +  5)	/* ai_family not supported */
#define	EAI_MEMORY	(EAI_OFFSET +  6)	/* memory allocation failure */
#define	EAI_NODATA	(EAI_OFFSET +  7)	/* no address associated with hostname */
#define	EAI_NONAME	(EAI_OFFSET +  8)	/* hostname nor servname provided, or not known */
#define	EAI_SERVICE	(EAI_OFFSET +  9)	/* servname not supported for ai_socktype */
#define	EAI_SOCKTYPE	(EAI_OFFSET + 10)	/* ai_socktype not supported */
#define	EAI_SYSTEM	(EAI_OFFSET + 11)	/* internal error */
#define	EAI_BADHINTS	(EAI_OFFSET + 12)	/* invalid value for hints */
#define	EAI_OVERFLOW	(EAI_OFFSET + 14)	/* argument buffer overflow */
#define	EAI_MAX		(EAI_OFFSET + 15)


/*
 *  T2EX extended error codes
 */
#define EX_PERM 		ERCD(EC_ERRNO, EPERM)
#define EX_NOENT		ERCD(EC_ERRNO, ENOENT)

#ifndef _BTRON_
#define EX_SRCH 		ERCD(EC_ERRNO, ESRCH)
#endif

#define EX_IO			ERCD(EC_ERRNO, EIO)
#define EX_NXIO 		ERCD(EC_ERRNO, ENXIO)

#ifndef _BTRON_
#define EX_2BIG 		ERCD(EC_ERRNO, E2BIG)
#endif

#define EX_NOEXEC		ERCD(EC_ERRNO, ENOEXEC)

#ifndef _BTRON_
#define EX_BADF 		ERCD(EC_ERRNO, EBADF)
#define EX_CHILD		ERCD(EC_ERRNO, ECHILD)
#endif

#define EX_AGAIN		ERCD(EC_ERRNO, EAGAIN)

#ifndef _BTRON_
#define EX_DEADLK		ERCD(EC_ERRNO, EDEADLK)
#endif

#define EX_NOMEM		ERCD(EC_ERRNO, ENOMEM)
#define EX_ACCES		ERCD(EC_ERRNO, EACCES)
#define EX_FAULT		ERCD(EC_ERRNO, EFAULT)
#define EX_BUSY 		ERCD(EC_ERRNO, EBUSY)
#define EX_EXIST		ERCD(EC_ERRNO, EEXIST)
#define EX_XDEV 		ERCD(EC_ERRNO, EXDEV)
#define EX_NODEV		ERCD(EC_ERRNO, ENODEV)
#define EX_NOTDIR		ERCD(EC_ERRNO, ENOTDIR)
#define EX_ISDIR		ERCD(EC_ERRNO, EISDIR)
#define EX_INVAL		ERCD(EC_ERRNO, EINVAL)
#define EX_NFILE		ERCD(EC_ERRNO, ENFILE)
#define EX_MFILE		ERCD(EC_ERRNO, EMFILE)

#ifndef _BTRON_
#define EX_NOTTY		ERCD(EC_ERRNO, ENOTTY)
#endif

#define EX_FBIG 		ERCD(EC_ERRNO, EFBIG)

#ifndef _BTRON_
#define EX_NOSPC		ERCD(EC_ERRNO, ENOSPC)
#endif

#define EX_SPIPE		ERCD(EC_ERRNO, ESPIPE)
#define EX_ROFS 		ERCD(EC_ERRNO, EROFS)

#ifndef _BTRON_
#define EX_MLINK		ERCD(EC_ERRNO, EMLINK)
#define EX_PIPE 		ERCD(EC_ERRNO, EPIPE)
#define EX_DOM			ERCD(EC_ERRNO, EDOM)
#endif

#define EX_RANGE		ERCD(EC_ERRNO, ERANGE)

#ifndef _BTRON_
#define EX_WOULDBLOCK		ERCD(EC_ERRNO, EWOULDBLOCK)
#define EX_INPROGRESS		ERCD(EC_ERRNO, EINPROGRESS)
#define EX_ALREADY		ERCD(EC_ERRNO, EALREADY)
#define EX_NOTSOCK		ERCD(EC_ERRNO, ENOTSOCK)
#define EX_DESTADDRREQ		ERCD(EC_ERRNO, EDESTADDRREQ)
#define EX_MSGSIZE		ERCD(EC_ERRNO, EMSGSIZE)
#define EX_PROTOTYPE		ERCD(EC_ERRNO, EPROTOTYPE)
#define EX_NOPROTOOPT		ERCD(EC_ERRNO, ENOPROTOOPT)
#define EX_PROTONOSUPPORT	ERCD(EC_ERRNO, EPROTONOSUPPORT)
#define EX_SOCKTNOSUPPORT	ERCD(EC_ERRNO, ESOCKTNOSUPPORT)
#define EX_OPNOTSUPP		ERCD(EC_ERRNO, EOPNOTSUPP)
#define EX_PFNOSUPPORT		ERCD(EC_ERRNO, EPFNOSUPPORT)
#define EX_AFNOSUPPORT		ERCD(EC_ERRNO, EAFNOSUPPORT)
#define EX_ADDRINUSE		ERCD(EC_ERRNO, EADDRINUSE)
#define EX_ADDRNOTAVAIL 	ERCD(EC_ERRNO, EADDRNOTAVAIL)
#define EX_NETDOWN		ERCD(EC_ERRNO, ENETDOWN)
#define EX_NETUNREACH		ERCD(EC_ERRNO, ENETUNREACH)
#define EX_NETRESET		ERCD(EC_ERRNO, ENETRESET)
#define EX_CONNABORTED		ERCD(EC_ERRNO, ECONNABORTED)
#define EX_CONNRESET		ERCD(EC_ERRNO, ECONNRESET)
#define EX_NOBUFS		ERCD(EC_ERRNO, ENOBUFS)
#define EX_ISCONN		ERCD(EC_ERRNO, EISCONN)
#define EX_NOTCONN		ERCD(EC_ERRNO, ENOTCONN)
#define EX_SHUTDOWN		ERCD(EC_ERRNO, ESHUTDOWN)
#define EX_TIMEDOUT		ERCD(EC_ERRNO, ETIMEDOUT)
#define EX_CONNREFUSED		ERCD(EC_ERRNO, ECONNREFUSED)
#define EX_LOOP 		ERCD(EC_ERRNO, ELOOP)
#endif

#define EX_NAMETOOLONG		ERCD(EC_ERRNO, ENAMETOOLONG)

#ifndef _BTRON_
#define EX_HOSTDOWN		ERCD(EC_ERRNO, EHOSTDOWN)
#define EX_HOSTUNREACH		ERCD(EC_ERRNO, EHOSTUNREACH)
#endif

#define EX_NOTEMPTY		ERCD(EC_ERRNO, ENOTEMPTY)

#ifndef _BTRON_
#define EX_DQUOT		ERCD(EC_ERRNO, EDQUOT)
#define EX_NOLCK		ERCD(EC_ERRNO, ENOLCK)
#endif

#define EX_NOSYS		ERCD(EC_ERRNO, ENOSYS)

#ifndef _BTRON_
#define EX_FTYPE		ERCD(EC_ERRNO, EFTYPE)
#define EX_ILSEQ		ERCD(EC_ERRNO, EILSEQ)
#endif

#define EX_NOTSUP		ERCD(EC_ERRNO, ENOTSUP)
#define EX_INTR 		ERCD(EC_ERRNO, EINTR)
#define EX_OVERFLOW		ERCD(EC_ERRNO, EOVERFLOW)

#ifndef _BTRON_
#define	EX_AI_AGAIN		ERCD(EC_ERRNO, EAI_AGAIN)
#define	EX_AI_BADFLAGS		ERCD(EC_ERRNO, EAI_BADFLAGS)
#define	EX_AI_FAIL		ERCD(EC_ERRNO, EAI_FAIL)
#define	EX_AI_FAMILY		ERCD(EC_ERRNO, EAI_FAMILY)
#define	EX_AI_MEMORY		ERCD(EC_ERRNO, EAI_MEMORY)
#define	EX_AI_NODATA		ERCD(EC_ERRNO, EAI_NODATA)
#define	EX_AI_NONAME		ERCD(EC_ERRNO, EAI_NONAME)
#define	EX_AI_SERVICE		ERCD(EC_ERRNO, EAI_SERVICE)
#define	EX_AI_SOCKTYPE		ERCD(EC_ERRNO, EAI_SOCKTYPE)
#define	EX_AI_SYSTEM		ERCD(EC_ERRNO, EAI_SYSTEM)
#define	EX_AI_BADHINTS		ERCD(EC_ERRNO, EAI_BADHINTS)
#define	EX_AI_OVERFLOW		ERCD(EC_ERRNO, EAI_OVERFLOW)
#endif
#ifdef	__cplusplus
}
#endif

#endif /* __ERRNO_H__ */

