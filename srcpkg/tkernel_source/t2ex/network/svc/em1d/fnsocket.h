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
 *	T2EX SVC function code
 *
 *	   (generated automatically)
 */

#include <t2ex/ssid.h>

#define NET_SO_ACCEPT_FN	(0x00010300 | NET_SVC)
#define NET_SO_BIND_FN	(0x00020300 | NET_SVC)
#define NET_SO_CLOSE_FN	(0x00030100 | NET_SVC)
#define NET_SO_CONNECT_FN	(0x00040300 | NET_SVC)
#define NET_SO_GETPEERNAME_FN	(0x00050300 | NET_SVC)
#define NET_SO_GETSOCKNAME_FN	(0x00060300 | NET_SVC)
#define NET_SO_GETSOCKOPT_FN	(0x00070500 | NET_SVC)
#define NET_SO_SETSOCKOPT_FN	(0x00080500 | NET_SVC)
#define NET_SO_LISTEN_FN	(0x00090200 | NET_SVC)
#define NET_SO_READ_FN	(0x000a0300 | NET_SVC)
#define NET_SO_RECV_FN	(0x000b0400 | NET_SVC)
#define NET_SO_RECVFROM_FN	(0x000c0600 | NET_SVC)
#define NET_SO_RECVMSG_FN	(0x000d0300 | NET_SVC)
#define NET_SO_SELECT_US_FN	(0x000e0500 | NET_SVC)
#define NET_SO_SELECT_MS_FN	(0x000f0500 | NET_SVC)
#define NET_SO_SELECT_FN	(0x00100500 | NET_SVC)
#define NET_SO_WRITE_FN	(0x00110300 | NET_SVC)
#define NET_SO_SEND_FN	(0x00120400 | NET_SVC)
#define NET_SO_SENDTO_FN	(0x00130600 | NET_SVC)
#define NET_SO_SENDMSG_FN	(0x00140300 | NET_SVC)
#define NET_SO_SHUTDOWN_FN	(0x00150200 | NET_SVC)
#define NET_SO_SOCKET_FN	(0x00160300 | NET_SVC)
#define NET_SO_GETHOSTNAME_FN	(0x00170200 | NET_SVC)
#define NET_SO_SETHOSTNAME_FN	(0x00180200 | NET_SVC)
#define NET_SO_GETADDRINFO_US_FN	(0x00190700 | NET_SVC)
#define NET_SO_GETADDRINFO_MS_FN	(0x001a0700 | NET_SVC)
#define NET_SO_GETADDRINFO_FN	(0x001b0700 | NET_SVC)
#define NET_SO_GETNAMEINFO_US_FN	(0x001c0800 | NET_SVC)
#define NET_SO_GETNAMEINFO_MS_FN	(0x001d0800 | NET_SVC)
#define NET_SO_GETNAMEINFO_FN	(0x001e0800 | NET_SVC)
#define NET_SO_SOCKATMARK_FN	(0x001f0100 | NET_SVC)
#define NET_SO_BREAK_FN	(0x00200100 | NET_SVC)
#define NET_SO_IFATTACH_FN	(0x00210100 | NET_SVC)
#define NET_SO_IFDETACH_FN	(0x00220100 | NET_SVC)
#define NET_SO_RTLIST_FN	(0x00230500 | NET_SVC)
#define NET_SO_RESCTL_FN	(0x00240300 | NET_SVC)
#define NET_SO_GETIFADDRS_FN	(0x00250300 | NET_SVC)
#define NET_SO_IFINDEXTONAME_FN	(0x00260200 | NET_SVC)
#define NET_SO_IFNAMETOINDEX_FN	(0x00270100 | NET_SVC)
#define NET_SO_BPFOPEN_FN	(0x00280200 | NET_SVC)
#define NET_SO_TUNOPEN_FN	(0x00290200 | NET_SVC)
#define NET__SO_FCNTL_FN	(0x002a0300 | NET_SVC)
#define NET__SO_IOCTL_FN	(0x002b0300 | NET_SVC)

