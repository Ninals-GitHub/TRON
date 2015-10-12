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
 *	@(#)tkn_syscall.h
 *
 */

/*
 * System call numbers.
 */

#ifndef __SYS_TKN_SYSCALL_H__
#define __SYS_TKN_SYSCALL_H__

#include "fnsocket.h"

#define	SYS_NO(x)	( (x) >> 16 )

enum sys_names {
	SYS___socket30		= SYS_NO(NET_SO_SOCKET_FN),
	SYS_listen		= SYS_NO(NET_SO_LISTEN_FN),
	SYS_accept		= SYS_NO(NET_SO_ACCEPT_FN),
	SYS_bind		= SYS_NO(NET_SO_BIND_FN),
	SYS_connect		= SYS_NO(NET_SO_CONNECT_FN),
	SYS_select		= SYS_NO(NET_SO_SELECT_FN),
	SYS_select_us		= SYS_NO(NET_SO_SELECT_US_FN),
	SYS_select_ms		= SYS_NO(NET_SO_SELECT_MS_FN),
	compat_43_SYS_recv	= SYS_NO(NET_SO_RECV_FN),
	SYS_recvfrom		= SYS_NO(NET_SO_RECVFROM_FN),
	SYS_recvmsg		= SYS_NO(NET_SO_RECVMSG_FN),
	compat_43_SYS_send	= SYS_NO(NET_SO_SEND_FN),
	SYS_sendto		= SYS_NO(NET_SO_SENDTO_FN),
	SYS_sendmsg		= SYS_NO(NET_SO_SENDMSG_FN),
	SYS_close		= SYS_NO(NET_SO_CLOSE_FN),
	SYS_shutdown		= SYS_NO(NET_SO_SHUTDOWN_FN),
	SYS_getpeername		= SYS_NO(NET_SO_GETPEERNAME_FN),
	SYS_getsockname		= SYS_NO(NET_SO_GETSOCKNAME_FN),
	SYS_setsockopt		= SYS_NO(NET_SO_SETSOCKOPT_FN),
	SYS_getsockopt		= SYS_NO(NET_SO_GETSOCKOPT_FN),
	SYS_ioctl		= SYS_NO(NET__SO_IOCTL_FN),
	SYS_read		= SYS_NO(NET_SO_READ_FN),
	SYS_write		= SYS_NO(NET_SO_WRITE_FN),
	SYS_fcntl		= SYS_NO(NET__SO_FCNTL_FN),
	SYS_gethostname		= SYS_NO(NET_SO_GETHOSTNAME_FN),
	SYS_sethostname		= SYS_NO(NET_SO_SETHOSTNAME_FN),
	SYS_getaddrinfo		= SYS_NO(NET_SO_GETADDRINFO_FN),
	SYS_getaddrinfo_ms	= SYS_NO(NET_SO_GETADDRINFO_MS_FN),
	SYS_getaddrinfo_us	= SYS_NO(NET_SO_GETADDRINFO_US_FN),
	SYS_getnameinfo		= SYS_NO(NET_SO_GETNAMEINFO_FN),
	SYS_getnameinfo_ms	= SYS_NO(NET_SO_GETNAMEINFO_MS_FN),
	SYS_getnameinfo_us	= SYS_NO(NET_SO_GETNAMEINFO_US_FN),
	SYS_sockatmark		= SYS_NO(NET_SO_SOCKATMARK_FN),
	SYS_break		= SYS_NO(NET_SO_BREAK_FN),
	SYS_ifattach		= SYS_NO(NET_SO_IFATTACH_FN),
	SYS_ifdetach		= SYS_NO(NET_SO_IFDETACH_FN),
	SYS_rtlist		= SYS_NO(NET_SO_RTLIST_FN),
	SYS_resctl		= SYS_NO(NET_SO_RESCTL_FN),
	SYS_getifaddrs		= SYS_NO(NET_SO_GETIFADDRS_FN),
	SYS_ifindextoname	= SYS_NO(NET_SO_IFINDEXTONAME_FN),
	SYS_ifnametoindex	= SYS_NO(NET_SO_IFNAMETOINDEX_FN),
	SYS_bpfopen		= SYS_NO(NET_SO_BPFOPEN_FN),
	SYS_tunopen		= SYS_NO(NET_SO_TUNOPEN_FN),
};

#endif /* __SYS_TKN_SYSCALL_H__ */
