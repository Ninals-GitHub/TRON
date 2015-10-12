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
 *	@(#)ping.c
 *
 */

#include <t2ex/socket.h>
#include <tk/tkernel.h>

#include <stdio.h>
#include <strings.h>

#include "util.h"


struct icmp_state {
	short xid;
	int seq;
	SYSTIM start_time;
};

struct icmp_packet {
	struct ip ip;
	struct icmp icmp;
};

static void init_icmp_state(struct icmp_state* state)
{
	bzero(state, sizeof *state);

	state->xid = (short)net_random();
}

static int recv_icmp_echo_reply(int sd, struct icmp_state* state, int hop)
{
	struct icmp_packet* p;
	char buf[2048];
	int re;
	int i;
	fd_set fds;
	struct timeval tv;
	SYSTIM tim;
	int type;
	char rbuf[18];

	bzero(buf, sizeof buf);
	FD_ZERO(&fds);
	FD_SET(sd, &fds);

	tv.tv_sec = hop == 0 ? 1 : 3;
	tv.tv_usec = 0;

	for(i = 0; i < 3; i++) {
		re = so_select(sd+1, &fds, NULL, NULL, &tv);
		DEBUG_PRINT(("recv_icmp_echo_reply: so_select = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
		if ( re < 0 ) {
			return re;
		} else if ( re == 0 ) {
			// timed out
		} else {
			re = so_recv(sd, buf, sizeof buf, 0);
			DEBUG_PRINT(("recv_icmp_echo_reply: so_recv = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));

			p = (struct icmp_packet*)buf;

			type = p->icmp.icmp_type;
			if ( (type != ICMP_ECHOREPLY && type != ICMP_TIMXCEED) || (p->icmp.icmp_type == ICMP_ECHOREPLY && (p->icmp.icmp_id != state->xid || p->icmp.icmp_seq != state->seq ) ) ) {
				DEBUG_PRINT(("recv_icmp_echo_reply: type=%d, id=%x:%x, seq=%d:%d\n", type, p->icmp.icmp_id, state->xid, p->icmp.icmp_seq, state->seq));
				break;
			}

			tk_get_tim(&tim);
			if ( hop == 0 ) {
				printf("ICMP reply from %s: %d bytes %d ms TTL=%d\n", inet_ntop(AF_INET, &p->ip.ip_src, rbuf, sizeof(rbuf)), re, tim.lo - state->start_time.lo, p->ip.ip_ttl);
			} else {
				printf(" %2d  %4d ms %s\n", hop, tim.lo - state->start_time.lo, inet_ntop(AF_INET, &p->ip.ip_src, rbuf, sizeof(rbuf)));
			}
			if ( type == ICMP_ECHOREPLY || type == ICMP_TIMXCEED ) {
				return type;
			}
		}
	}

	if ( i == 5 ) {
		return -1;
	}

	return 0;
}

static int send_icmp_echo_request(int sd, in_addr_t addr, struct icmp_state* state, int ttl)
{
	struct icmp_packet p;
	struct sockaddr_in sa;
	int re;

	bzero(&p, sizeof p);

	p.ip.ip_v = IPVERSION;
	p.ip.ip_hl = sizeof p.ip >> 2;
	p.ip.ip_len = sizeof p;
	p.ip.ip_id = state->xid;
	p.ip.ip_ttl = ttl != 0 ? ttl : 255;
	p.ip.ip_p = IPPROTO_ICMP;
	p.ip.ip_dst.s_addr = addr;

	p.icmp.icmp_type = ICMP_ECHO;
	p.icmp.icmp_id = state->xid;
	p.icmp.icmp_seq = ++state->seq;
	p.icmp.icmp_cksum = checksum((unsigned short*)&p.icmp, sizeof p.icmp);

	bzero(&sa, sizeof sa);
	sa.sin_len = sizeof sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = addr;

	tk_get_tim(&state->start_time);

	printf("sending ICMP packet: id=%x, seq=%d\n", state->xid, state->seq);
	re = so_sendto(sd, &p, sizeof p, 0, (struct sockaddr*)&sa, sizeof sa);
	DEBUG_PRINT(("send_icmp_echo_request: so_sendto = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));

	return re;
}

void ping(in_addr_t addr)
{
	struct icmp_state state;
	int sender, receiver;
	int re;
	int on = 1;

	init_icmp_state(&state);

	sender = so_socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	DEBUG_PRINT(("ping: so_socket = %d(%d, %d)\n", sender, MERCD(sender), SERCD(sender)));
	receiver = so_socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	DEBUG_PRINT(("ping: so_socket = %d(%d, %d)\n", receiver, MERCD(receiver), SERCD(receiver)));

	re = so_setsockopt(sender, IPPROTO_IP, IP_HDRINCL, &on, sizeof on);
	DEBUG_PRINT(("ping: so_setsockopt = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));

	send_icmp_echo_request(sender, addr, &state, 0);
	recv_icmp_echo_reply(receiver, &state, 0);

	so_close(sender);
	so_close(receiver);
}

void traceroute(in_addr_t addr)
{
	struct icmp_state state;
	int sender, receiver;
	int hop;
	int re;
	int on = 1;
	struct in_addr a;
	char rbuf[18];

	init_icmp_state(&state);

	sender = so_socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	DEBUG_PRINT(("traceroute: so_socket = %d(%d, %d)\n", sender, MERCD(sender), SERCD(sender)));
	receiver = so_socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	DEBUG_PRINT(("traceroute: so_socket = %d(%d, %d)\n", receiver, MERCD(receiver), SERCD(receiver)));

	re = so_setsockopt(sender, IPPROTO_IP, IP_HDRINCL, &on, sizeof on);
	DEBUG_PRINT(("traceroute: so_setsockopt = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));

	a.s_addr = addr;
	printf("traceroute to %s, 30 hops max\n", inet_ntop(AF_INET, &a, rbuf, sizeof(rbuf)));
	for(hop = 1; hop <= 30; hop++) {
		if ( hop != 1 ) {
			tk_dly_tsk(1000);
		}
		send_icmp_echo_request(sender, addr, &state, hop);
		re = recv_icmp_echo_reply(receiver, &state, hop);
		if ( re == ICMP_ECHOREPLY ) {
			break;
		}
	}

	so_close(sender);
	so_close(receiver);
}
