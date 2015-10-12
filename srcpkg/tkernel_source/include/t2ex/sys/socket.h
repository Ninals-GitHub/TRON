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
 *	@(#)socket.h
 *
 */

#ifndef __SYS_SOCKET_H__
#define __SYS_SOCKET_H__

#include <basic.h>
#include <stdint.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Socket types.
 */
#define	SOCK_STREAM	1		/* stream socket */
#define	SOCK_DGRAM	2		/* datagram socket */
#define	SOCK_RAW	3		/* raw-protocol interface */
#define	SOCK_RDM	4		/* reliably-delivered message */
#define	SOCK_SEQPACKET	5		/* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x0001		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define	SO_REUSEADDR	0x0004		/* allow local address reuse */
#define	SO_KEEPALIVE	0x0008		/* keep connections alive */
#define	SO_DONTROUTE	0x0010		/* just use interface addresses */
#define	SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define	SO_LINGER	0x0080		/* linger on close if data present */
#define	SO_OOBINLINE	0x0100		/* leave received OOB data in line */
#define	SO_REUSEPORT	0x0200		/* allow local address & port reuse */
#define	SO_TIMESTAMP	0x0400		/* timestamp received dgram traffic */
#define	SO_ACCEPTFILTER	0x1000		/* there is an accept filter */


/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF	0x1001		/* send buffer size */
#define SO_RCVBUF	0x1002		/* receive buffer size */
#define SO_SNDLOWAT	0x1003		/* send low-water mark */
#define SO_RCVLOWAT	0x1004		/* receive low-water mark */
#define SO_SNDTIMEO	0x1005		/* send timeout */
#define SO_RCVTIMEO	0x1006		/* receive timeout */
#define	SO_ERROR	0x1007		/* get error status and clear */
#define	SO_TYPE		0x1008		/* get socket type */
#define	SO_OVERFLOWED	0x1009		/* datagrams: return packets dropped */

#define	SO_NOHEADER	0x100a		/* user supplies no header to kernel;
					 * kernel removes header and supplies
					 * payload
					 */

/*
 * Structure used for manipulating linger option.
 */
struct	linger {
	int	l_onoff;		/* option on/off */
	int	l_linger;		/* linger time in seconds */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		/* options for socket level */

/*
 * Address families.
 */
#define	AF_UNSPEC	0		/* unspecified */
#define	AF_INET		2		/* internetwork: UDP, TCP, etc. */
#define	AF_ROUTE	17		/* Internal Routing Protocol */
#define	AF_LINK		18		/* Link layer interface */

/*
 * Protocol families, same as address families for now.
 */
#define	PF_UNSPEC	AF_UNSPEC
#define	PF_INET		AF_INET
#define	PF_ROUTE	AF_ROUTE
#define	PF_LINK		AF_LINK

struct sockaddr {
	uint8_t		sa_len;		/* total length */
	sa_family_t	sa_family;	/* address family */
	char		sa_data[14];	/* up to 14 bytes of direct address */
};

/*
 * RFC 2553: protocol-independent placeholder for socket addresses
 */
#define _SS_MAXSIZE	128
#define _SS_ALIGNSIZE	(sizeof(int64_t))
#define _SS_PAD1SIZE	(_SS_ALIGNSIZE - 2)
#define _SS_PAD2SIZE	(_SS_MAXSIZE - 2 - \
				_SS_PAD1SIZE - _SS_ALIGNSIZE)

struct sockaddr_storage {
	uint8_t		ss_len;		/* address length */
	sa_family_t	ss_family;	/* address family */
	char		__ss_pad1[_SS_PAD1SIZE];
	int64_t		__ss_align;/* force desired structure storage alignment */
	char		__ss_pad2[_SS_PAD2SIZE];
};

#define NET_RT_DUMP	1		/* dump; may limit to a.f. */
#define NET_RT_FLAGS	2		/* by flags, e.g. RESOLVING */
#define NET_RT_IFLIST	4		/* survey interface list */

#define	SOMAXCONN	128

struct msghdr { 			/* Message header */
	void		*msg_name;	/* optional address */
	socklen_t	msg_namelen;	/* size of address */
	struct iovec	*msg_iov;	/* scatter/gather array */
	int		msg_iovlen;	/* # elements in msg_iov */
	void		*msg_control;	/* ancillary data */
	socklen_t	msg_controllen;	/* ancillary data buffer len */
	int		msg_flags;	/* flags on received message */
};

#define	MSG_OOB		0x0001		/* process out-of-band data */
#define	MSG_PEEK	0x0002		/* peek at incoming message */
#define	MSG_DONTROUTE	0x0004		/* send without using routing tables */
#define	MSG_EOR		0x0008		/* data completes record */
#define	MSG_TRUNC	0x0010		/* data discarded before delivery */
#define	MSG_CTRUNC	0x0020		/* control data lost before delivery */
#define	MSG_WAITALL	0x0040		/* wait for full request or error */
#define	MSG_DONTWAIT	0x0080		/* this message should be nonblocking */
#define	MSG_BCAST	0x0100		/* this message was rcvd using link-level brdcst */
#define	MSG_MCAST	0x0200		/* this message was rcvd using link-level mcast */
#define	MSG_NOSIGNAL	0x0400		/* do not generate SIGPIPE on EOF */

struct cmsghdr {
	socklen_t	cmsg_len;	/* data byte count, including hdr */
	int		cmsg_level;	/* originating protocol */
	int		cmsg_type;	/* protocol-specific type */
/* followed by	u_char  cmsg_data[]; */
};

struct iovec {				/* address and length */
	void		*iov_base;	/* base */
	size_t		iov_len;	/* size */
};

/*
 * Types of socket shutdown(2).
 */
#define	SHUT_RD		0		/* Disallow further receives. */
#define	SHUT_WR		1		/* Disallow further sends. */
#define	SHUT_RDWR	2		/* Disallow further sends/receives. */

#ifdef __cplusplus
}
#endif
#endif	/* __SYS_SOCKET_H__ */

