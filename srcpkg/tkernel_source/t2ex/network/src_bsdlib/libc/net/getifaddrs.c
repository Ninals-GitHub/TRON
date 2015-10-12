/*	$NetBSD: getifaddrs.c,v 1.11.12.1 2009/05/03 13:17:52 bouyer Exp $	*/

/*
 * Copyright (c) 1995, 1999
 *	Berkeley Software Design, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY Berkeley Software Design, Inc. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Berkeley Software Design, Inc. BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	BSDI getifaddrs.c,v 2.12 2000/02/23 14:51:59 dab Exp
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: getifaddrs.c,v 1.11.12.1 2009/05/03 13:17:52 bouyer Exp $");
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/param.h>
#include <net/route.h>
#ifndef T2EX
#include <sys/sysctl.h>
#endif
#include <net/if_dl.h>

#include <assert.h>
#include <errno.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>

#ifdef T2EX
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <t2ex/socket.h>
#include <t2ex/errno.h>
#endif

#ifdef __weak_alias
__weak_alias(getifaddrs,_getifaddrs)
#ifndef T2EX
__weak_alias(freeifaddrs,_freeifaddrs)
#endif
#endif

#define	SALIGN	(sizeof(long) - 1)
#define	SA_RLEN(sa)	((sa)->sa_len ? (((sa)->sa_len + SALIGN) & ~SALIGN) : (SALIGN + 1))

#ifndef T2EX
int
getifaddrs(struct ifaddrs **pif)
#else
int
getifaddrs(struct ifaddrs **pif, void* rbuf, size_t rbufsz, size_t *asize)
#endif
{
	int icnt = 1;
	int dcnt = 0;
	int ncnt = 0;
#ifndef T2EX
	int mib[6];
#else
	size_t limit = 0;
	int usable_icnt = 1;
	int usable_dcnt = 0;
	int usable_ncnt = 0;
	ER ercd;
#endif
	size_t needed;
	char *buf;
	char *next;
	struct ifaddrs cif;
	char *p, *p0;
	struct rt_msghdr *rtm;
	struct if_msghdr *ifm;
	struct ifa_msghdr *ifam;
	struct sockaddr *sa;
	struct ifaddrs *ifa, *ift;
	u_short idx = 0;
	int i;
	size_t len, alen;
	char *data;
	char *names;

	_DIAGASSERT(pif != NULL);

#ifndef T2EX
	mib[0] = CTL_NET;
	mib[1] = PF_ROUTE;
	mib[2] = 0;             /* protocol */
	mib[3] = 0;             /* wildcard address family */
	mib[4] = NET_RT_IFLIST;
	mib[5] = 0;             /* no flags */
	if (sysctl(mib, __arraycount(mib), NULL, &needed, NULL, 0) < 0)
		return (-1);
	if ((buf = malloc(needed)) == NULL)
		return (-1);
	if (sysctl(mib, __arraycount(mib), buf, &needed, NULL, 0) < 0) {
		free(buf);
		return (-1);
	}
#else
	if ((needed = so_rtlist(AF_UNSPEC, NET_RT_IFLIST, 0, NULL, 0)) < 0)
		return ERRNO(needed);
	if ((buf = malloc(needed)) == NULL)
		return ENOMEM;
	if ((ercd = so_rtlist(AF_UNSPEC, NET_RT_IFLIST, 0, buf, needed)) < 0) {
		free(buf);
		return ERRNO(ercd);
	}
#endif

	for (next = buf; next < buf + needed; next += rtm->rtm_msglen) {
		rtm = (struct rt_msghdr *)(void *)next;
#ifdef T2EX
		if (rtm->rtm_msglen == 0) {
			break;
		}
#endif
		if (rtm->rtm_version != RTM_VERSION)
			continue;
		switch (rtm->rtm_type) {
		case RTM_IFINFO:
			ifm = (struct if_msghdr *)(void *)rtm;
			if (ifm->ifm_addrs & RTA_IFP) {
				const struct sockaddr_dl *dl;

				idx = ifm->ifm_index;
				++icnt;
				dl = (struct sockaddr_dl *)(void *)(ifm + 1);
				dcnt += SA_RLEN((const struct sockaddr *)(const void *)dl) +
				    ALIGNBYTES;
				dcnt += sizeof(ifm->ifm_data);
				ncnt += dl->sdl_nlen + 1;
			} else
				idx = 0;
			break;

		case RTM_NEWADDR:
			ifam = (struct ifa_msghdr *)(void *)rtm;
			if (idx && ifam->ifam_index != idx)
#ifndef T2EX
				abort();	/* this cannot happen */
#else
				return ENOTSUP;
#endif

#define	RTA_MASKS	(RTA_NETMASK | RTA_IFA | RTA_BRD)
			if (idx == 0 || (ifam->ifam_addrs & RTA_MASKS) == 0)
				break;
			p = (char *)(void *)(ifam + 1);
			++icnt;
			/* Scan to look for length of address */
			alen = 0;
			for (p0 = p, i = 0; i < RTAX_MAX; i++) {
				if ((RTA_MASKS & ifam->ifam_addrs & (1 << i))
				    == 0)
					continue;
				sa = (struct sockaddr *)(void *)p;
				len = SA_RLEN(sa);
				if (i == RTAX_IFA) {
					alen = len;
					break;
				}
				p += len;
			}
			for (p = p0, i = 0; i < RTAX_MAX; i++) {
				if ((RTA_MASKS & ifam->ifam_addrs & (1 << i))
				    == 0)
					continue;
				sa = (struct sockaddr *)(void *)p;
				len = SA_RLEN(sa);
				if (i == RTAX_NETMASK && sa->sa_len == 0)
					dcnt += alen;
				else
					dcnt += len;
				p += len;
			}
			break;
		}
#ifdef T2EX
		if( (rbuf != NULL) &&
		    (rbufsz >= sizeof(struct ifaddrs) * icnt + dcnt + ncnt) ) {	
			limit = (size_t)(next - buf + rtm->rtm_msglen);
			usable_icnt = icnt;
			usable_dcnt = dcnt;
			usable_ncnt = ncnt;
		}
#endif		
	}

	if (icnt + dcnt + ncnt == 1) {
		*pif = NULL;
		free(buf);
#ifdef T2EX
		*asize = 0;
#endif
		return (0);
	}
#ifndef T2EX
	data = malloc(sizeof(struct ifaddrs) * icnt + dcnt + ncnt);
	if (data == NULL) {
		free(buf);
		return(-1);
	}

	ifa = (struct ifaddrs *)(void *)data;
	data += sizeof(struct ifaddrs) * icnt;
	names = data + dcnt;
#else
	*asize = sizeof(struct ifaddrs) * icnt + dcnt + ncnt;

	if ((rbuf != 0 && usable_icnt + usable_dcnt + usable_ncnt == 1) ||
	    rbuf == NULL) {
		*pif = NULL;
		free(buf);
		return 0;
	}
	data = rbuf;
	needed = limit;

	ifa = (struct ifaddrs *)(void *)data;
	data += sizeof(struct ifaddrs) * usable_icnt;
	names = data + usable_dcnt;
#endif

	memset(ifa, 0, sizeof(struct ifaddrs) * icnt);
	ift = ifa;

	idx = 0;
	for (next = buf; next < buf + needed; next += rtm->rtm_msglen) {
		rtm = (struct rt_msghdr *)(void *)next;
		if (rtm->rtm_version != RTM_VERSION)
			continue;
		switch (rtm->rtm_type) {
		case RTM_IFINFO:
			ifm = (struct if_msghdr *)(void *)rtm;
			if (ifm->ifm_addrs & RTA_IFP) {
				const struct sockaddr_dl *dl;

				idx = ifm->ifm_index;
				dl = (struct sockaddr_dl *)(void *)(ifm + 1);

				memset(&cif, 0, sizeof(cif));

				cif.ifa_name = names;
				cif.ifa_flags = (int)ifm->ifm_flags;
				memcpy(names, dl->sdl_data,
				    (size_t)dl->sdl_nlen);
				names[dl->sdl_nlen] = 0;
				names += dl->sdl_nlen + 1;

				cif.ifa_addr = (struct sockaddr *)(void *)data;
				memcpy(data, dl, (size_t)dl->sdl_len);
				data += SA_RLEN((const struct sockaddr *)(const void *)dl);

				/* ifm_data needs to be aligned */
				cif.ifa_data = data = (void *)ALIGN(data);
				memcpy(data, &ifm->ifm_data, sizeof(ifm->ifm_data));
 				data += sizeof(ifm->ifm_data);
			} else
				idx = 0;
			break;

		case RTM_NEWADDR:
			ifam = (struct ifa_msghdr *)(void *)rtm;
			if (idx && ifam->ifam_index != idx)
#ifndef T2EX
				abort();	/* this cannot happen */
#else
				return ENOTSUP;
#endif

			if (idx == 0 || (ifam->ifam_addrs & RTA_MASKS) == 0)
				break;
			ift->ifa_name = cif.ifa_name;
			ift->ifa_flags = cif.ifa_flags;
			ift->ifa_data = NULL;
			p = (char *)(void *)(ifam + 1);
			/* Scan to look for length of address */
			alen = 0;
			for (p0 = p, i = 0; i < RTAX_MAX; i++) {
				if ((RTA_MASKS & ifam->ifam_addrs & (1 << i))
				    == 0)
					continue;
				sa = (struct sockaddr *)(void *)p;
				len = SA_RLEN(sa);
				if (i == RTAX_IFA) {
					alen = len;
					break;
				}
				p += len;
			}
			for (p = p0, i = 0; i < RTAX_MAX; i++) {
				if ((RTA_MASKS & ifam->ifam_addrs & (1 << i))
				    == 0)
					continue;
				sa = (struct sockaddr *)(void *)p;
				len = SA_RLEN(sa);
				switch (i) {
				case RTAX_IFA:
					ift->ifa_addr =
					    (struct sockaddr *)(void *)data;
					memcpy(data, p, len);
					data += len;
					if (ift->ifa_addr->sa_family == AF_LINK)
						ift->ifa_data = cif.ifa_data;
					break;

				case RTAX_NETMASK:
					ift->ifa_netmask =
					    (struct sockaddr *)(void *)data;
					if (sa->sa_len == 0) {
						memset(data, 0, alen);
						data += alen;
						break;
					}
					memcpy(data, p, len);
					data += len;
					break;

				case RTAX_BRD:
					ift->ifa_broadaddr =
					    (struct sockaddr *)(void *)data;
					memcpy(data, p, len);
					data += len;
					break;
				}
				p += len;
			}


			ift = (ift->ifa_next = ift + 1);
			break;
		}
	}

	free(buf);
	if (--ift >= ifa) {
		ift->ifa_next = NULL;
		*pif = ifa;
	} else {
		*pif = NULL;
#ifndef T2EX
		free(ifa);
#endif
	}
	return (0);
}

#ifndef T2EX
void
freeifaddrs(struct ifaddrs *ifp)
{

	_DIAGASSERT(ifp != NULL);

	free(ifp);
}
#endif
