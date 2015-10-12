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
 *	@(#)net_test.c
 *
 */

#include <tk/tkernel.h>

#include <t2ex/socket.h>
#include <t2ex/errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "dhclient.h"
#include "route.h"
#include "ping.h"
#include "util.h"
#include "httpclient.h"


#define IP_T_ENGINE_ORG "202.32.0.87"
#define IP_GOOGLE_CO_JP "74.125.235.152"

#define HTTP_BUFSIZE 8192

static ID semid;
static ID semid2;
static ID server_tskid;
static ID client_tskid;

static void test_http(void)
{
	int re;
	char* buf;

	printf("[http] start\n");

	buf = malloc(HTTP_BUFSIZE);
	bzero(buf, HTTP_BUFSIZE);
	re = http_get("t-engine.org", "/", buf, HTTP_BUFSIZE);
	DEBUG_PRINT(("server_task: http_get = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		printf("[http] FAILED\n");
		return;
	}
	printf(buf);
	free(buf);

	printf("[http] OK\n");
}

static void test_getaddrinfo(void)
{
	const char* hostname = "www.t-engine.org";//"localhost";
	struct addrinfo hints, *res;
	struct in_addr addr;
	char* buf;
	int re;
	int size;
	char rbuf[18];

	printf("[getaddrinfo] start\n");

	bzero(&hints, sizeof hints);
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	size = so_getaddrinfo(hostname, NULL, &hints, &res, NULL, 0, NULL);
	DEBUG_PRINT(("test_getaddrinfo: so_getaddrinfo = %d(%d, %d)\n", size, MERCD(size), SERCD(size)));
	if ( size < 0 ) {
		printf("[getaddrinfo] FAILED\n");
		return;
	}

	buf = malloc(size);
	re = so_getaddrinfo(hostname, NULL, &hints, &res, buf, size, NULL);
	DEBUG_PRINT(("test_getaddrinfo: so_getaddrinfo = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		printf("[getaddrinfo] FAILED\n");
		return;
	}
	addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
	printf("test_getaddrinfo: %s => %s\n", hostname, inet_ntop(AF_INET, &addr, rbuf, sizeof(rbuf)));
	free(buf);

	printf("[getaddrinfo] OK\n");
}

static void test_getnameinfo(void)
{
	struct sockaddr_in sa;
	char buf[NI_MAXHOST];
	char buf2[NI_MAXSERV];
	int re;
	char rbuf[18];

	printf("[getnameinfo] start\n");

	bzero(&sa, sizeof sa);
	sa.sin_len = sizeof sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sa.sin_port = htons(12345);

	re = so_getnameinfo((struct sockaddr*)&sa, sizeof sa, buf, sizeof buf, buf2, sizeof buf2, 0, NULL);
	DEBUG_PRINT(("test_getnameinfo: so_getnameinfo = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		printf("[getnameinfo] FAILED\n");
		return;
	}
	printf("test_getnameinfo: %s:%d => %s:%s\n", inet_ntop(AF_INET, &sa.sin_addr, rbuf, sizeof(rbuf)), ntohs(sa.sin_port), buf, buf2);

	sa.sin_addr.s_addr = inet_addr(IP_T_ENGINE_ORG);
	sa.sin_port = htons(80);
	re = so_getnameinfo((struct sockaddr*)&sa, sizeof sa, buf, sizeof buf, buf2, sizeof buf2, 0, NULL);
	DEBUG_PRINT(("test_getnameinfo: so_getnameinfo = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		printf("[getnameinfo] FAILED\n");
		return;
	}
	printf("test_getnameinfo: %s:%d => %s:%s\n", inet_ntop(AF_INET, &sa.sin_addr, rbuf, sizeof(rbuf)), ntohs(sa.sin_port), buf, buf2);

	printf("[getnameinfo] OK\n");
}

static void wait_data(int sd)
{
	int re;
	fd_set fdset;

	FD_ZERO(&fdset);
	FD_SET(sd, &fdset);

	re = so_select(sd+1, &fdset, NULL, NULL, NULL);
	DEBUG_PRINT(("wait_data: so_select = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
}

static void test_tcp_server(void)
{
	int re;
	int sd;
	int reader = 0;
	char buf[5];
	struct sockaddr_in sa;
	struct sockaddr_in sa2;
	socklen_t sa_len;

	printf("[tcp(server)] start\n");

	sd = so_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( sd < 0 ) {
		goto error;
	}
	DEBUG_PRINT(("server_task: so_socket = %d(%d, %d)\n", sd, MERCD(sd), SERCD(sd)));

	bzero(&sa, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(12345);
	re = so_bind(sd, (struct sockaddr*)&sa, sizeof sa);
	DEBUG_PRINT(("server_task: so_bind = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		goto error;
	}

	re = so_listen(sd, 5);
	DEBUG_PRINT(("server_task: so_listen = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		goto error;
	}

	tk_sig_sem(semid, 1);
	DEBUG_PRINT(("server_task: server semaphore signaled 1\n"));

	reader = so_accept(sd, (struct sockaddr*)&sa2, &sa_len);
	DEBUG_PRINT(("server_task: so_accept = %d(%d, %d)\n", reader, MERCD(reader), SERCD(reader)));
	if ( reader < 0 ) {
		goto error;
	}

	wait_data(reader);

	bzero(buf, sizeof buf);
	re = so_sockatmark(reader);
	DEBUG_PRINT(("server_task: so_sockatmark = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		goto error;
	}
	re = so_read(reader, buf, 4);
	DEBUG_PRINT(("server_task: so_read = %d(%d, %d), buf = %s\n", re, MERCD(re), SERCD(re), buf));
	if ( re < 0 || memcmp(buf, "1234", 4) != 0 ) {
		goto error;
	}

	wait_data(reader);

	bzero(buf, sizeof buf);
	re = so_sockatmark(reader);
	DEBUG_PRINT(("server_task: so_sockatmark = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		goto error;
	}
	re = so_recv(reader, buf, 4, MSG_OOB);
	DEBUG_PRINT(("server_task: so_recv = %d(%d, %d), buf = %s\n", re, MERCD(re), SERCD(re), buf));
	if ( re < 0 || buf[0] != 'a' ) {
		goto error;
	}

	tk_sig_sem(semid2, 1);
	DEBUG_PRINT(("server_task: server semaphore for break signaled 2\n"));

	DEBUG_PRINT(("server_task: pre-accept for break\n"));
	re = so_accept(sd, (struct sockaddr*)&sa2, &sa_len);
	DEBUG_PRINT(("server_task: so_accept = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re != EX_INTR ) {
		goto error;
	}

	so_close(reader);
	so_close(sd);

	printf("[tcp(server)] OK\n");
	return;

error:
	printf("[tcp(server)] FAILED\n");
	if ( sd > 0 ) {
		so_close(sd);
	}
	if ( reader > 0 ) {
		so_close(reader);
	}
	tk_del_sem(semid2);
	return;
}

static void test_tcp_client(void)
{
	int sd;
	int re;
	struct sockaddr_in sa;

	printf("[tcp(client)] start\n");

	sd = so_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	DEBUG_PRINT(("so_socket = %d(%d, %d)\n", sd, MERCD(sd), SERCD(sd)));
	if ( sd < 0 ) {
		goto error2;
	}

	bzero(&sa, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sa.sin_port = htons(12345);
	re = so_connect(sd, (struct sockaddr*)&sa, sizeof sa);
	printf("so_connect = %d(%d, %d)\n", re, MERCD(re), SERCD(re));
	if ( re < 0 ) {
		goto error2;
	}

	re = so_write(sd, "1234", 4);
	DEBUG_PRINT(("so_write = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		goto error2;
	}

	re = so_send(sd, "a", 1, MSG_OOB);
	DEBUG_PRINT(("so_send = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		goto error2;
	}

	so_close(sd);

	tk_wai_sem(semid2, 1, TMO_FEVR);
	re = so_break(server_tskid);
	DEBUG_PRINT(("so_break = %d(%d, %d)\n", re, MERCD(re), SERCD(re)));
	if ( re < 0 ) {
		goto error2;
	}

	printf("[tcp(client)] OK\n");
	return;

error2:
	if ( sd > 0 ) {
		so_close(sd);
	}
	so_break(server_tskid);
	printf("[tcp(client)] FAILED\n");
}

static void server_task(INT stacd, VP exinf)
{
	DEBUG_PRINT(("server task started\n"));

	ping(inet_addr(IP_T_ENGINE_ORG)); // t-engine.org
	traceroute(inet_addr(IP_T_ENGINE_ORG)); // t-engine.org
	//traceroute(inet_addr(IP_GOOGLE_CO_JP)); // google.co.jp

	test_tcp_server();

	test_getaddrinfo();
	test_getnameinfo();

	test_http();

	tk_sig_sem(semid, 1);

	tk_exd_tsk();
}

void client_task(INT stacd, VP exinf)
{
	DEBUG_PRINT(("client task started\n"));

	test_tcp_client();

	tk_sig_sem(semid, 1);

	tk_exd_tsk();
}

void net_test(void)
{
	T_CSEM csem;
	T_CTSK ctsk;

	printf(" == net test == \n");

	net_conf(NET_CONF_EMULATOR, NET_CONF_DHCP);
	net_show();

	csem.maxsem = 100;
	csem.isemcnt = 0;
	csem.sematr = TA_TFIFO | TA_FIRST;
	semid = tk_cre_sem(&csem);
	semid2 = tk_cre_sem(&csem);

	bzero(&ctsk, sizeof ctsk);
	ctsk.tskatr = TA_HLNG | TA_RNG0;
	ctsk.task = server_task;
	ctsk.itskpri = 100;
	ctsk.stksz = 32 * 1024 * 2;
	server_tskid = tk_cre_tsk(&ctsk);
	DEBUG_PRINT(("start server task %d\n", server_tskid));
	tk_sta_tsk(server_tskid, 0);

	DEBUG_PRINT(("wait server semaphore\n"));
	tk_wai_sem(semid, 1, TMO_FEVR);

	bzero(&ctsk, sizeof ctsk);
	ctsk.tskatr = TA_HLNG | TA_RNG0;
	ctsk.task = client_task;
	ctsk.itskpri = 101;
	ctsk.stksz = 4 * 1024 * 2;
	client_tskid = tk_cre_tsk(&ctsk);
	DEBUG_PRINT(("start client task %d\n", client_tskid));
	tk_sta_tsk(client_tskid, 0);

	DEBUG_PRINT(("waiting for server and client semaphore\n"));
	tk_wai_sem(semid, 2, TMO_FEVR);

	printf(" == net test end == \n");

	tk_del_sem(semid);
	tk_del_sem(semid2);
}

