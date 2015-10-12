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
 *	@(#)util.h
 *
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <t2ex/socket.h>
#include <string.h>

#define NET_CONF_MACHINE	(0)
#define NET_CONF_EMULATOR	(1)

#define NET_CONF_STATIC		(0)
#define NET_CONF_DHCP		(1)

int net_conf(int emu, int dhcp);
void net_show(void);

int net_random(void);
unsigned short checksum2(unsigned short *buf, int bufsz, unsigned long initial);
unsigned short checksum(unsigned short *buf, int bufsz);
int get_hwaddr(const char* ifname, char* buf);
int if_updown(const char* ifname, int is_up);
void set_ifaddr(const char* ifname, in_addr_t addr, in_addr_t mask);
void set_bpfif(const char* path, const char* ifname);
void add_hosttable(const char* hostname, in_addr_t addr);
in_addr_t resolv_host(const char* host);
void netdrv_set_minpktsz(const char* ifname, W minsz);

#ifdef DEBUG
#define DEBUG_PRINT(x)	printf x
#else
#define DEBUG_PRINT(x)
#endif

#endif /* UTIL_H_ */
