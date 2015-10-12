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
 *	@(#)tkn_netdmn.c
 *
 */

#include <tk/tkernel.h>

#include <sys/proc.h>
#include <sys/tkn_syscall.h>

#include <sys/syscallargs.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <t2ex/socket.h>
#include <sys/atomic.h>
#include "tkn.h"
#include "tkn_resctl.h"
#include "netmain/tkn_spl.h"

LOCAL ID porid;
LOCAL ID netdmn_taskid;
LOCAL ID req_taskid;

/*
 * Request packet
 */
typedef struct reqpkt {
	INT	fn;
	ID	tskid;
	union {
		NET_SO_GETADDRINFO_PARA	getaddrinfo;
		NET_SO_GETNAMEINFO_PARA	getnameinfo;
		NET_SO_RESCTL_PARA	resctl;
	} a;
} REQPKT;

/*
 * Response packet
 */
typedef struct respkt {
	INT	fn;
	INT	len;
	INT	error;
} RESPKT;

typedef union {
	REQPKT	req;
	RESPKT	res;
} CALPKT;

/*
 * Abort by so_break()
 */
LOCAL void break_netdmn( ID tid )
{
	int breaked = 0;

	/*
	 * Keep calling so_break() while the network daemon task is
	 * processing a task's request whose task ID is tid.
	 */
	while( breaked == 0 ) {
		if( req_taskid == tid ) {
			so_break( netdmn_taskid );
		} else {
			breaked = 1;
		}
		tk_dly_tsk( 10 );
	}
}

LOCAL int call_netdmn( CALPKT *cp, TMO_U tmo )
{
	ER	err;
	ID	tid;
	lwp_t	*lwp = NULL;

	lwp = curlwp;
	lwp->is_waiting = 1;

	/*
	 * Pass a request packet to the network daemon and wait for the
	 * completion.
	 */
	tid = tk_get_tid();
	cp->req.tskid = tid;
	err = tk_cal_por_u(porid, 1, cp, sizeof(REQPKT), tmo);

	lwp->is_waiting = 0;

	if ( err == E_TMOUT ) {
		return ETIMEDOUT;
	} else if ( err == E_DISWAI ) {
		break_netdmn( tid );
		return EINTR;
	} else if ( err < E_OK ) {
		goto err_ret;
	}

	if ( cp->res.error != 0 ) {
		return cp->res.error;
	}

	return 0;

  err_ret:
	//DEBUG_PRINT(("call_netdmn err = %d\n", err));
	return err;
}

IMPORT int getaddrinfo(const char *hostname, const char *servname, const struct addrinfo *hints, struct addrinfo **res, void* buf, size_t bufsz, int* len);
IMPORT int getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);

LOCAL int resctl(int cmd, void* buf, size_t bufsz, size_t *len);

LOCAL void netdmn_accept( INT stacd, VP exinf )
{
	REQPKT	req;
	RESPKT	res;

	RNO	rdv;
	ER	err;
	T_RTSK	rtsk;

	int	error = 0;

	(void)stacd;
	(void)exinf;

	for ( ;; ) {
		err = tk_acp_por(porid, 1, &rdv, &req, TMO_FEVR);
		if ( err == E_DLT ) {
			break;
		} else if ( err < E_OK ) {
			continue;
		}

		atomic_xchg( (UINT *)&req_taskid, req.tskid );

		/*
		 * Check if the task that called tk_cal_por() has its WAITING
		 * state released by so_break() during the period from
		 * accepting a rendezvous until putting the requested task
		 * id into req_taskid. In this case, the accepted rendezvous
		 * should be ignored.
		 * This is the rare case.
		 */
		err = tk_ref_tsk( req_taskid, &rtsk );
		if ( err < 0 ) {
			continue;
		}
		if ( (rtsk.tskwait & TTW_RDV) == 0 ) {
			continue;
		}

		/* Set task space */
		err = SetTaskSpace( req_taskid );
		if ( err < 0 ) {
			continue;
		}

		switch ( req.fn ) {
		  case NET_SO_GETADDRINFO_FN: {
			int len = 0;
			error = getaddrinfo(req.a.getaddrinfo.node, req.a.getaddrinfo.service, req.a.getaddrinfo.hints, req.a.getaddrinfo.res, req.a.getaddrinfo.buf, req.a.getaddrinfo.bufsz, &len);
			res.error = error;
			res.len = len;
			break;
		  }
		  case NET_SO_GETNAMEINFO_FN: {
			int re = getnameinfo(req.a.getnameinfo.sa, req.a.getnameinfo.salen, req.a.getnameinfo.host, req.a.getnameinfo.hostlen, req.a.getnameinfo.serv, req.a.getnameinfo.servlen, req.a.getnameinfo.flags);
			res.error = re;
			break;
		  }
		case NET_SO_RESCTL_FN: {
			int len = 0;
			error = resctl(req.a.resctl.cmd, req.a.resctl.buf, req.a.resctl.bufsz, (size_t*)&len);
			res.error = error;
			res.len = len;
			break;
		}
		  default:
			res.error = -1;
		}

		res.fn = req.fn;

		atomic_xchg( (UINT *)&req_taskid, 0);

		tk_rpl_rdv(rdv, &res, sizeof(res));
	}

	tk_exd_tsk();
}

EXPORT ER init_netdmn( void )
{
	T_CPOR	cpor;
	T_CTSK ctsk;
	ER	err;

	/* Create a rendezvous port */
	cpor.exinf = NULL;
	cpor.poratr = TA_TFIFO;
	cpor.maxcmsz = sizeof(REQPKT);
	cpor.maxrmsz = sizeof(RESPKT);
	err = tk_cre_por(&cpor);
	if ( err < E_OK ) goto err_ret0;
	porid = err;

	/* Create and start a network daemon task. */
	ctsk.itskpri = tkn_spl_priority(IPL_NONE) + 1;
	ctsk.task = netdmn_accept;
	ctsk.tskatr = TA_RNG0 | TA_HLNG;
	ctsk.stksz  = TKN_NETDMN_STKSZ;
	ctsk.dsname[0] = 'n';
	ctsk.dsname[1] = 'e';
	ctsk.dsname[2] = 't';
	ctsk.dsname[3] = 'm';
	ctsk.dsname[4] = NULL;
	err = tk_cre_tsk(&ctsk);
	if ( err < E_OK ) goto err_ret1;
	netdmn_taskid = err;

	err = tk_sta_tsk(netdmn_taskid, 0);
	if ( err < E_OK ) goto err_ret2;

	return E_OK;

err_ret2:
  	tk_del_tsk(netdmn_taskid);
err_ret1:
	tk_del_por(porid);
err_ret0:
	//DEBUG_PRINT(("init err = %d\n", err));
	return err;
}

EXPORT ER finish_netdmn(void)
{
	ER ercd;

	ercd = tk_del_por(porid);

	tkn_resctl_flush_tables();

	return ercd;
}

/* ----------------------------------------------------- */

LOCAL int check_getaddrinfo_args(const char* node, const char* service,
				 const struct addrinfo* hints, void* buf,
				 size_t bufsz)
{
	ER ercd;

	if ( node != NULL ) {
		ercd = ChkSpaceBstrR(node, 0);
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}

	if ( service != NULL ) {
		ercd = ChkSpaceBstrR(service, 0);
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}

	if ( hints != NULL ) {
		ercd = ChkSpaceR(hints, sizeof(struct addrinfo));
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}

	if ( buf != NULL ) {
		ercd = ChkSpaceR(buf, bufsz);
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}

	return 0;
}

int
sys_getaddrinfo(struct lwp *l, const struct sys_getaddrinfo_args *uap, register_t *retval)
{
	int error;

	(void)l;

	error = check_getaddrinfo_args(SCARG(uap, node), SCARG(uap, service),
				       SCARG(uap, hints), SCARG(uap, buf),
				       SCARG(uap, bufsz));
	if ( error != 0 ) {
		return error;
	}
	if ( SCARG(uap, timeout) != NULL ) {
		error = ChkSpaceR(SCARG(uap, timeout), sizeof(struct timeval));
		if ( error < E_OK ) {
			return EFAULT;
		}
	}

	CALPKT cp;
	cp.req.fn = NET_SO_GETADDRINFO_FN;
	cp.req.a.getaddrinfo = uap->a;

	TMO_U tmo = SCARG(uap, timeout) == NULL ? TMO_FEVR : (TMO_U)SCARG(uap, timeout)->tv_sec * 1000000 + SCARG(uap, timeout)->tv_usec;

	int re = call_netdmn(&cp, tmo);

	*retval = cp.res.len;
	return re;
}

int
sys_getaddrinfo_ms(struct lwp *l, const struct sys_getaddrinfo_ms_args *uap, register_t *retval)
{
	int error;

	(void)l;

	error = check_getaddrinfo_args(SCARG(uap, node), SCARG(uap, service),
				       SCARG(uap, hints), SCARG(uap, buf),
				       SCARG(uap, bufsz));
	if ( error != 0 ) {
		return error;
	}

	CALPKT cp;
	cp.req.fn = NET_SO_GETADDRINFO_FN;
	cp.req.a.getaddrinfo.node = SCARG(uap, node);
	cp.req.a.getaddrinfo.service = SCARG(uap, service);
	cp.req.a.getaddrinfo.hints = SCARG(uap, hints);
	cp.req.a.getaddrinfo.res = SCARG(uap, res);
	cp.req.a.getaddrinfo.buf = SCARG(uap, buf);
	cp.req.a.getaddrinfo.bufsz = SCARG(uap, bufsz);

	TMO_U tmo = (TMO_U)SCARG(uap, tmout);
	if ( tmo > 0 ) {
		tmo = tmo * 1000;
	}

	int re = call_netdmn(&cp, tmo);

	*retval = cp.res.len;
	return re;
}

int
sys_getaddrinfo_us(struct lwp *l, const struct sys_getaddrinfo_us_args *uap, register_t *retval)
{
	int error;

	(void)l;

	error = check_getaddrinfo_args(SCARG(uap, node), SCARG(uap, service),
				       SCARG(uap, hints), SCARG(uap, buf),
				       SCARG(uap, bufsz));
	if ( error != 0 ) {
		return error;
	}

	CALPKT cp;
	cp.req.fn = NET_SO_GETADDRINFO_FN;
	cp.req.a.getaddrinfo.node = SCARG(uap, node);
	cp.req.a.getaddrinfo.service = SCARG(uap, service);
	cp.req.a.getaddrinfo.hints = SCARG(uap, hints);
	cp.req.a.getaddrinfo.res = SCARG(uap, res);
	cp.req.a.getaddrinfo.buf = SCARG(uap, buf);
	cp.req.a.getaddrinfo.bufsz = SCARG(uap, bufsz);

	TMO_U tmo = SCARG(uap, tmout_u);

	int re = call_netdmn(&cp, tmo);

	*retval = cp.res.len;
	return re;
}

LOCAL int check_getnameinfo_args(const struct sockaddr* sa, socklen_t salen,
				 char* host, size_t hostlen,
				 char* serv, size_t servlen)
{
	ER ercd;

	if ( sa != NULL ) {
		ercd = ChkSpaceR(sa, salen);
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}

	if ( host != NULL ) {
		ercd = ChkSpaceRW(host, hostlen);
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}

	if ( serv != NULL ) {
		ercd = ChkSpaceRW(serv, servlen);
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}

	return 0;
}

int
sys_getnameinfo(struct lwp *l, const struct sys_getnameinfo_args *uap, register_t *retval)
{
	int error;

	(void)l;

	error = check_getnameinfo_args(SCARG(uap, sa), SCARG(uap, salen),
				       SCARG(uap, host), SCARG(uap, hostlen),
				       SCARG(uap, serv), SCARG(uap, servlen));
	if ( error != 0 ) {
		return error;
	}
	if ( SCARG(uap, timeout) != NULL ) {
		error = ChkSpaceR(SCARG(uap, timeout), sizeof(struct timeval));
		if ( error < E_OK ) {
			return EFAULT;
		}
	}

	CALPKT cp;
	cp.req.fn = NET_SO_GETNAMEINFO_FN;
	cp.req.a.getnameinfo = uap->a;

	TMO_U tmo = SCARG(uap, timeout) == NULL ? TMO_FEVR : (TMO_U)SCARG(uap, timeout)->tv_sec * 1000000 + SCARG(uap, timeout)->tv_usec;

	int re = call_netdmn(&cp, tmo);

	*retval = cp.res.error;
	return re;
}

int
sys_getnameinfo_ms(struct lwp *l, const struct sys_getnameinfo_ms_args *uap, register_t *retval)
{
	int error;

	(void)l;

	error = check_getnameinfo_args(SCARG(uap, sa), SCARG(uap, salen),
				       SCARG(uap, host), SCARG(uap, hostlen),
				       SCARG(uap, serv), SCARG(uap, servlen));
	if ( error != 0 ) {
		return error;
	}

	CALPKT cp;
	cp.req.fn = NET_SO_GETNAMEINFO_FN;
	cp.req.a.getnameinfo.sa = SCARG(uap, sa);
	cp.req.a.getnameinfo.salen = SCARG(uap, salen);
	cp.req.a.getnameinfo.host = SCARG(uap, host);
	cp.req.a.getnameinfo.hostlen = SCARG(uap, hostlen);
	cp.req.a.getnameinfo.serv = SCARG(uap, serv);
	cp.req.a.getnameinfo.servlen = SCARG(uap, servlen);
	cp.req.a.getnameinfo.flags = SCARG(uap, flags);

	TMO_U tmo = (TMO_U)SCARG(uap, tmout);
	if ( tmo > 0 ) {
		tmo = tmo * 1000;
	}

	int re = call_netdmn(&cp, tmo);

	*retval = cp.res.error;
	return re;
}

int
sys_getnameinfo_us(struct lwp *l, const struct sys_getnameinfo_us_args *uap, register_t *retval)
{
	int error;

	(void)l;

	error = check_getnameinfo_args(SCARG(uap, sa), SCARG(uap, salen),
				       SCARG(uap, host), SCARG(uap, hostlen),
				       SCARG(uap, serv), SCARG(uap, servlen));
	if ( error != 0 ) {
		return error;
	}

	CALPKT cp;
	cp.req.fn = NET_SO_GETNAMEINFO_FN;
	cp.req.a.getnameinfo.sa = SCARG(uap, sa);
	cp.req.a.getnameinfo.salen = SCARG(uap, salen);
	cp.req.a.getnameinfo.host = SCARG(uap, host);
	cp.req.a.getnameinfo.hostlen = SCARG(uap, hostlen);
	cp.req.a.getnameinfo.serv = SCARG(uap, serv);
	cp.req.a.getnameinfo.servlen = SCARG(uap, servlen);
	cp.req.a.getnameinfo.flags = SCARG(uap, flags);

	TMO_U tmo = SCARG(uap, tmout_u);

	int re = call_netdmn(&cp, tmo);

	*retval = cp.res.error;
	return re;
}

int
sys_resctl(struct lwp *l, const struct sys_resctl_args *uap, register_t *retval)
{
	ER ercd;

	(void)l;

	if ( (SCARG(uap,cmd) & _RESCTL_OUT) != 0 && SCARG(uap,buf) != NULL ) {
		ercd = ChkSpaceRW(SCARG(uap, buf), SCARG(uap, bufsz));
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}
	else if( (SCARG(uap,cmd) & _RESCTL_IN) != 0 && SCARG(uap,buf) != NULL ) {
		ercd = ChkSpaceR(SCARG(uap, buf), SCARG(uap, bufsz));
		if ( ercd < E_OK ) {
			return EFAULT;
		}
	}

	CALPKT cp;
	cp.req.fn = NET_SO_RESCTL_FN;
	cp.req.a.resctl.cmd = SCARG(uap, cmd);
	cp.req.a.resctl.buf = SCARG(uap, buf);
	cp.req.a.resctl.bufsz = SCARG(uap, bufsz);

	int re = call_netdmn(&cp, TMO_FEVR);

	*retval = cp.res.len;
	return re;
}

IMPORT int res_addserver(const struct sockaddr *addr, size_t size);
IMPORT int res_delserver(const struct sockaddr *addr, size_t size);
IMPORT int res_listservers(struct sockaddr **addrs, size_t bufsz, size_t *len);
IMPORT int res_flushservers(void);
IMPORT int res_adddomain(const char* domain);
IMPORT int res_deldomain(const char* domain);
IMPORT int res_listdomains(char** domains, size_t size, size_t *len);
IMPORT int res_flushdomains(void);

LOCAL int resctl(int cmd, void* buf, size_t bufsz, size_t *len)
{
	int error;
	*len = 0;

	switch(cmd) {
	case SO_RES_ADD_TABLE:
		error = tkn_resctl_add_table((struct hosttable*)buf, bufsz);
		break;
	case SO_RES_DEL_TABLE:
		error = tkn_resctl_del_table((struct hosttable*)buf, bufsz);
		break;
	case SO_RES_GET_TABLES:
		error = tkn_resctl_get_tables((struct hosttable*)buf, bufsz, len);
		break;
	case SO_RES_FLUSH_TABLES:
		error = tkn_resctl_flush_tables();
		break;
	case SO_RES_ADD_SERVER:
		error = res_addserver((struct sockaddr*)buf, bufsz);
		break;
	case SO_RES_DEL_SERVER:
		error = res_delserver((struct sockaddr*)buf, bufsz);
		break;
	case SO_RES_GET_SERVERS:
		error = res_listservers((struct sockaddr**)buf, bufsz, len);
		break;
	case SO_RES_FLUSH_SERVERS:
		error = res_flushservers();
		break;
	case SO_RES_ADD_DOMAIN:
		error = res_adddomain((char*)buf);
		break;
	case SO_RES_DEL_DOMAIN:
		error = res_deldomain((char*)buf);
		break;
	case SO_RES_GET_DOMAINS:
		error = res_listdomains((char**)buf, bufsz, len);
		break;
	case SO_RES_FLUSH_DOMAINS:
		error = res_flushdomains();
		break;
	default:
		error = EINVAL;
	}

	return error;
}
