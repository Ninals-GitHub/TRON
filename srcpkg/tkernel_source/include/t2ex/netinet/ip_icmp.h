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
 *	@(#)ip_icmp.h
 *
 */

#ifndef __NETINET_IP_ICMP_H__
#define	__NETINET_IP_ICMP_H__

#include <basic.h>
#include <stdint.h>
#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t n_short;
typedef uint32_t n_time;

/*
 * Internal of an ICMP Router Advertisement
 */
struct icmp_ra_addr {
	uint32_t ira_addr;
	uint32_t ira_preference;
} __packed;

/*
 * Structure of an icmp header.
 */
struct icmp {
	uint8_t  icmp_type;		/* type of message, see below */
	uint8_t  icmp_code;		/* type sub code */
	uint16_t icmp_cksum;		/* ones complement cksum of struct */
	union {
		uint8_t  ih_pptr;		/* ICMP_PARAMPROB */
		struct in_addr ih_gwaddr;	/* ICMP_REDIRECT */
		struct ih_idseq {
			  n_short icd_id;
			  n_short icd_seq;
		} ih_idseq __packed;
		int32_t   ih_void;

		/* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
		struct ih_pmtu {
			  n_short ipm_void;
			  n_short ipm_nextmtu;
		} ih_pmtu __packed;
		struct ih_rtradv {
			uint8_t irt_num_addrs;
			uint8_t irt_wpa;
			uint16_t irt_lifetime;
		} ih_rtradv __packed;
	} icmp_hun /* XXX __packed ??? */;
#define	icmp_pptr	  icmp_hun.ih_pptr
#define	icmp_gwaddr	  icmp_hun.ih_gwaddr
#define	icmp_id		  icmp_hun.ih_idseq.icd_id
#define	icmp_seq	  icmp_hun.ih_idseq.icd_seq
#define	icmp_void	  icmp_hun.ih_void
#define	icmp_pmvoid	  icmp_hun.ih_pmtu.ipm_void
#define	icmp_nextmtu	  icmp_hun.ih_pmtu.ipm_nextmtu
#define icmp_num_addrs    icmp_hun.ih_rtradv.irt_num_addrs
#define icmp_wpa	  icmp_hun.ih_rtradv.irt_wpa
#define icmp_lifetime	  icmp_hun.ih_rtradv.irt_lifetime
	union {
		struct id_ts {
			  n_time its_otime;
			  n_time its_rtime;
			  n_time its_ttime;
		} id_ts __packed;
		struct id_ip  {
			  struct ip idi_ip;
			  /* options and then 64 bits of data */
		} id_ip /* XXX: __packed ??? */;
		struct icmp_ra_addr id_radv;
		uint32_t id_mask;
		int8_t	  id_data[1];
	} icmp_dun /* XXX __packed ??? */;
#define	icmp_otime	  icmp_dun.id_ts.its_otime
#define	icmp_rtime	  icmp_dun.id_ts.its_rtime
#define	icmp_ttime	  icmp_dun.id_ts.its_ttime
#define	icmp_ip		  icmp_dun.id_ip.idi_ip
#define icmp_radv	  icmp_dun.id_mask
#define	icmp_mask	  icmp_dun.id_mask
#define	icmp_data	  icmp_dun.id_data
};

/*
 * Definition of type and code field values.
 */
#define	ICMP_ECHOREPLY			0	/* echo reply */

#define	ICMP_UNREACH			3	/* dest unreachable, codes: */
#define	ICMP_UNREACH_NET		0	/* bad net */
#define	ICMP_UNREACH_HOST		1	/* bad host */
#define	ICMP_UNREACH_PROTOCOL		2	/* bad protocol */
#define	ICMP_UNREACH_PORT		3	/* bad port */
#define	ICMP_UNREACH_NEEDFRAG		4	/* IP_DF caused drop */
#define	ICMP_UNREACH_SRCFAIL		5	/* src route failed */
#define	ICMP_UNREACH_NET_UNKNOWN	6	/* unknown net */
#define	ICMP_UNREACH_HOST_UNKNOWN	7	/* unknown host */
#define	ICMP_UNREACH_ISOLATED		8	/* src host isolated */
#define	ICMP_UNREACH_NET_PROHIB		9	/* prohibited access */
#define	ICMP_UNREACH_HOST_PROHIB	10	/* ditto */
#define	ICMP_UNREACH_TOSNET		11	/* bad tos for net */
#define	ICMP_UNREACH_TOSHOST		12	/* bad tos for host */
#define	ICMP_UNREACH_ADMIN_PROHIBIT	13	/* communication
						   administratively
						   prohibited */

#define	ICMP_SOURCEQUENCH	4		/* packet lost, slow down */
#define	ICMP_REDIRECT		5		/* shorter route, codes: */

#define	ICMP_REDIRECT_NET	0		/* for network */
#define	ICMP_REDIRECT_HOST	1		/* for host */
#define	ICMP_REDIRECT_TOSNET	2		/* for tos and net */
#define	ICMP_REDIRECT_TOSHOST	3		/* for tos and host */

#define	ICMP_ECHO		8		/* echo service */
#define	ICMP_ROUTERADVERT	9		/* router advertisement */
#define	ICMP_ROUTERSOLICIT	10		/* router solicitation */
#define	ICMP_TIMXCEED		11		/* time exceeded, code: */

#define	ICMP_TIMXCEED_INTRANS	0		/* ttl==0 in transit */
#define	ICMP_TIMXCEED_REASS	1		/* ttl==0 in reass */

#define	ICMP_PARAMPROB		12		/* ip header bad */

#define	ICMP_PARAMPROB_OPTABSENT 1		/* req. opt. absent */

#define	ICMP_TSTAMP		13		/* timestamp request */
#define	ICMP_TSTAMPREPLY	14		/* timestamp reply */
#define	ICMP_IREQ		15		/* information request */
#define	ICMP_IREQREPLY		16		/* information reply */
#define	ICMP_MASKREQ		17		/* address mask request */
#define	ICMP_MASKREPLY		18		/* address mask reply */

#define	ICMP_MAXTYPE		18

#ifdef __cplusplus
}
#endif
#endif /* __NETINET_IP_ICMP_H__ */
