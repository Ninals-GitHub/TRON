/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2014/07/31.
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
 *	@(#)route.c
 *
 */

#include <tk/tkernel.h>

#include <t2ex/socket.h>

#include <stdio.h>
#include <strings.h>

#include "util.h"

static void route_cmd(int cmd, in_addr_t dest, in_addr_t gate, in_addr_t mask, int index, int direct)
{
	int i = 0;
	int sock;
	int re;
	struct {
		struct rt_msghdr rtm;
		struct sockaddr_in addr[3];
	} buf;

	bzero(&buf, sizeof buf);
	buf.rtm.rtm_version = RTM_VERSION;
	buf.rtm.rtm_type    = cmd;
	buf.rtm.rtm_index   = index;
	buf.rtm.rtm_flags   = RTF_STATIC;
	if ( direct ) {
		buf.rtm.rtm_flags |= RTF_HOST;
	} else {
		buf.rtm.rtm_flags |= RTF_GATEWAY;
	}
	buf.rtm.rtm_addrs   = RTA_DST | RTA_NETMASK;
	if ( gate != 0 ) {
		buf.rtm.rtm_addrs |= RTA_GATEWAY;
	}
	buf.rtm.rtm_inits   = RTV_HOPCOUNT;
	buf.rtm.rtm_rmx.rmx_hopcount = 1;
	buf.rtm.rtm_seq	    = 1;

	buf.addr[i].sin_len = sizeof(struct sockaddr_in);
	buf.addr[i].sin_family = AF_INET;
	buf.addr[i].sin_addr.s_addr = dest;
	i++;

	if ( gate != 0 ) {
		buf.addr[i].sin_len = sizeof(struct sockaddr_in);
		buf.addr[i].sin_family = AF_INET;
		buf.addr[i].sin_addr.s_addr = gate;
		i++;
	}

	buf.addr[i].sin_len = sizeof(struct sockaddr_in);
	buf.addr[i].sin_family = AF_INET;
	buf.addr[i].sin_addr.s_addr = mask;
	i++;

	buf.rtm.rtm_msglen = sizeof(struct rt_msghdr) + sizeof(struct sockaddr) * i;

	sock = so_socket(AF_ROUTE, SOCK_RAW, 0);
	DEBUG_PRINT(("route_add: so_socket = %d(%d, %d)\n", sock, MERCD(sock), SERCD(sock)));
	so_shutdown(sock, SHUT_RD);
	re = so_write(sock, &buf, buf.rtm.rtm_msglen);
	DEBUG_PRINT(("route_add: so_write = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	so_close(sock);
}

void route_change(in_addr_t dest, in_addr_t gate, in_addr_t mask, int index, int direct)
{
	route_cmd(RTM_CHANGE, dest, gate, mask, index, direct);
}

void route_delete(in_addr_t dest, in_addr_t mask, int index, int direct)
{
	route_cmd(RTM_DELETE, dest, 0, mask, index, direct);
}

void route_add(in_addr_t dest, in_addr_t gate, in_addr_t mask, int index, int direct)
{
	route_cmd(RTM_ADD, dest, gate, mask, index, direct);
}

#define ROUNDUP(a) \
	((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

static void get_rtaddrs(int addrs, struct sockaddr* sa, struct sockaddr** rti_info)
{
	int i;

    for (i = 0; i < RTAX_MAX; i++) {
		if ( addrs & (1 << i) ) {
			rti_info[i] = sa;
			sa = (struct sockaddr*)((char*)sa + ROUNDUP(sa->sa_len));
		} else {
			rti_info[i] = NULL;
		}
    }
}

static void np_rtentry(struct rt_msghdr* rtm)
{
    char ifname[IFNAMSIZ + 1];
    struct sockaddr *rti_info[RTAX_MAX];
    struct sockaddr_in* dest = NULL;
    struct sockaddr_in* mask = NULL;
	struct sockaddr* sa;
    char flags[33];
    char* bp = flags;
    char rbuf[18];
	char workbuf[20];
	int len;

	sa = (struct sockaddr*)(rtm + 1);
    if (sa->sa_family != AF_INET)
    	return;

    if (rtm->rtm_flags & RTF_UP)
    	*bp++ = 'U';
    if (rtm->rtm_flags & RTF_GATEWAY)
		*bp++ = 'G';
    if (rtm->rtm_flags & RTF_HOST)
		*bp++ = 'H';
    if (rtm->rtm_flags & RTF_STATIC)
		*bp++ = 'S';
    if (bp == flags)
		return;
    *bp = '\0';

    get_rtaddrs(rtm->rtm_addrs, sa, rti_info);

    if ((rtm->rtm_addrs & RTA_DST) &&
		    rti_info[RTAX_DST]->sa_family == AF_INET) {
		dest = (struct sockaddr_in*)rti_info[RTAX_DST];
    }

    if ((rtm->rtm_addrs & RTA_NETMASK) &&
            rti_info[RTAX_NETMASK]->sa_len != 0) {
        mask = (struct sockaddr_in*)rti_info[RTAX_NETMASK];
    }

	memset(workbuf, 0, sizeof(workbuf));
    if (rtm->rtm_addrs & RTA_GATEWAY) {
		len = so_getnameinfo(rti_info[RTAX_GATEWAY], rti_info[RTAX_GATEWAY]->sa_len,
							 workbuf, sizeof(workbuf), NULL, 0, NI_NUMERICHOST, NULL);
		if (len < 0) {
			strcpy(workbuf, "invalid");
		}
    }

    so_ifindextoname(rtm->rtm_index, ifname);

    printf("%-16s ", dest != NULL ? inet_ntop(AF_INET, &dest->sin_addr, rbuf, sizeof(rbuf)) : "-");
    printf("%-16s ", mask != NULL ? inet_ntop(AF_INET, &mask->sin_addr, rbuf, sizeof(rbuf)) : "-");
    printf("%-18s ", workbuf[0] != '\0' ? workbuf : "-");
    printf(" %-8s ", flags);
    printf(" %-6s ", ifname);
    printf(" %d ", rtm->rtm_index);

    printf("\n");
}

void dump_rtlist(void)
{
	union {
		struct rt_msghdr	rt;
		char	buf[4096];
	} rtbuf;
	struct rt_msghdr* rtm;
	ER er;

	printf("Destination      Netmask          Gateway             Flags     Interface\n");
	er = so_rtlist(AF_INET, NET_RT_DUMP, 0, &rtbuf, sizeof(rtbuf) );
	DEBUG_PRINT(("dump_rtlist: so_rtlist = %d(%d, %d)\n", er, MERCD(er), SERCD(er)));
	for(rtm = &rtbuf.rt; rtm->rtm_msglen != 0; rtm = (struct rt_msghdr *)((void*)rtm + rtm->rtm_msglen) ) {
		np_rtentry(rtm);
	}
}

