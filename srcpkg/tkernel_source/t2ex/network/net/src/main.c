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
 *	@(#)main.c
 *
 */

#include <tk/tkernel.h>
#include <sys/commarea.h>
#include <sys/util.h>

#include "fnsocket.h"

#include "rominfo.h"

#define DEBUG_PRINT(x)

#include <sys/syscall.h>
#include <sys/queue.h>
#include <strings.h>

#include <netmain/tkn_init.h>
#include <netmain/if_tkn.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/filedesc.h>
#include <t2ex/socket.h>
#include <t2ex/errno.h>

#include "tkn.h"

IMPORT ER ChkT2EXLevel( void );

/*
 * SVC handler
 */
EXPORT ER SyscallEntry( VP para, W fn )
{
	FunctionNumber	fno;
	int		error;
	int		ret = 0;

	error = ChkT2EXLevel();
	if (error < E_OK) {
		goto err_ret;
	}

	/* Invoke a system call. */
	fno.w = fn;
	error = tkn_syscall(fno.efn.funcno, para, &ret);
	if (error != 0) goto err_ret;

	return ret;

err_ret:
	DEBUG_PRINT(("SyscallEntry fn=%#x err=%#x\n", fn, error));
	/*
	 * Convert a error number (POSIX-style) into a error code
	 * (T-Kernel-style) if error represents a error number.
	 */
	if (error > 0) {
		error = ERRNOtoER(error);
	}
	return error;
}

/*
 * System configuration information
 */
IMPORT u_int maxfiles;
IMPORT INT tkn_task_base_pri;
IMPORT INT tkn_dev_rbuf_num;
IMPORT int tcp_sendspace;
IMPORT int tcp_recvspace;
IMPORT int udp_sendspace;
IMPORT int udp_recvspace;
IMPORT int rip_sendspace;
IMPORT int rip_recvspace;
IMPORT int raw_sendspace;
IMPORT int raw_recvspace;
IMPORT int tcp_do_autosndbuf;
IMPORT int tcp_autosndbuf_inc;
IMPORT int tcp_autosndbuf_max;
IMPORT int tcp_do_autorcvbuf;
IMPORT int tcp_autorcvbuf_inc;
IMPORT int tcp_autorcvbuf_max;

LOCAL ER read_sysconf( void )
{
	INT i;

	/* Read system information. */
	i = tk_get_cfn(SCTAG_SOMAXSOCKET, &maxfiles, 1);
	if (i < 1) {
		return E_SYS;
	}
	maxfiles += NDFDFILE;
	i = tk_get_cfn(SCTAG_SOTASKBASEPRI, &tkn_task_base_pri, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SODRVRXBUFNUM, &tkn_dev_rbuf_num, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOTCPTXBUFSZ, &tcp_sendspace, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOTCPRXBUFSZ, &tcp_recvspace, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOUDPTXBUFSZ, &udp_sendspace, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOUDPRXBUFSZ, &udp_recvspace, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SORAWIPTXBUFSZ, &rip_sendspace, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SORAWIPRXBUFSZ, &rip_recvspace, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SORAWTXBUFSZ, &raw_sendspace, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SORAWRXBUFSZ, &raw_recvspace, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOTCPDOAUTOTX, &tcp_do_autosndbuf, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOTCPINCAUTOTX, &tcp_autosndbuf_inc, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOTCPMAXAUTOTX, &tcp_autosndbuf_max, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOTCPDOAUTORX, &tcp_do_autorcvbuf, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOTCPINCAUTORX, &tcp_autorcvbuf_inc, 1);
	if (i < 1) {
		return E_SYS;
	}
	i = tk_get_cfn(SCTAG_SOTCPMAXAUTORX, &tcp_autorcvbuf_max, 1);
	if (i < 1) {
		return E_SYS;
	}

	return E_OK;
}

/* ------------------------------------------------------------------------ */

EXPORT	INT	tkn_block_size;

/*
 * Lock for mutual exclusion.
 */
EXPORT	FastMLock  TKNLock;

/*
 * Initialize the network communication manager.
 */
LOCAL ER initialize( void )
{
	ER	err;

	T_DSSY	dssy;
	T_RSMB	rsmb;

	err = read_sysconf();
	if ( err < E_OK ) {
		return err;
	}

	err = tk_ref_smb(&rsmb);
	if ( err < E_OK ) {
		return err;
	}
	tkn_block_size = rsmb.blksz;

	err = CreateMLock(&TKNLock, (CONST UB*)"Ngbl");
	if ( err < E_OK ) {
		return err;
	}

	err = tkn_initialize();
	if ( err < E_OK ) {
		return err;
	}

	dssy.ssyatr = TA_NULL;
	dssy.ssypri = NET_PRI;
	dssy.svchdr = (FP)SyscallEntry;
	dssy.breakfn = NULL;
	dssy.startupfn = NULL;
	dssy.cleanupfn = NULL;
	dssy.eventfn = NULL;
	dssy.resblksz = 0;
	err = tk_def_ssy(NET_SVC, &dssy);
	if ( err < E_OK ) {
		return err;
	}

	return E_OK;

}

/*
 * Terminate the network communication manager.
 */
LOCAL ER finish( void )
{
	struct tkn_nif_info *nif;
	ER err = 0;

	err |= so_resctl(SO_RES_FLUSH_DOMAINS, NULL, 0);
	err |= so_resctl(SO_RES_FLUSH_TABLES, NULL, 0);
	err |= so_resctl(SO_RES_FLUSH_SERVERS, NULL, 0);

	err |= tk_def_ssy(NET_SVC, NULL);

	for (nif = tkn_nif_mng.nifm_list; nif != NULL; nif = nif->nif_next) {
		err |= tkn_nif_detach(nif);
	}

	err |= tkn_finish();

	DeleteMLock(&TKNLock);

	return err;
}

/*
 * Network communication functions entry.
 */
EXPORT ER so_main( INT ac, UB* arg[] )
{
	ER	err = E_OK;

	(void)arg; /* UNUSED */

	if ( ac >= 0 ) {
		/* Initialize the network communication manager. */
		err = initialize();
	} else {
		/* Terminate the network communication manager. */
		err = finish();
	}

	return err;
}
