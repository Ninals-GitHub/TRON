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
 *	@(#)dhclient.c
 *
 */

#include <t2ex/socket.h>
#include <tk/tkernel.h>
#include <tk/errno.h>
#include <tk/devmgr.h>
#include <device/netdrv.h>

#include <strings.h>

#include "dhcp.h"
#include "route.h"
#include "util.h"

struct dhcp_state {
	uint8_t state;
	const char* ifname;
	int xid;
	SYSTIM start_time;
	size_t option_length;
	char hwaddr[6];
	uint32_t yiaddr;
	uint32_t dhcp_server;
	uint32_t mask;
	uint32_t gate;
	uint32_t bcaddr;
	uint32_t dns[2];
	char domain_name[256];
};

struct dhcp_packet {
	struct ether_header eth;
	struct ip ip;
	struct udphdr udphdr;
	struct dhcp_message dhcp;
} __packed;

static int init_receiver(void)
{
	int re;
	int sd;
	struct sockaddr_in s;

	sd = so_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	DEBUG_PRINT(("init_receiver: so_socket = %d(%d, %d)\n", sd, MERCD(sd), SERCD(sd)));

	bzero(&s, sizeof s);
	s.sin_len = sizeof s;
	s.sin_family = AF_INET;
	s.sin_addr.s_addr = htonl(INADDR_ANY);
	s.sin_port = htons(DHCP_CLIENT_PORT);
	re = so_bind(sd, (struct sockaddr*)&s, sizeof s);
	DEBUG_PRINT(("init_receiver: so_bind = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));

	return sd;
}

static void init_state(struct dhcp_state* state, const char* ifname)
{
	int re;

	bzero(state, sizeof *state);

	state->state = DHCP_DISCOVER;

	state->ifname = ifname;

	// ether address
	re = get_hwaddr(ifname, state->hwaddr);
	DEBUG_PRINT(("init_state: get_hwaddr = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));

	// generate transaction ID
	state->xid = net_random();

	// start time
	tk_get_tim(&state->start_time);
}

static int send_request(struct dhcp_state* state)
{
	struct dhcp_packet p;
	int i = 0;
	int j;
	int re;
	int fd;
	unsigned long sum;
	int udp_len;
	struct ifreq ifr;

	bzero(&ifr, sizeof ifr);
	strncpy(ifr.ifr_name, state->ifname, IFNAMSIZ-1);

	bzero(&p, sizeof p);

	for(j = 0; j < 6; j++) {
		p.eth.ether_dhost[j] = 0xffU;
		p.eth.ether_shost[j] = state->hwaddr[j];
	}
	p.eth.ether_type = htons(0x0800); // IPv4

	p.ip.ip_v = IPVERSION;
	p.ip.ip_hl = sizeof p.ip >> 2;
	p.ip.ip_id = state->xid;
	p.ip.ip_ttl = 255;
	p.ip.ip_p = IPPROTO_UDP;
	p.ip.ip_dst.s_addr = htonl(INADDR_BROADCAST);

	p.udphdr.uh_sport = htons(DHCP_CLIENT_PORT);
	p.udphdr.uh_dport = htons(DHCP_SERVER_PORT);

	p.dhcp.op = DHCP_BOOTREQUEST;
	p.dhcp.hwtype = 1;
	p.dhcp.hwlen = 6;
	p.dhcp.xid = state->xid;
	if ( state->state == DHCP_DISCOVER ) {
		tk_get_tim(&state->start_time);
	} else {
		SYSTIM tim;
		UW time;
		tk_get_tim(&tim);
		time = (tim.lo - state->start_time.lo) / 1000;
		p.dhcp.secs = htons(time > 65535 ? 65535 : time);
	}
	p.dhcp.flags = htons(BROADCAST_FLAG);
	p.dhcp.yiaddr = state->yiaddr;
	memcpy(p.dhcp.chaddr, state->hwaddr, 6);
	p.dhcp.cookie = htonl(MAGIC_COOKIE);

	p.dhcp.options[i] = DHO_MESSAGETYPE;
	i++;
	p.dhcp.options[i] = 1;
	i++;
	p.dhcp.options[i] = state->state;
	i++;
	if ( state->state == DHCP_REQUEST ) {
		p.dhcp.options[i] = DHO_IPADDRESS;
		i++;
		p.dhcp.options[i] = sizeof(uint32_t);
		i++;
		memcpy(&p.dhcp.options[i], &state->yiaddr, sizeof(uint32_t));
		i += sizeof(uint32_t);
	}
	p.dhcp.options[i] = DHO_END;
	i++;

	state->option_length = i;

	udp_len = sizeof p.udphdr + offsetof(struct dhcp_message, options) + state->option_length;
	p.udphdr.uh_ulen = htons(udp_len);
	sum = p.ip.ip_src.s_addr & 0xffffU;
	sum += (p.ip.ip_src.s_addr >> 16) & 0xffffU;
	sum += p.ip.ip_dst.s_addr & 0xffffU;
	sum += (p.ip.ip_dst.s_addr >> 16) & 0xffffU;
	sum += htons(IPPROTO_UDP);
	sum += htons(udp_len);
	p.udphdr.uh_sum = checksum2((unsigned short*)&p.udphdr, udp_len, sum);

	p.ip.ip_len = htons(sizeof p.ip + udp_len);
	p.ip.ip_sum = checksum((unsigned short*)&p.ip, sizeof p.ip);

	fd = so_bpfopen("/dev/bpf0", O_RDWR);
	DEBUG_PRINT(("send_request: so_bpfopen = %d(%d, %d)\n", fd, MERCD(fd), SERCD(fd)));
	re = so_ioctl(fd, BIOCSETIF, &ifr);
	DEBUG_PRINT(("send_request: so_ioctl = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	re = so_write(fd, &p, sizeof p.eth + sizeof p.ip + udp_len);
	DEBUG_PRINT(("send_request: so_write = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	so_close(fd);

	return re;
}

static int wait_response(int server, long tmo)
{
	fd_set fds;
	struct timeval tv;
	int re;

	FD_ZERO(&fds);
	FD_SET(server, &fds);
	tv.tv_sec = tmo;
	tv.tv_usec = 0;

	DEBUG_PRINT(("dhclient: pre-select for %d secs\n", tmo));
	re = so_select(server+1, &fds, NULL, NULL, &tv);
	DEBUG_PRINT(("dhclient: so_select = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));

	return re;
}

static void get_params(struct dhcp_message* offer, struct dhcp_state* state)
{
	int i;
	for(i = 0; offer->options[i] != DHO_END; i += offer->options[i+1] + 2) {
		switch(offer->options[i]) {
		case DHO_PAD:
			break;

		case DHO_SUBNETMASK:
			memcpy(&state->mask, &offer->options[i+2], 4);
			break;

		case DHO_ROUTER:
			memcpy(&state->gate, &offer->options[i+2], 4);
			break;

		case DHO_BROADCAST:
			memcpy(&state->bcaddr, &offer->options[i+2], 4);
			break;

		case DHO_SERVERID:
			memcpy(&state->dhcp_server, &offer->options[i+2], 4);
			break;

		case DHO_DNSSERVER:
			memcpy(&state->dns[0], &offer->options[i+2], offer->options[i+1]);
			break;

		case DHO_DNSDOMAIN:
			strncpy(state->domain_name, &offer->options[i+2], offer->options[i+1]);
			break;
		}
	}
}

static int get_response(int server, struct dhcp_state* state)
{
	struct dhcp_message offer;
	struct sockaddr_in ssa;
	socklen_t ssa_len;
	int re;

	bzero(&offer, sizeof offer);
	bzero(&ssa, sizeof ssa);
	ssa_len = sizeof ssa;

	re = so_recvfrom(server, &offer, sizeof offer, 0, (struct sockaddr*)&ssa, &ssa_len);
	DEBUG_PRINT(("dhclient: so_recvfrom = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		return re;
	}
	if ( offer.xid != state->xid ) {
		return 0;
	}
	if ( state->state == DHCP_OFFER ) {
		state->yiaddr = offer.yiaddr;
	} else {
		if ( offer.options[2] == DHCP_NAK ) {
			/*
			 * Start over, returning a packet size which is too large to receive.
			 */
			return INT32_MAX;
		}

		get_params(&offer, state);
	}

	return re;
}

static void setup(const struct dhcp_state* state, const char* ifname)
{
	int i;
	int re;
	struct sockaddr_in sa;
	struct in_addr a;
	int index = so_ifnametoindex(ifname);
#ifdef DEBUG
	char rbuf[18];
#endif

	set_ifaddr(ifname, state->yiaddr, state->mask);
	a.s_addr = state->yiaddr;
	DEBUG_PRINT(("%s: IP address = %s\n", ifname, inet_ntop(AF_INET, &a, rbuf, sizeof(rbuf))));
	route_add(INADDR_ANY, state->gate, INADDR_ANY, index, 0);

	bzero(&sa, sizeof sa);
	sa.sin_len = sizeof sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(53);
	re = so_resctl(SO_RES_DEL_SERVER, &sa, sizeof sa);
	DEBUG_PRINT(("setup: so_resctl(%s) = %d(%d, %d)\n", "delete", re, MERCD(re), SERCD(re)));

	for(i = 0; state->dns[i] != 0 && i < 2; i++ ) {
		bzero(&sa, sizeof sa);
		sa.sin_len = sizeof sa;
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = state->dns[i];
		sa.sin_port = htons(53);
		re = so_resctl(SO_RES_ADD_SERVER, &sa, sizeof sa);
		a.s_addr = state->dns[i];
		DEBUG_PRINT(("setup: so_resctl(%s) = %d(%d, %d)\n", inet_ntop(AF_INET, &a, rbuf, sizeof(rbuf)), re, MERCD(re), SERCD(re)));
	}

	so_resctl(SO_RES_ADD_DOMAIN, (void*)state->domain_name, strlen(state->domain_name) + 1);
}

int dhclient(const char* ifname)
{
	int re;
	int receiver;
	struct dhcp_state state;

	init_state(&state, ifname);
	receiver = init_receiver();

	long tmo = DHCP_BASE;
	while(state.state != 0) {
		switch(state.state) {
		case DHCP_DISCOVER:
		case DHCP_REQUEST:
			re = send_request(&state);
			DEBUG_PRINT(("dhclient: [REQUEST:%d] = %d(%d, %d)\n", state.state, re, MERCD(re), SERCD(re)));
			if ( re < 0 ) {
				return re;
			}
			state.state = (state.state == DHCP_DISCOVER) ? DHCP_OFFER : DHCP_ACK;
			break;

		case DHCP_OFFER:
		case DHCP_ACK:
			re = wait_response(receiver, tmo);
			if ( re == 0 ) {
				DEBUG_PRINT(("dhclient: select timed out\n"));
				// timed out
				tmo *= 2;
				tmo += net_random() % 3 - 1;
				if ( tmo > DHCP_MAX ) {
					tmo = DHCP_MAX;
				}
				state.state = (state.state == DHCP_OFFER) ? DHCP_DISCOVER : DHCP_REQUEST;
				continue;
			} else if ( re < 0 ) {
				return re;
			}

			re = get_response(receiver, &state);
			if ( re == 0 ) {
				// unrelated packet
				DEBUG_PRINT(("dhclient: unrelated packet\n"));
				continue;
			} else if ( re == INT32_MAX ) {
				state.state = DHCP_DISCOVER;
				continue;
			} else if ( re < 0 ) {
				return re;
			}
			tmo = DHCP_BASE;
			state.state = (state.state == DHCP_OFFER) ? DHCP_REQUEST : 0;
			break;
		}
	}

	so_close(receiver);

	setup(&state, ifname);

	return 0;
}
