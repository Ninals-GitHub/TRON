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
 *	@(#)httpclient.c
 *
 */

#include <t2ex/socket.h>
#include <tk/tkernel.h>

#include <stdio.h>
#include <strings.h>

#include "util.h"
#include "httpclient.h"

int http_get(const char* host, const char* path, char* buf, size_t buflen)
{
	int re;
	int sd;
	int error = 0;
	in_addr_t hostaddr;
	struct sockaddr_in sa;
	char req[1024];
	size_t offset;

	hostaddr = resolv_host(host);
	if ( hostaddr == INADDR_NONE ) {
		return -1;
	}

	sd = so_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	DEBUG_PRINT(("http_get: so_socket = %d(%d, %d)\n", sd, MERCD(sd), SERCD(sd)));
	if ( sd < 0 ) {
		return sd;
	}

	bzero(&sa, sizeof sa);
	sa.sin_len = sizeof sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = hostaddr;
	sa.sin_port = htons(80);
	re = so_connect(sd, (struct sockaddr*)&sa, sizeof sa);
	DEBUG_PRINT(("http_get: so_connect = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		return re;
	}

	sprintf(req, "GET %s HTTP/1.1\r\nhost: %s\r\nConnection: close\r\n\r\n", path, host);
	re = so_write(sd, req, strlen(req));
	DEBUG_PRINT(("http_get: so_write = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));

	for(offset = 0; offset < buflen;) {
		re = so_read(sd, &buf[offset], buflen - offset);
		DEBUG_PRINT(("http_get: so_read = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
		if ( re == 0 ) {
			break;
		} else if ( re < 0 ) {
			error = re;
			break;
		}
		offset += re;
	}

	so_close(sd);

	return error;
}
