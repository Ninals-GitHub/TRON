/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */

#include <machine.h>
#include <sys/svc/svc.h>
#include "fnsocket.h"

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
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:so_accept - _so_ioctl
 Input		:void
 Output		:void
 Return		:void
 Description	:svc for network subsystem
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
define_svc_arg3 so_accept		NET_SO_ACCEPT_FN
define_svc_arg3 so_bind			NET_SO_BIND_FN
define_svc_arg1 so_close		NET_SO_CLOSE_FN
define_svc_arg3 so_connect		NET_SO_CONNECT_FN
define_svc_arg3 so_getpeername		NET_SO_GETPEERNAME_FN
define_svc_arg3 so_getsockname		NET_SO_GETSOCKNAME_FN
define_svc_arg5 so_getsockopt		NET_SO_GETSOCKOPT_FN
define_svc_arg5 so_setsockopt		NET_SO_SETSOCKOPT_FN
define_svc_arg2 so_listen		NET_SO_LISTEN_FN
define_svc_arg3 so_read			NET_SO_READ_FN
define_svc_arg4 so_recv			NET_SO_RECV_FN
define_svc_arg6 so_recvfrom		NET_SO_RECVFROM_FN
define_svc_arg3 so_recvmsg		NET_SO_RECVMSG_FN
define_svc_arg5 so_select_us		NET_SO_SELECT_US_FN
define_svc_arg5 so_select_ms		NET_SO_SELECT_MS_FN
define_svc_arg5 so_select		NET_SO_SELECT_FN
define_svc_arg3 so_write		NET_SO_WRITE_FN
define_svc_arg4 so_send			NET_SO_SEND_FN
define_svc_arg6 so_sendto		NET_SO_SENDTO_FN
define_svc_arg3 so_sendmsg		NET_SO_SENDMSG_FN
define_svc_arg2 so_shutdown		NET_SO_SHUTDOWN_FN
define_svc_arg3 so_socket		NET_SO_SOCKET_FN
define_svc_arg2 so_gethostname		NET_SO_GETHOSTNAME_FN
define_svc_arg2 so_sethostname		NET_SO_SETHOSTNAME_FN
define_svc_arg7 so_getaddrinfo_us	NET_SO_GETADDRINFO_US_FN
define_svc_arg7 so_getaddrinfo_ms	NET_SO_GETADDRINFO_MS_FN
define_svc_arg7 so_getaddrinfo		NET_SO_GETADDRINFO_FN
define_svc_arg8 so_getnameinfo_us	NET_SO_GETNAMEINFO_US_FN
define_svc_arg8 so_getnameinfo_ms	NET_SO_GETNAMEINFO_MS_FN
define_svc_arg8 so_getnameinfo		NET_SO_GETNAMEINFO_FN
define_svc_arg1 so_sockatmark		NET_SO_SOCKATMARK_FN
define_svc_arg1 so_break		NET_SO_BREAK_FN
define_svc_arg1 so_ifattach		NET_SO_IFATTACH_FN
define_svc_arg1 so_ifdetach		NET_SO_IFDETACH_FN
define_svc_arg5 so_rtlist		NET_SO_RTLIST_FN
define_svc_arg3 so_resctl		NET_SO_RESCTL_FN
define_svc_arg3 so_getifaddrs		NET_SO_GETIFADDRS_FN
define_svc_arg2 so_ifindextoname	NET_SO_IFINDEXTONAME_FN
define_svc_arg1 so_ifnametoindex	NET_SO_IFNAMETOINDEX_FN
define_svc_arg2 so_bpfopen		NET_SO_BPFOPEN_FN
define_svc_arg2 so_tunopen		NET_SO_TUNOPEN_FN
define_svc_arg2 _so_fcntl		NET__SO_FCNTL_FN
define_svc_arg3 _so_ioctl		NET__SO_IOCTL_FN


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
