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
#include <stdarg.h>
#include <t2ex/socket.h>

#include "network_sample/util.h"
#include "network_sample/dhclient.h"
#include "network_sample/route.h"

#define IPADDR	"192.168.0.2"
#define NETMASK	"255.255.255.0"
#define GATEWAY	"192.168.0.1"

#define DNSSERVER1 "127.0.0.1"
//#define DNSSERVER2 "127.0.0.2"
//#define DNSSERVER3 "127.0.0.3"
#define DNSDOMAIN "localhost.localdomain"

#define DEBUG

#ifdef DEBUG
static void NETDBG(const char *format, ...)
{
	va_list ap;

	printf("[NET] ");
	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
	printf("\n");
}
#else
#define NETDBG(x, ...)
#endif

int net_conf_dhcp(void)
{
	/*
	 * Configuration of network addresses using DHCP.
	 *
	 */
	NETDBG("Assign INADDR_ANY to Neta before transmitting DHCP packets.");
	set_ifaddr("Neta", htonl(INADDR_ANY), htonl(INADDR_ANY));

	NETDBG("Retrieve network information from a DHCP server.");
	dhclient("Neta");

	return 0;
}

int net_conf_static(void)
{
	int index;
	struct sockaddr_in sin;

	/*
	 * Configuration for the network interface card Neta.
	 *
	 *  - Assign a IP address and a broadcast address to Neta.
	 *  - Bring up lo0.
	 */
	NETDBG("Assign a IP address to Neta.");
	set_ifaddr("Neta", inet_addr(IPADDR), inet_addr(NETMASK));
	NETDBG("Bring up Neta.");
	if_updown("Neta", 1);

	/*
	 * Configuration for a default gateway.
	 */
	NETDBG("Set a default gateway.");
	index = so_ifnametoindex("Neta");
	route_add(INADDR_ANY, inet_addr(GATEWAY), INADDR_ANY, index, 0);

	/*
	 * Configuration for DNS servers.
	 */
#if defined(DNSSERVER1) || defined(DNSSERVER2) || defined(DNSSERVER3)
	NETDBG("Configure DNS.");
	memset(&sin, 0, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
#endif
#ifdef DNSSERVER1
	sin.sin_addr.s_addr = inet_addr(DNSSERVER1);
	so_resctl(SO_RES_ADD_SERVER, &sin, sizeof(sin));
#endif
#ifdef DNSSERVER2
	sin.sin_addr.s_addr = inet_addr(DNSSERVER2);
	so_resctl(SO_RES_ADD_SERVER, &sin, sizeof(sin));
#endif
#ifdef DNSSERVER3
	sin.sin_addr.s_addr = inet_addr(DNSSERVER3);
	so_resctl(SO_RES_ADD_SERVER, &sin, sizeof(sin));
#endif

#ifdef DNSDOMAIN
	so_resctl(SO_RES_ADD_DOMAIN, DNSDOMAIN, sizeof(DNSDOMAIN)+1);
#endif

	return 0;
}

/*
 * This function configures 
 *  - IP addresses of the loopback device and the network interface card Neta,
 *  - a default gateway, and 
 *  - DNS servers.
 *
 * argument:
 *   emu: 
 *     - zero:     initialization for em1d
 *     - non-zero: initialization for emulator
 *   dhcp:
 *     - zero:     static 
 *     - non-zero: DHCP
 * 
 */
int net_conf(int emu, int dhcp)
{
	ER ercd;

	NETDBG("Network initialization.");

	/*
	 * Configuration for the emulator.
	 *
	 * The LAN driver passes packets to upper layers whose size are larger
	 * than 60 bytes by default, otherwise the driver discards them.  The
	 * minimum size 60 bytes is the minimum ethernet frame size without
	 * CRC. So small-sized packets are padded to bring them to this length
	 * before transmission.
	 *
	 * But it can be possible to receive packets whose size smaller than
	 * 60 bytes by using a TUN device, because paddings are usually
	 * inserted into those small packets by a network adapter and packets
	 * do not go thorugh a network adapter when they are directly received
	 * from or sent to a TUN device.
	 *
	 * So we have to change the minimum packet size then we use the QEMU
	 * emulator which uses a TUN device.
	 */
	if (emu != 0) {
		NETDBG("Set the minium packet size to 42 byte for using emulator.");
		netdrv_set_minpktsz("Neta", 42);
	}

	/*
	 * Configuration for the loopback device lo0.
	 *
	 *  - Assign a local address 127.0.0.1 to lo0.
	 *  - Bring up lo0.
	 */
	NETDBG("Assign 127.0.0.1 to lo0.");
	set_ifaddr("lo0", htonl(INADDR_LOOPBACK), inet_addr("255.0.0.0"));
	NETDBG("Bring up lo0.");
	if_updown("lo0", 1);

	/*
	 * Configuration for the network interface card Neta.
	 *
	 * - Attach a device driver Neta.
	 * - Assign an IP address to Neta.
	 * - Set a default gateway address.
	 * - Set DNS servers.
	 */
	NETDBG("Attach a device driver Neta.");
	ercd = so_ifattach("Neta");
	if (ercd < 0) {
		return ercd;
	}
	if (dhcp) {
		net_conf_dhcp();
	}
	else {
		net_conf_static();
	}

	/*
	 * Configuration for the host table.
	 *
	 * - Add an entry for "localhost" whose IP address is 127.0.0.1.
	 */
	NETDBG("Add an entry for localhost into the host name table.");
	add_hosttable("localhost", htonl(INADDR_LOOPBACK));
	add_hosttable("test", inet_addr("127.0.0.2"));

	return 0;
}
