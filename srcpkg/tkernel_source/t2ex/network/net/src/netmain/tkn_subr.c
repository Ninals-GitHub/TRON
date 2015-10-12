/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2014/07/14.
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
 *	@(#)tkn_subr.c
 *
 */

/* BSD general subroutines */

#include <tk/tkernel.h>

#include <sys/param.h>
#ifndef T2EX
#include <sys/ucred.h>
#endif
#include <sys/systm.h>
#include <sys/sysctl.h>
#include <sys/malloc.h>
#include <sys/sleepq.h>
#include <sys/filedesc.h>
#include <sys/syscallargs.h>

#include <net/if.h>
#include <net/route.h>
#include <netmain/if_tkn.h>
#include "tkn.h"

#include <ifaddrs.h>

lwp_t lwp0;
proc_t proc;
filedesc_t* filedesc0;

static lwp_t *tskid_lwp_table;
static int tskid_lwp_table_maxid;

kmutex_t spc_lock;

EXPORT ER tkn_lwp_init(void)
{
	int i;
	int error;
	ER ercd;

	error = mutex_init(&spc_lock, 0, IPL_NONE);
	if ( error != 0 ) {
		ercd = ERRNOtoER(error);
		goto err_ret0;
	}

	if (tk_get_cfn("TMaxTskId", &tskid_lwp_table_maxid, 1) < 1) {
		ercd = E_SYS;
		goto err_ret1;
	}

	tskid_lwp_table = (lwp_t *)malloc(sizeof(lwp_t)*tskid_lwp_table_maxid, M_KMEM, M_NOWAIT | M_ZERO);
	if (tskid_lwp_table == NULL) {
		ercd = E_NOMEM;
		goto err_ret1;
	}

	for(i = 0; i < tskid_lwp_table_maxid; i++) {
		tskid_lwp_table[i].l_proc = &proc;
		tskid_lwp_table[i].l_mutex = &spc_lock;

		callout_init(&tskid_lwp_table[i].l_timeout_ch, 0);
		callout_setfunc(&tskid_lwp_table[i].l_timeout_ch, sleepq_timeout, &tskid_lwp_table[i]);
	}

	return E_OK;

err_ret1:
	mutex_destroy(&spc_lock);
err_ret0:
	return ercd;
}

EXPORT ER tkn_lwp_setfd(void)
{
	int i;

	for(i = 0; i < tskid_lwp_table_maxid; i++) {
		tskid_lwp_table[i].l_fd = filedesc0;
	}

	proc.p_fd = filedesc0;

	return E_OK;
}

EXPORT ER tkn_lwp_finish(void)
{
	int i;

	for(i = 0; i < tskid_lwp_table_maxid; i++) {
		callout_destroy(&tskid_lwp_table[i].l_timeout_ch);
	}

	free(tskid_lwp_table, M_KMEM);

	mutex_destroy(&spc_lock);

	return E_OK;
}

static lwp_t*
get_lwp(ID tskid)
{
	return &tskid_lwp_table[tskid - 1];
}

lwp_t*
tkn_curlwp(void)
{
	ID tskid = tk_get_tid();

	lwp_t* re = get_lwp(tskid);

	return re;
}

/*
 * Move data described by a struct uio.
 *   The uiomove function copies up to n bytes between the kernel-space
 *   address pointed to by buf and the addresses described by uio, which may
 *   be in user-space or kernel-space.
 */
int
uiomove(void *buf, size_t n, struct uio *uio)
{
#ifndef T2EX
	struct vmspace *vm = uio->uio_vmspace;
#endif
	struct iovec *iov;
	size_t cnt;
	int error = 0;
	char *cp = buf;

	ASSERT_SLEEPABLE();

#ifdef DIAGNOSTIC
	if (uio->uio_rw != UIO_READ && uio->uio_rw != UIO_WRITE)
		panic("uiomove: mode");
#endif
	while (n > 0 && uio->uio_resid) {
		iov = uio->uio_iov;
		cnt = iov->iov_len;
		if (cnt == 0) {
			KASSERT(uio->uio_iovcnt > 0);
			uio->uio_iov++;
			uio->uio_iovcnt--;
			continue;
		}
		if (cnt > n)
			cnt = n;
#ifndef T2EX
		if (!VMSPACE_IS_KERNEL_P(vm)) {
			if (curcpu()->ci_schedstate.spc_flags &
			    SPCF_SHOULDYIELD)
				preempt();
		}

		if (uio->uio_rw == UIO_READ) {
			error = copyout_vmspace(vm, cp, iov->iov_base,
			    cnt);
		} else {
			error = copyin_vmspace(vm, iov->iov_base, cp,
			    cnt);
		}
#else
		if (uio->uio_rw == UIO_READ) {
			error = copyout(cp, iov->iov_base, cnt);
		} else {
			error = copyin(iov->iov_base, cp, cnt);
		}
#endif
		if (error) {
			break;
		}
		iov->iov_base = (char *)iov->iov_base + cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		uio->uio_offset += cnt;
		cp += cnt;
		KDASSERT(cnt <= n);
		n -= cnt;
	}

	return (error);
}

int
sys_select_us(struct lwp *l, const struct sys_select_us_args *uap, register_t *retval)
{
	struct timeval tv;
	struct sys_select_args a = {
		.a = {
			.nfds = SCARG(uap, nfds),
			.readfds = SCARG(uap, readfds),
			.writefds = SCARG(uap, writefds),
			.exceptfds = SCARG(uap, exceptfds),
		}
	};

	if (SCARG(uap, tmout_u) == TMO_FEVR) {
		a.a.tv = NULL;
	}
	else if (SCARG(uap, tmout_u) >= 0) {
		tv.tv_sec = SCARG(uap, tmout_u) / 1000000;
		tv.tv_usec = SCARG(uap, tmout_u) % 1000000;
		a.a.tv = &tv;
	}
	else {
		return EINVAL;
	}

	return sys_select(l, &a, retval);
}

int
sys_select_ms(struct lwp *l, const struct sys_select_ms_args *uap, register_t *retval)
{
	struct timeval tv;
	struct sys_select_args a = {
		.a = {
			.nfds = SCARG(uap, nfds),
			.readfds = SCARG(uap, readfds),
			.writefds = SCARG(uap, writefds),
			.exceptfds = SCARG(uap, exceptfds),
		}
	};

	if (SCARG(uap, tmout) == TMO_FEVR) {
		a.a.tv = NULL;
	}
	else if (SCARG(uap, tmout) >= 0) {
		tv.tv_sec = SCARG(uap, tmout) / 1000;
		tv.tv_usec = (SCARG(uap, tmout) % 1000) * 1000;
		a.a.tv = &tv;
	}
	else {
		return EINVAL;
	}

	return sys_select(l, &a, retval);
}

static char sys_hostname[MAXHOSTNAMELEN];

int
sys_gethostname(struct lwp *l,  const struct sys_gethostname_args *uap, register_t *retval)
{
	ER ercd;

	(void)l;

	ercd = ChkSpaceRW(SCARG(uap, name), SCARG(uap, len));
	if ( ercd < E_OK ) {
		return EFAULT;
	}

	strlcpy(SCARG(uap, name), sys_hostname, SCARG(uap, len));

	*retval = 0;
	return 0;
}

int
sys_sethostname(struct lwp *l,  const struct sys_sethostname_args *uap, register_t *retval)
{
	ER ercd;

	(void)l;

	ercd = ChkSpaceR(SCARG(uap, name), SCARG(uap, len));
	if ( ercd < E_OK ) {
		return EFAULT;
	}

	if ( SCARG(uap, len) > MAXHOSTNAMELEN ) {
		*retval = -1;
		return EINVAL;
	}

	strlcpy(sys_hostname, SCARG(uap, name), MAXHOSTNAMELEN);

	*retval = 0;
	return 0;
}

IMPORT int sockatmark(int s);
int
sys_sockatmark(struct lwp *l, const struct sys_sockatmark_args *uap, register_t *retval)
{
	int re;

	(void)l;

	re = sockatmark(SCARG(uap, sd));
	*retval = re;
	return 0;
}



/*
 * This function is called by so_break().
 *
 * The following pre-condition is always satisfied.
 *    tskid != TSK_ALL
 */
static int do_sys_break_task(ID tskid)
{
	int cnt = 0;

	if( get_lwp(tskid)->is_waiting != 0 ) {
		tk_dis_wai(tskid, TTW_FLG | TTW_CAL | TTW_RDV );
		cnt = 1;
	}

	return cnt;
}

int
sys_break(struct lwp *l, const struct sys_break_args *uap, register_t *retval)
{
	/* {
		syscallarg(ID)		tskid;
	 } */
	int tskid = SCARG(uap, tskid);
	int cnt = 0;
	int err = 0;
	int i;
	T_RTSK rtsk;

	(void)l;

	tk_dis_dsp();

	if(tskid == TSK_ALL) {
		for(i=1; i<=tskid_lwp_table_maxid; i++) {
			cnt += do_sys_break_task(tskid);
		}
	}
	else {
		err = tk_ref_tsk(tskid, &rtsk);
		if ( err == E_OK ) {
			cnt = do_sys_break_task(tskid);
		}
	}

	tk_ena_dsp();

	*retval = cnt;
	return err;
}

int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
	if (tz != NULL)
		goto err_param;

	microtime(tv);

	return 0;

err_param:
	return EINVAL;
}

static struct tkn_nif_info tkn_dev_info[TKN_NIFDEV_MAX];

int
sys_ifattach(struct lwp *l, const struct sys_ifattach_args *uap, register_t *retval)
{
	const char* devname = SCARG(uap, devnm);
	int dev_num;
	int error = 0;
	int ret;
	T_RDEV rdev;
	struct tkn_nif_info *nif;

	(void)l;

	ret = ChkSpaceBstrR(devname, 0);
	if ( ret < 0 ) {
		return EFAULT;
	}

	for (nif = tkn_nif_mng.nifm_list; nif; nif = nif->nif_next) {
		if (strncmp((char*)nif->nif_dev, devname, TKN_NIFNAMESIZE) == 0) {
			break;
		}
	}
	if( nif != 0 ) {
		/* Already registered. */
		error = EBUSY;
		goto error;
	}

	ret = tk_ref_dev((UB*)devname, &rdev);
	if (ret < 0) {
		error = EINVAL;
		goto error;
	}

	dev_num  = tkn_nif_mng.nifm_nifmax;
	strncpy((char*)tkn_dev_info[dev_num].nif_dev, devname, TKN_NIFNAMESIZE);

	ret = tkn_nif_attach(&tkn_dev_info[dev_num]);
	if (ret != 0) {
		error = EINVAL;
	}

error:
	*retval = 0;

	return error;
}

int
sys_ifdetach(struct lwp *l, const struct sys_ifdetach_args *uap, register_t *retval)
{
	const char* devname = SCARG(uap, devnm);
	int error = 0;
	int ret;
	T_RDEV rdev;
	struct tkn_nif_info *nif;

	(void)l;

	ret = ChkSpaceBstrR(devname, 0);
	if ( ret < 0 ) {
		return EFAULT;
	}

	ret = tk_ref_dev((UB*)devname, &rdev);
	if (ret < 0) {
		error = EINVAL;
		goto error;
	}

	for (nif = tkn_nif_mng.nifm_list; nif != NULL; nif = nif->nif_next) {
		if (strncmp((char*)nif->nif_dev, devname, TKN_NIFNAMESIZE) == 0) {
			break;
		}
	}
	if( nif == 0 ) {
		/* Not registered. */
		error = ENOENT;
		goto error;
	}

	ret = tkn_nif_detach(nif);
	if (ret != 0) {
		error = EINVAL;
	}

error:
	*retval = 0;

	return error;
}

int
sys_rtlist(struct lwp *l, const struct sys_rtlist_args *uap, register_t *retval)
{
	int error;
	size_t len;

	(void)l;

	if ( SCARG(uap, buf) != NULL ) {
		error = ChkSpaceRW(SCARG(uap, buf), SCARG(uap, bufsz));
		if ( error < E_OK ) {
			return EFAULT;
		}
	}

	len = SCARG(uap, bufsz);
	error = sysctl_rtable(SCARG(uap,af), SCARG(uap,cmd), SCARG(uap,flags),
			      SCARG(uap,buf), &len );

	if (error == 0) {
		*retval = len;
	}

	return error;
}

int
sys_getifaddrs(struct lwp *l, const struct sys_getifaddrs_args *uap, register_t *retval)
{
	int error;
	size_t asize;

	(void)l;

	if ( SCARG(uap, buf) != NULL ) {
		error = ChkSpaceRW(SCARG(uap, buf), SCARG(uap, bufsz));
		if ( error < E_OK ) {
			return EFAULT;
		}
	}

	error = getifaddrs(SCARG(uap,ifap), SCARG(uap,buf), SCARG(uap,bufsz),
			   &asize);

	if (error == 0) {
		*retval = asize;
	}

	return error;
}

int
sys_ifindextoname(struct lwp *l, const struct sys_ifindextoname_args *uap, register_t *retval)
{
	int error;

	(void)l;

	error = ChkSpaceBstrRW(SCARG(uap, ifname), 0);
	if ( error < 0 ) {
		return EFAULT;
	}

	error = if_indextoname(SCARG(uap,ifindex), SCARG(uap,ifname));
	if (error == 0) {
		*retval = 0;
	}

	return error;
}

int
sys_ifnametoindex(struct lwp *l, const struct sys_ifnametoindex_args *uap, register_t *retval)
{
	int error;
	unsigned int ifindex;

	(void)l;

	error = ChkSpaceBstrR(SCARG(uap, ifname), 0);
	if ( error < 0 ) {
		return EFAULT;
	}

	error = if_nametoindex(SCARG(uap,ifname), &ifindex);
	if (error == 0) {
		*retval = ifindex;
	}

	return error;
}

int sys_bpfopen(struct lwp *l, const struct sys_bpfopen_args *uap, register_t *retval)
{
	int error;
	struct file* fp;
	int indx;

	error = ChkSpaceBstrR(SCARG(uap, path), 0);
	if ( error < E_OK ) {
		return EFAULT;
	}

	if ((error = fd_allocfile(&fp, &indx)) != 0)
		return (error);

	if ((error = tkn_bpf_open(SCARG(uap, path), SCARG(uap, oflag), fp)) != 0) {
		fd_abort(l->l_proc, fp, indx);
		if ((error == EDUPFD || error == EMOVEFD) &&
		    l->l_dupfd >= 0 &&			/* XXX from fdopen */
		    (error =
			fd_dupopen(l->l_dupfd, &indx, SCARG(uap, oflag), error)) == 0) {
			*retval = indx;
			return (0);
		}
		if (error == ERESTART)
			error = EINTR;
		return (error);
	}

	return ENOTSUP;
}

int sys_tunopen(struct lwp *l, const struct sys_tunopen_args *uap, register_t *retval)
{
	int error;
	struct file* fp;
	int indx;

	error = ChkSpaceBstrR(SCARG(uap, path), 0);
	if ( error < E_OK ) {
		return EFAULT;
	}

	if ((error = fd_allocfile(&fp, &indx)) != 0)
		return (error);

	if ((error = tkn_tun_open(SCARG(uap, path), SCARG(uap, oflag), fp)) != 0) {
		fd_abort(l->l_proc, fp, indx);
		if ((error == EDUPFD || error == EMOVEFD) &&
		    l->l_dupfd >= 0 &&			/* XXX from fdopen */
		    (error =
			fd_dupopen(l->l_dupfd, &indx, SCARG(uap, oflag), error)) == 0) {
			*retval = indx;
			return (0);
		}
		if (error == ERESTART)
			error = EINTR;
		return (error);
	}

	return ENOTSUP;
}
