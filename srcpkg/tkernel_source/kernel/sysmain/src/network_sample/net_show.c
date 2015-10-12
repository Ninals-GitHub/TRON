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

#include <basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <t2ex/socket.h>

#include "network_sample/util.h"
#include "network_sample/route.h"

static void net_show_inet(const struct ifaddrs *ifa)
{
	unsigned char str[32];
	struct sockaddr_in *sin;

	sin = (struct sockaddr_in *)ifa->ifa_addr;
	printf("    inet:      %s\n", 
	       inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)));

	if ((ifa->ifa_flags & IFF_BROADCAST) != 0) {
		sin = (struct sockaddr_in *)ifa->ifa_broadaddr;
		printf("    broadcast: %s\n", 
		       inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)));
	}

	sin = (struct sockaddr_in *)ifa->ifa_netmask;
	printf("    netmask:   %s\n",
	       inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)));
}

static void net_show_link(const struct ifaddrs *ifa)
{
	char buf[32];
	int len;

	len = so_getnameinfo(ifa->ifa_addr, ifa->ifa_addr->sa_len,
			     buf, sizeof(buf), NULL, 0, NI_NUMERICHOST, NULL);
	if (len < 0) {
		return;
	}
	if (buf[0] == '\0') {
		return;
	}

	printf("    address:   %s\n", buf);
}

static void net_show_if(void)
{
	struct ifaddrs *ifa;
	void *buf;
	int len;
	char lastname[IFNAMSIZ];

	len = so_getifaddrs(&ifa, NULL, 0);
	if (len < 0) {
		printf("error: so_getifaddrs: %d(%d,%d)\n", len, MERCD(len), 
		       SERCD(len));
		return;
	}
	buf = malloc(len);
	if (buf == NULL) {
		return;
	}
	len = so_getifaddrs(&ifa, buf, len);
	if (len < 0) {
		printf("error: so_getifaddrs: %d(%d,%d)\n", len, MERCD(len), 
		       SERCD(len));
		free(buf);
		return;
	}

	lastname[0] = '\0';
	for( ; ifa != NULL; ifa=ifa->ifa_next) {
		if (strncmp(ifa->ifa_name, lastname, IFNAMSIZ) != 0) {
			printf("%s:\n", ifa->ifa_name);
			strncpy(lastname, ifa->ifa_name, IFNAMSIZ-1);
		}
		if (ifa->ifa_addr->sa_family == AF_LINK) {
			net_show_link(ifa);
		}
		else if (ifa->ifa_addr->sa_family == AF_INET) {
			net_show_inet(ifa);
		}
	}
	free(buf);
}

static void net_show_dns(void)
{
	union {
		struct sockaddr *stop;
		char *ctop;
		UB c[256];
	} buf;
	int len, i;
	struct sockaddr_in *sin;
	struct sockaddr **res;
	unsigned char rbuf[32];
	char **domains;

	len = so_resctl(SO_RES_GET_SERVERS, &buf.stop, sizeof(buf));
	if (len < 0) {
		printf("error: so_resctl()\n");
		return;
	}
	res = &buf.stop;
	for( i=0; res[i] != NULL; i++ ) {
		sin = (struct sockaddr_in *)res[i];
		printf( "server: %s\n", 
			inet_ntop(AF_INET, &sin->sin_addr, rbuf, sizeof(rbuf)));
	}

	len = so_resctl(SO_RES_GET_DOMAINS, &buf.ctop, sizeof(buf));
	if (len < 0) {
		printf("error: so_resctl()\n");
		return;
	}
	domains = &buf.ctop;
	for( i=0; domains[i] != NULL; i++ ) {
		printf( "domain: %s\n", domains[i] );
	}
}

static void net_show_hosttable(void) __attribute__((noinline));
static void net_show_hosttable(void)
{
	union {
		struct hosttable top;
		UB c[256];
	} buf;
	int len, i;
	struct hosttable *table;
	unsigned char rbuf[32];
	struct sockaddr_in *sin;

	len = so_resctl(SO_RES_GET_TABLES, &buf.top, sizeof(buf));
	if (len < 0) {
		printf("error: so_resctl()\n");
		return;
	}
	table = &buf.top;
	for(i=0; table[i].addr != NULL; i++) {
		sin = (struct sockaddr_in *)table[i].addr;
		printf("%-18s ", inet_ntop(AF_INET, &sin->sin_addr, rbuf, sizeof(rbuf)));
		printf("%s ", table[i].host );
		printf("%s\n", table[i].aliases != NULL ? table[i].aliases : "");
	}
}

void net_show(void)
{
	printf("\n--------------------------------\n");
	printf("Network interface configurations\n");
	printf("--------------------------------\n");

	net_show_if();

	printf("\n-----------------\n");
	printf("DNS configuraions\n");
	printf("-----------------\n");

	net_show_dns();

	printf("\n----------\n");
	printf("Host table\n");
	printf("----------\n");

	net_show_hosttable();

	printf("\n-------------------\n");
	printf("Routing information\n");
	printf("-------------------\n");

	dump_rtlist();

	printf("\n");
}
