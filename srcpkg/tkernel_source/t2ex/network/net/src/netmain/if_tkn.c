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
 *	@(#)if_tkn.c
 *
 */

/*
 * Network interface for the T-Kernel standard LAN driver
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/cdefs.h>
#include <sys/kernel.h>
#include <sys/syslog.h>
#include <sys/sockio.h>

#include <net/if.h>
#include <netmain/if_tkn.h>
#if NBPFILTER > 0
#include <net/bpf.h>
#endif

#include <tk/tkernel.h>
#include <tk/devmgr.h>
#include <device/netdrv.h>

extern pool_cache_t mb_cache;

#include "netmain/tkn_init.h"
#include "netmain/tkn_taskutil.h"
#include "netmain/tkn_spl.h"

#include "tkn.h"

#define ETHER_CMP(a, b)       memcmp(a, b, ETHER_ADDR_LEN)

#ifdef T2EX_LOG
static int tkn_msg_flag = LOG_ERR;
#endif

#ifndef TKN_ETHERNET_MTU
#define TKN_ETHERNET_MTU	ETHERMTU		/* mtu */
#endif

#define	WATCHDOG_TIME		( 3 / IFNET_SLOWHZ )	/* 3 seconds */

static int	tkn_init(struct ifnet *ifp);
static void	tkn_start(struct ifnet *ifp);
static void	tkn_stop(struct ifnet *ifp, int disable);

/*
 * Network interface manager
 */
struct tkn_nif_mng tkn_nif_mng = {
	.nifm_status = 0,
	.nifm_nifmax = 0,
	.nifm_list   = NULL,

	.nifm_hardclock  = 0,
};

/*
 * Receive/Transmit buffer allocation management.
 */
#include <sys/memalloc.h>

static MACB	txrxbufcb;

/*
 * Initialize receive/transmit buffer allocation management
 */
static void
txrxbuf_init( void )
{
	/* Check if the control block has been already initialized. */
	if ( AlignMACB(&txrxbufcb)->pagesz > 0 ) return;

	//_tkm_init(TA_RNG0|_MEM_MINPAGESZ, &txrxbufcb);
	_tkm_init(TA_RNG0, &txrxbufcb);

	//if ( _mem_minpagesz < unitsz ) _mem_minpagesz = unitsz;
}

/*
 * Retrive a receive/transmit buffer.
 */
static struct tkn_extbuf*
txrxbuf_alloc( void )
{
	return _mem_malloc(sizeof(struct tkn_extbuf), &txrxbufcb);
}

/*
 * Release a receive/transmit buffer.
 */
static void
txrxbuf_free( struct tkn_extbuf *p )
{
	_mem_free(p, &txrxbufcb);
}

/*
 * Get a resource ID.
 */
static ID
new_resource_id(void)
{
	return tk_get_rid(TSK_SELF);
}

/*
 * Create a message buffer.
 */
static ID
new_message_buffer(void)
{
	return tkn_cre_mbf(sizeof(NetEvent),
			   sizeof(NetEvent) * tkn_dev_rbuf_num * 2,
			   "network");
}

/*
 * Change the resource group to control the NIC device.
 */
static ID
device_enter(struct tkn_nif_info *nifp)
{
	return tk_set_rid(TSK_SELF, nifp->nif_resid);
}

/*
 * Restore the resource group.
 */
static void
device_leave(const struct tkn_nif_info *nifp, ID oldres)
{
	(void)nifp;
	tk_set_rid(TSK_SELF, oldres);
}

#define with_device_res(name, p)					\
	for (name = device_enter(p); name; device_leave(p, name), name = 0)

/*
 * Read a physical address from the NIC device.
 */
static ER
device_enaddr(ID devid, uint8_t *enaddr)
{
	W asize;
	ER er = tk_srea_dev(devid, DN_NETADDR, enaddr, ETHER_ADDR_LEN,
	    &asize);
	if (er < 0)
		goto err_dev;
	return 0;

err_dev:
	return er;
}

/*
 * Clear a multicast address filter.
 *
 */
static ER
device_clear_mcast_list(ID devid)
{
	ER er;
	W asize;

	er = tk_swri_dev(devid, DN_NETMCASTLIST, NULL, 0, &asize);
	if (er < 0)
		goto err_dev;
	return 0;

err_dev:
	return er;
}

/*
 * Set a hardware media type.
 */
static int
device_mediatype(ID devid, uint *media)
{
	NetDevInfo devinfo;
	W asize;
	ER er = tk_srea_dev(devid, DN_NETDEVINFO, &devinfo, sizeof(devinfo),
	    &asize);
	if (er < 0)
		goto err_dev;

	if (media) {
		uint typ;
		switch (devinfo.ifconn) {
		case IFC_UNKNOWN:
		default:
			/* Unknown                      */
			typ = IFM_NONE;
			break;

		case IFC_AUI:
			/* AUI (10 Base 5)              */
			typ = IFM_MANUAL | IFM_10_5;
			break;

		case IFC_TPE:
			/* TPE (10 Base T)              */
			typ = IFM_MANUAL | IFM_10_T;
			break;

		case IFC_BNC:
			/* BNC (10 Base 2)              */
			typ = IFM_MANUAL | IFM_10_2;
			break;

		case IFC_100TX:
			/* 100 Base TX                  */
			typ = IFM_MANUAL | IFM_100_TX;
			break;

		case IFC_100FX:
			/* 100 Base FX                  */
			typ = IFM_MANUAL | IFM_100_FX;
			break;

		case IFC_AUTO:
			/* Auto                         */
			typ = IFM_AUTO;
			break;
		}
		*media = typ;
	}
	if (devinfo.stat < 0)
		goto err_stat;
	return devinfo.stat;

err_stat:
	er = devinfo.stat;
	log(tkn_msg_flag,
	    "[TKN %s] get ifmedia, status=%d\n", __func__, er);
	return er;
err_dev:
	log(tkn_msg_flag, "[TKN %s] dev read error=%d\n", __func__, er);
	return er;
}

/*
 * Open a NIC device.
 */
static ID
device_open(const struct tkn_nif_info *nifp)
{
	ER er = tk_opn_dev(nifp->nif_dev, TD_UPDATE/*|TD_WEXCL*/);
	if (er < 0)
		goto err_open;
	return er;

err_open:
	return er;
}

/*
 * Close a NIC device.
 */
static ER
device_close(ID devid)
{
	ER er = tk_cls_dev(devid, 0);
	if (er < 0)
		goto err_close;
	return 0;

err_close:
	return er;
}

/*
 * Send a data to the NIC device.
 */
static ER
device_send_data(const struct tkn_nif_info *nifp, void *buf, int siz)
{
	ER er;
	W asize;

	er = tk_swri_dev(nifp->nif_dev_id, 0, buf, siz, &asize);
	if (er < 0)
		goto err_send_data;
	return asize;

err_send_data:
	log(tkn_msg_flag, "[TKN %s] error=%d size=%d", __func__, er, siz);
	return er;
}

/*
 * Set a message buffer ID for event notification.
 */
static int
device_attach_msgid(const struct tkn_nif_info *nifp)
{
	ER  er;
	W asize;
	ID msgid = nifp->nif_msg_id;

	er = tk_swri_dev(nifp->nif_dev_id, DN_NETEVENT, &msgid, sizeof(ID),
	    &asize);
	if (er < 0)
		goto err_set_msg_id;
	return 0;

err_set_msg_id:
	return er;
}

/*
 * Get a maximum size of receive buffer.
 */
static int
device_get_receive_size_max(const struct tkn_nif_info *nifp)
{
	ER  er;
	W result;

	NetRxBufSz bufsize;
	er = tk_srea_dev(nifp->nif_dev_id, DN_NETRXBUFSZ, &bufsize,
	    sizeof(bufsize), &result);
	if (er < E_OK)
		goto err_device_read;

	//log(tkn_msg_flag, " rcvbuf={min=%d max=%d}", bufsize.minsz,
	//    bufsize.maxsz);
	return bufsize.maxsz;

err_device_read:
	log(tkn_msg_flag, "[TKN %s] DN_NETRXBUFSZ read error %d\n",
	    __func__, er);
	return er;
}

/*
 * Add a receive buffer to the LAN driver.
 */
static int
device_attach_rxbufptr(struct tkn_nif_info *nifp, struct tkn_extbuf *eb)
{
	ER  er;
	W asize;

	if ( eb == NULL ) {
		eb = txrxbuf_alloc();
		if ( eb == NULL ) { er = E_NOMEM; goto err_nomem; }
		QueInsert(&eb->q, tkn_rxqh(nifp));
	}

	eb->nifp = (void *)nifp;

	void *ptr = tkn_extbuf_to_data(eb);
	er = tk_swri_dev(nifp->nif_dev_id, DN_NETRXBUF, &ptr, sizeof(ptr),
	    &asize);
	if (er < 0)
		goto err_device;
	return 0;

err_device:
	QueRemove(&eb->q);
	txrxbuf_free(eb);
err_nomem:
	log(tkn_msg_flag, "[TKN]%s: dev#%d: DN_NETRXBUF %d\n", __func__,
	    nifp->nif_dev_id, er);
	return er;
}

/*
 * Removes receive buffers from the LAN driver.
 */
static int
device_detach_rxbuf(struct tkn_nif_info *nifp)
{
	ER er;
	W asize;

	void *ptr = NULL;
	er = tk_swri_dev(nifp->nif_dev_id, DN_NETRXBUF, &ptr, sizeof(ptr),
	    &asize);
	if (er < 0)
		goto err_device;

	struct queue *q;
	while ( (q = QueRemoveNext(tkn_rxqh(nifp))) != NULL ) {
		txrxbuf_free((struct tkn_extbuf*)q);
	}

	return 0;

err_device:
	return er;
}

/*
 * Set receive buffers to the LAN driver.
 */
static int
device_attach_rxbuf(struct tkn_nif_info *nifp)
{
	int	i;
	ER	er;

	for ( i = 0; i < tkn_dev_rbuf_num; ++i ) {
		er = device_attach_rxbufptr(nifp, NULL);
		if ( er < E_OK ) break;
	}

	return i;
}

static int
tkn_mediachange(struct ifnet *ifp)
{
	(void)ifp;
	return 0;
}

static void
tkn_mediastatus_ether(struct ifnet *ifp, struct ifmediareq *req)
{
	struct tkn_nif_info *nifp = ifp->if_softc;
	ID oldres;

	with_device_res(oldres, nifp) {
		req->ifm_status = IFM_AVALID;
		if (0 < nifp->nif_dev_id
		    && 0 <= device_mediatype(nifp->nif_dev_id, NULL))
			req->ifm_status |= IFM_ACTIVE;
	}
}

/*
 * Initialize variables for media status.
 */
static int
tkn_init_ifmedia_ether(struct ifmedia * ifmedia)
{
	int error;

	ifmedia_init(ifmedia, IFM_IMASK, tkn_mediachange,
	    tkn_mediastatus_ether);
	error = ifmedia_add(ifmedia, IFM_ETHER | IFM_AUTO, 0, NULL);
	if ( error != 0 ) {
		goto err;
	}
	error = ifmedia_set(ifmedia, IFM_ETHER | IFM_AUTO);
	if ( error != 0 ) {
		goto err;
	}

	return 0;

err:
	ifmedia_removeall(ifmedia);
	return error;
}

static void
tkn_finish_ifmedia(struct ifmedia * ifmedia)
{
	ifmedia_removeall(ifmedia);
}

static void driver_task(INT, VP);

/*
 * Create a driver event task
 */
static ER
new_driver_task(struct tkn_nif_info *nif_info)
{
	int prio = tkn_spl_priority(IPL_NONE);
	ID tid;

	tid = tkn_cre_tsk(driver_task, prio, TKN_DRVTSK_STKSZ, nif_info);
	if (tid < 0) {
		log(LOG_ERR, "[TKN %s] %d\n", __func__, tid);
	}
	return tid;
}

/*
 * Timeout waiting for transmit completion notification.
 */
void
tkn_watchdog(struct ifnet *ifp)
{
	// log(tkn_msg_flag, "[TKN %s]\n", __func__);
	ifp->if_flags &= ~IFF_OACTIVE;
	ifp->if_timer=0;
	ifp->if_start(ifp);

	return;
}

static int
tkn_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	log(LOG_DEBUG, "ENTER %s cmd=%lx %c %d\n", __func__, cmd,
	    (int)IOCGROUP(cmd), (int)(cmd & 0xFF));

	int s = splnet();

	int error = 0;
	struct ifreq  *ifr = (struct ifreq *)data;
	struct tkn_nif_info *nifp = ifp->if_softc;

	switch (cmd) {
	case SIOCSIFMEDIA:
	case SIOCGIFMEDIA:
	{
		struct ifmedia *ifmedia = &nifp->nif_media;
		error = ifmedia_ioctl(ifp, ifr, ifmedia, cmd);
		break;
	}

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		error = EOPNOTSUPP;
		break;

	default:
		error = ether_ioctl(ifp, cmd, data);
	}

	splx(s);
	return error;
}

/*
 * Attach a network interface
 */
int
tkn_nif_attach(struct tkn_nif_info *nif_info)
{
	int s;
	int ret;
	ER er;

	if (nif_info == NULL) {
		goto err_param;
	}

	er = new_resource_id();
	if (er < 0) {
		log(LOG_ERR, "[TKN %s] get resource failed: %d\n", __func__, er);
		goto err_resid;
	}
	nif_info->nif_resid = er;
#if T2EX_LOG
	log(LOG_DEBUG, " extbuf=%u ehdrlen=%d resid=%d",
	    sizeof(struct tkn_extbuf),
	    ETHER_HDR_LEN, nif_info->nif_resid);
#endif

	er = new_message_buffer();
	if (er < 0) {
		log(LOG_ERR, "[TKN %s] get message buffer error: %d\n", __func__, er);
		goto err_message_buffer;
	}
	nif_info->nif_msg_id = er;

	er = new_driver_task(nif_info);
	if (er < 0) {
		log(LOG_ERR, "[TKN %s] new driver task error %d\n", __func__, er);
		goto err_driver_task;
	}
	nif_info->nif_tid = er;

	/* Initialize receive/transmit buffer. */
	txrxbuf_init();
	QueInit(tkn_rxqh(nif_info));
	nif_info->tx.ebuf = txrxbuf_alloc();
	if ( nif_info->tx.ebuf == NULL ) {
		goto err_txbuffer;
	}

	/* Get NIC device information. */
	ID nicid = device_open(nif_info);
	if (nicid < 0) {
		log(LOG_ERR, "[TKN %s] nic open error %d\n", __func__, nicid);
		goto err_nic_open;
	}

	uint8_t en[ETHER_ADDR_LEN];
	er = device_enaddr(nicid, en);
	if (er < 0) {
		log(LOG_ERR, "[TKN %s] Query Ethernet address failed.\n", __func__);
		goto err_ethernet_address;
	}
	log(LOG_DEBUG, " MAC=%s", ether_sprintf(en));

	uint mtype;
	er = device_mediatype(nicid, &mtype);
	if (er < 0) {
		goto err_ethernet_kind;
	}

	device_clear_mcast_list(nicid);

	device_close(nicid);
	nicid = -1;

	struct ifnet *ifp = &nif_info->nif_ifnet;

	/* Ethernet */
	er = tkn_init_ifmedia_ether(&nif_info->nif_media);
	if ( er != 0 ) {
		goto err_media_init;
	}
	//ifp->if_baudrate = ifmedia_baudrate(nif_info->nif_media.ifm_media);

	strlcpy(ifp->if_xname, (const char*)nif_info->nif_dev, IFNAMSIZ);

	ifp->if_softc    = &nif_info->nif_ec;
	ifp->if_flags    = IFF_BROADCAST | IFF_SIMPLEX;
	ifp->if_start    = tkn_start;
	ifp->if_ioctl    = (int (*)(struct ifnet *, u_long, void *)) tkn_ioctl;
	ifp->if_init     = tkn_init;
	ifp->if_stop     = tkn_stop;
	ifp->if_drain    = if_nulldrain;
	ifp->if_mtu      = TKN_ETHERNET_MTU;
	ifp->if_watchdog = tkn_watchdog;
	IFQ_SET_READY(&ifp->if_snd);

	ret = if_attach(ifp);
	if ( ret != 0 ) {
		goto err_if_attach;
	}
	ret = ether_ifattach(ifp, en);
	if ( ret != 0 ) {
		goto err_ether_ifattach;
	}

	s = splnet();

	nif_info->nif_next = tkn_nif_mng.nifm_list;
	tkn_nif_mng.nifm_list = nif_info;
	tkn_nif_mng.nifm_nifmax++;

	splx(s);

	/* Start a driver event task. */
#ifdef DEBUG
	printf("start driver task #%d\n", nif_info->nif_tid);
#endif
	ret = tk_sta_tsk(nif_info->nif_tid, (INT)nif_info);
	log(LOG_DEBUG, " driver=task#%d", nif_info->nif_tid);

	return E_OK;

err_ether_ifattach:
	if_detach(ifp);
err_if_attach:
	tkn_finish_ifmedia(&nif_info->nif_media);
err_media_init:
err_ethernet_kind:
err_ethernet_address:
	if (0 < nicid) {
		device_close(nicid);
	}
err_nic_open:
	txrxbuf_free(nif_info->tx.ebuf);
err_txbuffer:
	tk_del_tsk(nif_info->nif_tid);
err_driver_task:
	tk_del_mbf(nif_info->nif_msg_id);
err_message_buffer:
err_resid:
	return er;

err_param:
	log(LOG_ERR, "[TKN %s] params nifinfo NULL\n", __func__);
	return TKN_ERR_PARAM;
}

/*
 * Detach a network interface
 */
int
tkn_nif_detach(struct tkn_nif_info *nif_info)
{
	int s;

	if (nif_info == NULL) {
		goto err_param;
	}

	s = splnet();

	/* delete message buffer (and let driver event task exit and deleted) */
	tk_del_mbf(nif_info->nif_msg_id);

	splx(s);

	return E_OK;

err_param:
	log(LOG_ERR, "[TKN %s] params nifinfo NULL\n", __func__);
	return TKN_ERR_PARAM;
}

static int
nif_cleanup(struct tkn_nif_info *nif_info)
{
	struct tkn_nif_info *nif, *prev_nif;
	struct ifnet *ifp;

	prev_nif = NULL;
	for (nif = tkn_nif_mng.nifm_list; nif != NULL; nif = nif->nif_next) {
		if (nif == nif_info) {
			break;
		}
		prev_nif = nif;
	}

	ifp = &nif_info->nif_ifnet;

	tkn_stop(ifp, 1);

	if_down(ifp);

	/* Remove from the manager */
	if (prev_nif == NULL) {
		tkn_nif_mng.nifm_list = nif_info->nif_next;
	} else {
		prev_nif->nif_next = nif_info->nif_next;
	}
	tkn_nif_mng.nifm_nifmax--;

	// tkn_nif_delete_tunnel(ifp);
	ether_ifdetach(ifp);
	if_detach(ifp);
	//rtcache_free();

	tkn_finish_ifmedia(&nif_info->nif_media);

	/* Release receive/transmit buffers. */
	txrxbuf_free(nif_info->tx.ebuf);

	return E_OK;
}

static int
tkn_device_close(struct tkn_nif_info *nifp)
{
	log(LOG_DEBUG, "ENTER %s: nif_dev_id=%d\n", __func__,
	    nifp->nif_dev_id);

	if (nifp->nif_dev_id <= 0)
		goto err_not_open;

	ER er = -1;
	ID oldres;
	with_device_res(oldres, nifp) {
		device_detach_rxbuf(nifp);
		er = device_close(nifp->nif_dev_id);
	}
	if (er < 0)
		goto err_device_close;
	nifp->nif_dev_id = 0;

err_not_open:
	return 0;

err_device_close:
	return er;
}

/* ifnet.if_init routine */
static int
tkn_init(struct ifnet *ifp)
{
	log(LOG_DEBUG, "ENTER %s\n", __func__);

	struct tkn_nif_info *const nifp = ifp->if_softc;
	ER er;

	if (0 < nifp->nif_dev_id) {
		tkn_stop(ifp, 0);
		nifp->nif_dev_id = 0;
	}

	const ID oldres = device_enter(nifp);
	const INT s = splnet();

	const ID dev = er = device_open(nifp);
	if (er < 0) {
		log(tkn_msg_flag, "[TKN %s] device_open error %d\n",
		    __func__, er);
		goto err_device_open;
	}
	nifp->nif_dev_id = er;

	er = device_attach_msgid(nifp);
	if (er < 0) {
		log(tkn_msg_flag,
		    "[TKN %s] message buffer id attach error:%d\n",
		    __func__, er);
		goto err_attach_msgid;
	}

	const int rmax = device_get_receive_size_max(nifp);
	if (rmax < 0) {
		goto err_receive_max_size_check;
	}
	if (TKN_RXBUF_SIZE < rmax) {
		goto err_receive_max_size_over;
	}

	er = device_attach_rxbuf(nifp);
	if (er <= 0) {
		log(tkn_msg_flag,
		    "[TKN %s] error on set receive buffer\n", __func__);
		goto err_attach_rxbuf;
	}

	ifp->if_flags |= IFF_RUNNING;
	ifp->if_flags &= ~IFF_OACTIVE;

	er = 0;
	goto exit;

err_attach_rxbuf:
err_receive_max_size_over:
err_receive_max_size_check:
err_attach_msgid:
	device_close(dev);
	nifp->nif_dev_id = 0;
err_device_open:
	er = EIO;

exit:
	splx(s);
	device_leave(nifp, oldres);
	return er;
}

/*
 * Transmit a chain of mbufs.
 */
static int
tkn_transmit_chain(struct tkn_nif_info *nifp, struct mbuf *m)
{
	ER er;

	u_int const len = m_length(m);
	if (TKN_RXBUF_SIZE < len)
		goto err_too_big;

	void *buf = tkn_extbuf_to_data(nifp->tx.ebuf);
	m_copydata(m, 0, len, buf);

	er = device_send_data(nifp, buf, len);
	if (er < 0)
		goto err_send_data;
	return er;

err_send_data:
	return er;
err_too_big:
	log(tkn_msg_flag, "[TKN] too big packet=%u\n", len);
	return -1;
}

/*
 * Output routine for the LAN driver.
 */
static int
tkn_nif_output(struct tkn_nif_info *nifp)
{
	struct ifnet * const ifp = &nifp->nif_ifnet;
	struct mbuf	*m;
	ER		er;

	ifp->if_flags |= IFF_OACTIVE;
	ifp->if_timer = WATCHDOG_TIME;

	IFQ_POLL(&ifp->if_snd, m);
	if (m == NULL) {
		ifp->if_flags &= ~IFF_OACTIVE;
		ifp->if_timer = 0;
		return 0;
	}

	if (m->m_next) {
		er = tkn_transmit_chain(nifp, m);
	} else {
		er = device_send_data(nifp, mtod(m, void *), m->m_len);
	}
	if (er < 0) {
		if (er != E_BUSY) {
			ifp->if_oerrors++;
			ifp->if_flags &= ~IFF_OACTIVE;

			log(tkn_msg_flag,
			    "[TKN %s] transmit error %d\n", __func__, er);
		}
		return 0;
	}

	IFQ_DEQUEUE(&ifp->if_snd, m);
#if NBPFILTER > 0
	if (ifp->if_bpf)
		bpf_mtap(ifp->if_bpf, m);
#endif
	m_freem(m);

	ifp->if_opackets++;
	ifp->if_obytes += er;
	if (m->m_flags & M_MCAST)
		ifp->if_omcasts++;

	return 1;
}

/*
 * ifnet.if_start routine
 */
static void
tkn_start(struct ifnet *ifp)
{
	struct tkn_nif_info *const nifp = ifp->if_softc;
	NetEvent	ne;
	ER		er;

	if ((ifp->if_flags & (IFF_RUNNING|IFF_OACTIVE)) != IFF_RUNNING)
		return;

	ne.len = 0;
	ne.buf = NULL;

	er = tk_snd_mbf(nifp->nif_msg_id, &ne, sizeof(ne), TMO_FEVR);
	if (er < 0) {
		log(tkn_msg_flag,
		    "[TKN %s] send event error %d\n", __func__, er);
	}
}

/*
 * ifnet.if_stop routine
 */
static void
tkn_stop(struct ifnet *ifp, int disable)
{
	(void)disable;
	log(LOG_DEBUG, "ENTER %s disable=%d\n", __func__, disable);

	struct tkn_nif_info *const nifp = ifp->if_softc;
	ER er;

	INT s = splnet();

	// down the MII.
	//mii_down(&xxx);

	er = tkn_device_close(nifp);

	ifp->if_flags &= ~(IFF_RUNNING | IFF_OACTIVE);
	ifp->if_timer = 0;

	splx(s);
}

static void
extbuf_free(struct mbuf *m, caddr_t buf, size_t size, void *arg)
{
	void *const			bufptr = buf;
	struct tkn_extbuf *const	eb     = tkn_extbuf_from_data(bufptr);
	struct tkn_nif_info *const	nifp   = arg;
	(void)size;

	if ( eb->q.next == NULL ) {
		/* Reuse the buffer for receiving a packet. */
		QueInsert(&eb->q, tkn_rxqh(nifp));
		device_attach_rxbufptr(nifp, eb);
	} else {
		/* Discard a buffer. */
		txrxbuf_free(eb);
	}

	if (m) {
		pool_cache_put(mb_cache, m);
	}
}

/*
 * Input routine for the LAN driver.
 */
static void
tkn_nif_input(struct tkn_nif_info *nifp, NetEvent *ne)
{
	struct ifnet		*const	ifp   = &nifp->nif_ifnet;
	void			*const	nebuf = ne->buf;
	int const			nelen = ne->len;
	int s;

	struct tkn_extbuf *const eb = tkn_extbuf_from_data(nebuf);
	if (eb->nifp != nifp) {
		goto err_extbuf_owner;
	}

	ifp->if_ipackets++;
	ifp->if_ibytes += nelen;

	/* Store received data into mbuf. */
	struct mbuf *m;
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == NULL)
		goto err_no_header_memory;
	if ((unsigned)nelen <= MHLEN) {
		/* Return eb if a single mbuf can store received data. */
		memcpy(mtod(m, void *), nebuf, nelen);
		device_attach_rxbufptr(nifp, eb);
	}
	else {
		MEXTADD(m, nebuf, nelen, M_DEVBUF, extbuf_free, nifp);

		QueRemove(&eb->q);
		QueInit(&eb->q);

		if ( device_attach_rxbufptr(nifp, NULL) < 0 ) {
			/*
			 * eb will be reused if a new buffer cannot be
			 * registered.
			 */
			eb->q.next = NULL;
		}
	}
	m->m_len	  = nelen;
	m->m_pkthdr.len	  = nelen;
	m->m_pkthdr.rcvif = ifp;

#if NBPFILTER > 0
	if (ifp->if_bpf)
		bpf_mtap(ifp->if_bpf, m);
#endif

	ifp->if_input(ifp, m);

	return;

err_extbuf_owner:
	log(LOG_ERR, "[TKN]%s: FATAL extbuf owner different\n", __func__);
	return;

err_no_header_memory:
	log(LOG_ERR, "[TKN]%s: no MHDR memory\n", __func__);

	/* Return eb to the LAN driver. */
	device_attach_rxbufptr(nifp, eb);

	s = splvm();
	ifp->if_ierrors++;
	splx(s);

	return;
}

void
tkn_nif_set_mtu(struct tkn_nif_info *nif_infop, int mtu)
{
	nif_infop->nif_ec.ec_if.if_mtu = mtu;

	return;
}

static void
driver_task(INT code, VP arg)
{
	(void)code;
	struct tkn_nif_info *nif_infop = arg;
	ID mbf = nif_infop->nif_msg_id;
	ER er;
	NetEvent ne;
	int s;
	int cont = 0;
	ID oldres;

	oldres = device_enter(nif_infop);

	/*
	 * Wait for event notifications from the LAN driver.
	 */
	while (1) {
		er = tk_rcv_mbf(mbf, &ne, cont ? TMO_POL: TMO_FEVR);
		if (er < 0) {
			if (er == E_DLT) {
				goto out;
			} else if (er != E_TMOUT) {
				goto bad;
			}
			ne.len = 0;

		} else if ((size_t)er != sizeof(NetEvent)) {
			log(tkn_msg_flag,
			    "[TKN]%s: Unknown message size=%u\n", __func__,
			    er);
			continue;
		}

		if (ne.len == 0) {
			s = splnet();
			cont = tkn_nif_output(nif_infop);
			splx(s);

		} else if (ne.len > 0) {
			if (ne.buf == NULL) {
				panic("\n[TKN %s] unexpected packet pointer",
				      __func__);
			}
			s = splnet();
			tkn_nif_input(nif_infop, &ne);
			splx(s);

		} else {
			panic("\n[TKN %s] unexpected packet size %d",
			      __func__, ne.len);
		}
	}

bad:
	log(LOG_ERR, "[TKN] Unexpected driver task exit %d\n", er);

out:
	s = splnet();
	nif_cleanup(nif_infop);
	splx(s);

	tk_exd_tsk();
}
