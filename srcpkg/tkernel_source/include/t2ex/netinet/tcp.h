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
 *	@(#)tcp.h
 *
 */

#ifndef __NETINET_TCP_H__
#define	__NETINET_TCP_H__

#include <basic.h>
#include <stdint.h>
#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t tcp_seq;
/*
 * TCP header.
 * Per RFC 793, September, 1981.
 * Updated by RFC 3168, September, 2001.
 */
struct tcphdr {
	uint16_t	th_sport;	/* source port */
	uint16_t	th_dport;	/* destination port */
	tcp_seq		th_seq;		/* sequence number */
	tcp_seq		th_ack;		/* acknowledgement number */
#if BIGENDIAN
	/*LINTED non-portable bitfields*/
	uint8_t		th_off:4,	/* data offset */
			th_x2:4;	/* (unused) */
#else
	/*LINTED non-portable bitfields*/
	uint8_t		th_x2:4,	/* (unused) */
			th_off:4;	/* data offset */
#endif
	uint8_t		th_flags;
#define	TH_FIN	  0x01
#define	TH_SYN	  0x02
#define	TH_RST	  0x04
#define	TH_PUSH	  0x08
#define	TH_ACK	  0x10
#define	TH_URG	  0x20
#define	TH_ECE	  0x40
#define	TH_CWR	  0x80
	uint16_t	th_win;		/* window */
	uint16_t	th_sum;		/* checksum */
	uint16_t	th_urp;		/* urgent pointer */
} __packed;

#define	TCP_NODELAY	1	/* don't delay send to coalesce packets */
#define	TCP_MAXSEG	2	/* set maximum segment size */

#ifdef __cplusplus
}
#endif
#endif /* __NETINET_TCP_H__ */
