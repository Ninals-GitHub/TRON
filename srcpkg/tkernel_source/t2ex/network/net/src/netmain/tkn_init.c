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
 *	@(#)tkn_init.c
 *
 */

#include <tk/tkernel.h>

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/domain.h>
#include <sys/mbuf.h>
#include <sys/socketvar.h>
#if NBPFILTER > 0
#include <net/bpf.h>
#endif
#include <net/netisr.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/if_inarp.h>
#include <sys/syslog.h>
#include <sys/uidinfo.h>

#include <netmain/if_tkn.h>
#include "netmain/tkn_init.h"
#include "netmain/tkn_malloc.h"
#include "netmain/tkn_clock.h"
#include "netmain/tkn_spl.h"

#include <sys/sleepq.h>
#include <sys/proc.h>
#include <sys/filedesc.h>
#include <sys/percpu.h>

#include "../tkn.h"

/* Functions */
static	ER	tkn_create_task(void);
static	ER	tkn_delete_task(void);
static	ER	tkn_start_task(void);
static ER	tkn_stop_task(void);

IMPORT ER tkn_mutex_initialize(void);
IMPORT ER tkn_mutex_finish(void);
IMPORT ER tkn_condvar_init(void);
IMPORT ER tkn_condvar_finish(void);
IMPORT ER tkn_lwp_init(void);
IMPORT ER tkn_lwp_setfd(void);
IMPORT ER tkn_lwp_finish(void);
IMPORT filedesc_t* filedesc0;
IMPORT sleeptab_t	sleeptab;

#ifdef TKN_MONITOR
void	tkn_monitorInit(void);
#endif

#if NTUN > 0
void	tunattach(int unused);
void	tundetach(int unused);
#endif

/*
 *
 * Finalize the network communication function.
 */
int
tkn_finish(void)
{
	int s;
	int ret = 0;

	ret |= finish_netdmn();
	ret |= tkn_stop_task();
#if NBPFILTER > 0
	ret |= bpfilterdetach(1);
#endif
#if NTUN > 0
	tundetach(0);
#endif

	ret |= loopdetach(1);

	s = splnet();
	ret |= if_detachdomain();
	ret |= domainfinish();
	ret |= iffinish();
	splx(s);

	ret |= tkn_delete_task();

	fd_free();

	ret |= selsysfinish(NULL);
	ret |= pool_cache_cpu_finish(NULL);
	ret |= callout_finish_cpu(NULL);
	ret |= softint_finish(NULL);
	ret |= sofinish();
	ret |= tkn_lwp_finish();
	ret |= fd_sys_finish();
	ret |= mbfinish();
	uid_finish();
	ret |= pool_subsystem_finish();
	ret |= tkn_clock_unregister();
	ret |= tkn_kmem_finish();
	ret |= sleeptab_finish(&sleeptab);
	ret |= callout_finish();
	ret |= percpu_finish();
	ret |= tkn_condvar_finish();
	ret |= tkn_mutex_finish();
	ret |= tkn_spl_finish();

	return ret;
}

/*
 * Initialize the network communication function.
 */
int
tkn_initialize(void)
{
	int  s;
	int  ret;

	ret = tkn_spl_init();
	if ( ret != 0 ) {
		goto err_ret0;
	}

	ret = tkn_mutex_initialize();
	if ( ret != 0 ) {
		goto err_ret1;
	}

	ret = tkn_condvar_init();
	if ( ret != 0 ) {
		goto err_ret2;
	}

	ret = percpu_init();
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret3;
	}

	ret = callout_startup();
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret4;
	}

	ret = sleeptab_init(&sleeptab);
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret5;
	}

	hardclock_ticks = 0x7FFFC000;

	ret = tkn_kmem_init();
	if ( ret != 0 ) {
		goto err_ret6;
	}
	ret = tkn_clock_register();
	if ( ret != 0 ) {
		goto err_ret7;
	}
	ret = pool_subsystem_init();
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret8;
	}
	uid_init();

	ret = mbinit();
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret9;
	}
	ret = fd_sys_init();
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret10;
	}

	ret = tkn_lwp_init();
	if ( ret != 0 ) {
		goto err_ret11;
	}

	filedesc0 = fd_init(NULL);
	if ( filedesc0 == NULL ) {
		ret = ERRNOtoER(ENOMEM);
		goto err_ret12;
	}

	ret = tkn_lwp_setfd();
	if ( ret != 0 ) {
		goto err_ret13;
	}


	ret = soinit();
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret13;
	}

	/* Initialize per cpu data */
	ret = softint_init(NULL);
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret14;
	}
	ret = callout_init_cpu(NULL);
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret15;
	}
	ret = pool_cache_cpu_init(NULL);
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret16;
	}
	ret = selsysinit(NULL);
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret17;
	}

	/*
	 * Create network tasks
	 */
	if ((ret = tkn_create_task()) != E_OK) {
		goto err_ret18;
	}

	/*
	 * Initialize protocols
	 */
	s = splnet();
	ifinit();
	ret = domaininit();
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret19;
	}
	if_attachdomain();
	splx(s);

	/* Configure the loopback device */
	log(LOG_INFO, "[TKN] attach loopback: lo0\n");
	ret = loopattach(1);
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret20;
	}

#if NTUN > 0
	tunattach(1);
#endif
#if NBPFILTER > 0
	ret = bpfilterattach(1);
	if ( ret != 0 ) {
		ret = ERRNOtoER(ret);
		goto err_ret21;
	}
#endif

	/*
	 * Start network tasks
	 */
	if ((ret = tkn_start_task()) != E_OK) {
		goto err_ret22;
	}

	/*
	 * Initialize the network daemon task
	 */
	ret = init_netdmn();
	if ( ret != 0 ) {
		goto err_ret23;
	}

	return E_OK;

err_ret23:
	tkn_stop_task();
err_ret22:
#if NBPFILTER > 0
	bpfilterdetach(1);
err_ret21:
#endif
#if NTUN > 0
	tundetach(1);
#endif
	loopdetach(1);
err_ret20:
	s = splnet();
	if_detachdomain();
	domainfinish();
err_ret19:
	iffinish();
	splx(s);
	tkn_delete_task();
err_ret18:
	selsysfinish(NULL);
err_ret17:
	pool_cache_cpu_finish(NULL);
err_ret16:
	callout_finish_cpu(NULL);
err_ret15:
	softint_finish(NULL);
err_ret14:
	sofinish();
err_ret13:
	fd_free();
err_ret12:
	tkn_lwp_finish();
err_ret11:
	fd_sys_finish();
err_ret10:
	mbfinish();
err_ret9:
	uid_finish();
	pool_subsystem_finish();
err_ret8:
	tkn_clock_unregister();
err_ret7:
	tkn_kmem_finish();
err_ret6:
	sleeptab_finish(&sleeptab);
err_ret5:
	callout_finish();
err_ret4:
	percpu_finish();
err_ret3:
	tkn_condvar_finish();
err_ret2:
	tkn_mutex_finish();
err_ret1:
	tkn_spl_finish();
err_ret0:
	return ret;
}

static void
tkn_callout_hardclock_entry(VP exinf)
{
	(void)exinf;

	tkn_clockintr();
}

static ID
make_timewheel_kicker(void)
{
	T_CCYC ccyc = {
		.exinf = NULL,
		.cycatr = TA_HLNG,
		.cychdr = tkn_callout_hardclock_entry,
		.cyctim = 5,
	};

	return tk_cre_cyc(&ccyc);
}

/*
 * Create a clock tasks
 */
static int
tkn_create_task(void)
{
	int rtn;

	rtn = make_timewheel_kicker();
	if (rtn < E_OK) {
		log(LOG_ERR, "[TKN] create hard clock task err=%d\n", rtn);
		return rtn;
	}
	tkn_nif_mng.nifm_hardclock = rtn;

	return E_OK;
}

/*
 * Start a clock tasks
 */
static int
tkn_start_task(void)
{
	int rtn;

	rtn = tk_sta_cyc(tkn_nif_mng.nifm_hardclock);
	if (rtn != E_OK) {
		log(LOG_ERR, "[TKN] start hard clock task error=%d\n", rtn);
		return rtn;
	}

	return rtn;
}

static ER
tkn_stop_task(void)
{
	return tk_stp_cyc(tkn_nif_mng.nifm_hardclock);
}


/*
 * delete a clock tasks
 */
static ER
tkn_delete_task(void)
{
	return tk_del_cyc(tkn_nif_mng.nifm_hardclock);
}
