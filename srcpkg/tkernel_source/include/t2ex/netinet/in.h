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
 *	@(#)in.h
 *
 */

#ifndef _NETINET_IN_H_
#define	_NETINET_IN_H_

#include <stdint.h>

typedef	uint8_t	sa_family_t;
typedef	uint16_t	in_port_t;
typedef	uint32_t	in_addr_t;

struct in_addr {
	in_addr_t s_addr;
};

struct sockaddr_in {
	uint8_t		sin_len;
	sa_family_t	sin_family;
	in_port_t	sin_port;
	struct in_addr	sin_addr;
	char	sin_zero[8];
};

#define	IPPROTO_IP	0
#define	IPPROTO_ICMP	1
#define	IPPROTO_TCP	6
#define	IPPROTO_UDP	17
#define	IPPROTO_RAW	255
#define	INADDR_ANY		((in_addr_t)0U)
#define	INADDR_LOOPBACK		((in_addr_t)0x7f000001U)
#define	INADDR_BROADCAST	((in_addr_t)0xffffffffU)
#define	INADDR_NONE		((in_addr_t)0xffffffffU)
#define	INET_ADDRSTRLEN	16

#define	IP_OPTIONS		1    /* buf/ip_opts; set/get IP options */
#define	IP_HDRINCL		2    /* int; header is included with data */

#endif /* _NETINET_IN_H_ */

