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
 *	@(#)netdb.h
 *
 */

#ifndef __NETDB_H__
#define	__NETDB_H__

#include <basic.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

struct addrinfo {
	int		ai_flags;	/* AI_PASSIVE, AI_CANONNAME */
	int		ai_family;	/* PF_xxx */
	int		ai_socktype;	/* SOCK_xxx */
	int		ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
	socklen_t	ai_addrlen;	/* length of ai_addr */
	char		*ai_canonname;	/* canonical name for hostname */
	struct sockaddr	*ai_addr; 	/* binary address */
	struct addrinfo	*ai_next; 	/* next structure in linked list */
};


#define	AI_PASSIVE	0x00000001	/* get address to use bind() */
#define	AI_CANONNAME	0x00000002	/* fill ai_canonname */
#define	AI_NUMERICHOST	0x00000004	/* prevent host name resolution */
#define	AI_NUMERICSERV	0x00000008	/* prevent service name resolution */

#define	NI_MAXHOST	1025
#define	NI_MAXSERV	32

#define	NI_NOFQDN	0x00000001
#define	NI_NUMERICHOST	0x00000002
#define	NI_NAMEREQD	0x00000004
#define	NI_NUMERICSERV	0x00000008
#define	NI_DGRAM	0x00000010
#define	NI_WITHSCOPEID	0x00000020
#define	NI_NUMERICSCOPE	0x00000040

struct hosttable {
	struct sockaddr	*addr;
	char		*host;
	char		*aliases;
};

#define _RESCTL_IN	(0x80000000UL)
#define _RESCTL_OUT	(0x40000000UL)
#define _RESCTL_VOID	(0x20000000UL)
#define _RESCTL_DIRMASK	(0xe0000000UL)

#define _RESCTL_(dir, cmd)	((dir) | (cmd))
#define _RESCTLR(cmd)		_RESCTL_(_RESCTL_OUT, (cmd))
#define _RESCTLW(cmd)		_RESCTL_(_RESCTL_IN, (cmd))
#define _RESCTL(cmd)		_RESCTL_(_RESCTL_VOID, (cmd))

#define SO_RES_ADD_TABLE	_RESCTLW(1)
#define SO_RES_DEL_TABLE	_RESCTLW(2)
#define SO_RES_GET_TABLES	_RESCTLR(3)
#define SO_RES_FLUSH_TABLES	_RESCTL(4)
#define SO_RES_ADD_SERVER	_RESCTLW(5)
#define SO_RES_DEL_SERVER	_RESCTLW(6)
#define SO_RES_GET_SERVERS	_RESCTLR(7)
#define SO_RES_FLUSH_SERVERS	_RESCTL(8)
#define SO_RES_ADD_DOMAIN	_RESCTLW(9)
#define SO_RES_DEL_DOMAIN	_RESCTLW(10)
#define SO_RES_GET_DOMAINS	_RESCTLR(11)
#define SO_RES_FLUSH_DOMAINS	_RESCTL(12)

#ifdef __cplusplus
}
#endif
#endif /* __NETDB_H__ */

