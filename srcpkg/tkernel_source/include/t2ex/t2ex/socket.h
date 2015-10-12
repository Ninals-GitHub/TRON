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

#ifndef	__T2EX_SOCKET_H__
#define	__T2EX_SOCKET_H__

#include <basic.h>
#include <tk/typedef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fd_set.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/if_types.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_ether.h>
#include <net/route.h>
#include <limits.h>
#include <netdb.h>
#include <ifaddrs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Definition for interface library automatic generation (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<t2ex/socket.h>

[PREFIX]
NET
***/

/* Maximum length of host name */
#define HOST_NAME_MAX	(255)

IMPORT	ER	so_main(INT ac, UB* arg[]);
IMPORT	ER	so_fcntl(int sd, int cmd, ... /* arg */);
IMPORT	ER	so_ioctl(int sd, int request, ... /* arg */);

/* [BEGIN SYSCALLS] */
IMPORT	int	so_accept(int sd, struct sockaddr* addr, socklen_t *addrlen);
IMPORT	ER	so_bind(int sd, const struct sockaddr* addr, socklen_t addrlen);
IMPORT	ER	so_close(int sd);
IMPORT	ER	so_connect(int sd, const struct sockaddr* addr, socklen_t addrlen);
IMPORT	ER	so_getpeername(int sd, struct sockaddr* addr, socklen_t* addrlen);
IMPORT	ER	so_getsockname(int sd, struct sockaddr* addr, socklen_t* addrlen);
IMPORT	ER	so_getsockopt(int sd, int level, int optname, void* optval, socklen_t* optlen);
IMPORT	ER	so_setsockopt(int sd, int level, int optname, const void* optval, socklen_t optlen);
IMPORT	ER	so_listen(int sd, int backlog);
IMPORT	int	so_read(int sd, void* buf, size_t count);
IMPORT	int	so_recv(int sd, void* buf, size_t len, int flags);
IMPORT	int	so_recvfrom(int sd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen);
IMPORT	int	so_recvmsg(int sd, struct msghdr* msg, int flags);
IMPORT	int	so_select_us(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, TMO_U tmout_u);
IMPORT	int	so_select_ms(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, TMO tmout);
IMPORT	int	so_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* tv);
IMPORT	int	so_write(int sd, const void* buf, size_t count);
IMPORT	int	so_send(int sd, const void* buf, size_t len, int flags);
IMPORT	int	so_sendto(int sd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen);
IMPORT	int	so_sendmsg(int sd, const struct msghdr* msg, int flags);
IMPORT	ER	so_shutdown(int sd, int how);
IMPORT	int	so_socket(int domain, int type, int protocol);
IMPORT	ER	so_gethostname(char* name, size_t len);
IMPORT	ER	so_sethostname(const char* name, size_t len);
IMPORT	int	so_getaddrinfo_us(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res, void* buf, size_t bufsz, TMO_U tmout_u);
IMPORT	int	so_getaddrinfo_ms(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res, void* buf, size_t bufsz, TMO tmout);
IMPORT	int	so_getaddrinfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res, void* buf, size_t bufsz, struct timeval* timeout);
IMPORT	ER	so_getnameinfo_us(const struct sockaddr* sa, socklen_t salen, char* host, size_t hostlen, char* serv, size_t servlen, int flags, TMO_U tmout_u);
IMPORT	ER	so_getnameinfo_ms(const struct sockaddr* sa, socklen_t salen, char* host, size_t hostlen, char* serv, size_t servlen, int flags, TMO tmout);
IMPORT	ER	so_getnameinfo(const struct sockaddr* sa, socklen_t salen, char* host, size_t hostlen, char* serv, size_t servlen, int flags, struct timeval* timeout);
IMPORT	int	so_sockatmark(int sd);
IMPORT	ER	so_break(ID tskid);
IMPORT	ER	so_ifattach(const char* devnm);
IMPORT	ER	so_ifdetach(const char* devnm);
IMPORT	int	so_rtlist(int af, int cmd, int flags, void* buf, size_t bufsz);
IMPORT	int	so_resctl(int cmd, void* buf, size_t bufsz);
IMPORT	int	so_getifaddrs(struct ifaddrs** ifap, void* buf, size_t bufsz);
IMPORT	ER	so_ifindextoname(unsigned int ifindex, char* ifname);
IMPORT	int	so_ifnametoindex(const char* ifname);
IMPORT	int	so_bpfopen(const char* path, int oflag);
IMPORT	int	so_tunopen(const char* path, int oflag);

/* internal use */
IMPORT	ER	_so_fcntl(int sd, int cmd, void* arg);
IMPORT	ER	_so_ioctl(int sd, int request, void* data);
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif	/* __T2EX_SOCKET_H__ */

