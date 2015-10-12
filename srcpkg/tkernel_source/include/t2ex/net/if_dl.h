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
 *	@(#)if_dl.h
 *
 */

#ifndef	__NET_IF_DL_H__
#define	__NET_IF_DL_H__

#include <basic.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Structure of a Link-Level sockaddr:
 */
struct sockaddr_dl {
	uint8_t	    sdl_len;	/* Total length of sockaddr */
	sa_family_t sdl_family;	/* AF_LINK */
	uint16_t   sdl_index;	/* if != 0, system given index for interface */
	uint8_t	    sdl_type;	/* interface type */
	uint8_t	    sdl_nlen;	/* interface name length, no trailing 0 reqd. */
	uint8_t	    sdl_alen;	/* link level address length */
	uint8_t	    sdl_slen;	/* link layer selector length */
	/* minimum work area, can be larger; contains both if name
	 * and ll address
	 */
	char	    sdl_data[12];
};

#define	satosdl(__sa)	((struct sockaddr_dl *)(__sa))
#define	satocsdl(__sa)	((const struct sockaddr_dl *)(__sa))

#define LLADDR(s) ((char *)((s)->sdl_data + (s)->sdl_nlen))
#define CLLADDR(s) ((const char *)((s)->sdl_data + (s)->sdl_nlen))

#ifdef __cplusplus
}
#endif
#endif	/* __NET_IF_DL_H__ */
