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
 *	@(#)util.c
 *
 */

#include <tk/tkernel.h>

#include <t2ex/socket.h>
#include <device/netdrv.h>

#include <strings.h>

#include "util.h"

int net_random(void)
{
	SYSTIM tim;
	tk_get_tim(&tim);
	return tim.hi ^ tim.lo ^ 0x21274a1dU;
}

unsigned short checksum2(unsigned short *buf, int bufsz, unsigned long initial)
{
	unsigned long sum = initial;

	while (bufsz > 1) {
		sum += *buf;
		buf++;
		bufsz -= 2;
	}

	if (bufsz == 1) {
		sum += *(unsigned char *)buf;
	}

	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

unsigned short checksum(unsigned short *buf, int bufsz)
{
	return checksum2(buf, bufsz, 0);
}

int get_hwaddr(const char* ifname, char* buf)
{
	struct ifaddrs* ifa_list;
	struct ifaddrs* ifa;
	struct sockaddr_dl* dl;
	ER re;
	char b[2048];

	re = so_getifaddrs(&ifa_list, b, sizeof b);
	if ( re < 0 ) {
		return re;
	}
	for(ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
        dl = (struct sockaddr_dl*)ifa->ifa_addr;
        if (dl->sdl_family == AF_LINK && dl->sdl_type == IFT_ETHER) {
            if ( strncmp(ifname, dl->sdl_data, dl->sdl_nlen) == 0 ) {
            	memcpy(buf, LLADDR(dl), 6);
		return 0;
            }
        }
	}

	return EX_NOENT;
}

int if_updown(const char* ifname, int is_up)
{
	int re;
	int sd;
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

	sd = so_socket(AF_INET, SOCK_DGRAM, 0);

	re = so_ioctl(sd, SIOCGIFFLAGS, &ifr);
	DEBUG_PRINT(("if_updown: so_ioctl = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		so_close(sd);
		return re;
	}

	if ( is_up ) {
		ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
	} else {
		ifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
	}

	re = so_ioctl(sd, SIOCSIFFLAGS, &ifr);
	DEBUG_PRINT(("if_updown: so_ioctl = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		so_close(sd);
		return re;
	}

	so_close(sd);

	return 0;
}

void set_ifaddr(const char* ifname, in_addr_t addr, in_addr_t mask)
{
	int sd;
	int iore;
	struct ifreq ifr;
	struct sockaddr_in* p_iosa;

	sd = so_socket(AF_INET, SOCK_DGRAM, 0);
	DEBUG_PRINT(("set_ifaddr: so_socket = %d(%d, %d)\n", sd, MERCD(sd), SERCD(sd)));

	bzero(&ifr, sizeof ifr);
	p_iosa = (struct sockaddr_in*)&ifr.ifr_addr;
	p_iosa->sin_len = sizeof *p_iosa;
	p_iosa->sin_family = AF_INET;
	p_iosa->sin_addr.s_addr = addr;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
	iore = so_ioctl(sd, SIOCSIFADDR, &ifr);
	DEBUG_PRINT(("set_ifaddr: so_ioctl = %d(%d, %d)\n", iore, MERCD(iore), SERCD(iore)));

	if ( mask ) {
		bzero(&ifr, sizeof ifr);
		p_iosa = (struct sockaddr_in*)&ifr.ifr_addr;
		p_iosa->sin_len = sizeof *p_iosa;
		p_iosa->sin_family = AF_INET;
		p_iosa->sin_addr.s_addr = mask;
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
		iore = so_ioctl(sd, SIOCSIFNETMASK, &ifr);
		DEBUG_PRINT(("set_ifaddr: so_ioctl = %d(%d, %d)\n", iore, MERCD(iore), SERCD(iore)));
	}

	so_close(sd);
}

void add_hosttable(const char* hostname, in_addr_t addr)
{
	int re;
	struct hosttable ht;
	struct sockaddr_in sa;

	bzero(&sa, sizeof sa);
	sa.sin_len = sizeof sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = addr;
	bzero(&ht, sizeof ht);
	ht.addr = (struct sockaddr*)&sa;
	ht.host = (char*)hostname;

	re = so_resctl(SO_RES_ADD_TABLE, &ht, sizeof ht);
	DEBUG_PRINT(("so_resctl(localhost) = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
}

in_addr_t resolv_host(const char* host)
{
	int re;
	struct addrinfo hints;
	struct addrinfo* res;
	char buf[512];

	bzero(&hints, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	re = so_getaddrinfo(host, NULL, &hints, &res, buf, sizeof buf, NULL);
	DEBUG_PRINT(("resolv_host: so_getaddrinfo = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		return INADDR_NONE;
	}

	if ( res != NULL ) {
		return ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
	}

	return INADDR_NONE;
}

void netdrv_set_minpktsz(const char* ifname, W minsz)
{
	ID dd;
	W len;
	NetRxBufSz bufsz;
	ER ercd;

	dd = tk_opn_dev((UB*)ifname, TD_UPDATE);
	if (dd < 0) {
		return;
	}

	ercd = tk_srea_dev(dd, DN_NETRXBUFSZ, &bufsz, sizeof(bufsz), &len);
	if (ercd == E_OK) {
		bufsz.minsz = minsz;
		tk_swri_dev(dd, DN_NETRXBUFSZ, &bufsz, sizeof(bufsz), &len);
	}

	tk_cls_dev(dd, 0);
}
