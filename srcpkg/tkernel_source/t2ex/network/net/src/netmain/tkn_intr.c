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
 *	@(#)tkn_intr.c
 *
 */

#include <tk/tkernel.h>
#include "netmain/tkn_spl.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/syslog.h>
#include <sys/tkn_intr.h>
#include <sys/intr.h>
#include <sys/lwp.h>
#include <sys/malloc.h>

#include "tkn.h"
#include "netmain/tkn_taskutil.h"


struct tkn_intrhand {
	ID task;
	ID sem;
};

struct tkn_spl_trampo {
	struct tkn_intrhand ih;
	int level;
	int mpsafe;
	void (*hand)(void*);
	void* arg;
};

static void spl_trampo(INT arg, VP vp)
{
	ER er = 0;
	lwp_t *lwp = curlwp;
	struct tkn_spl_trampo *trampo = vp;
	if (trampo == NULL)
		goto err_bad_arg;

	ID sem = trampo->ih.sem;
	void (*hand)(void*) = trampo->hand;
	void* p_arg = trampo->arg;
	int level = trampo->level;
	int mpsafe = trampo->mpsafe;
	int s;

	for (;;) {
		/*
		 * Wait for a software interrupt notification.
		 */
		er = tk_wai_sem(sem, 1, TMO_FEVR);
		if (er < 0) {
			if (er == E_NOEXS || er == E_DLT)
				goto err_exit;
			goto err_sem;
		}

		s = tkn_spl_lock(level);
		if ( mpsafe ) {
			KERNEL_LOCK(1, lwp);
			hand(p_arg);
			KERNEL_UNLOCK_ONE(lwp);
		} else {
			hand(p_arg);
		}
		tkn_spl_unlock(s);
	}
	/* Never reach here. */
	return;

err_bad_arg:
	/* Invalid arguments. */
	log(LOG_ERR, "%s:tid=%d bad args", __func__, tk_get_tid());
	tk_exd_tsk();
	return;
err_sem:
	log(LOG_ERR, "%s: unexpected error %d(%d, %d)", __func__, er, MERCD(er), SERCD(er));
err_exit:
	/* Request for terminating the task by deleting the semaphore. */
	free(trampo, M_SOFTINTR);
	tk_exd_tsk();
}


/*
 * This function can be called from the context in which interrupts are disabled.
 */
int
tkn_spl_submit_intr_di(int id)
{
	return tk_sig_sem(id, 1);
}

int
tkn_spl_submit_intr(int id)
{
	ER er;

	er = tkn_spl_submit_intr_di(id);
	if (er < 0)
		goto err_signal;
	return 0;

err_signal:
	log(LOG_ERR, "%s: unexpected error %d(%d, %d) ID=%d\n", __func__, er, MERCD(er), SERCD(er), id);
	return er;
}

/*
 * Create a software interrupt handler task.
 *
 *   hand:  task start address
 *   level: interrupt level (a higher level value means a higher priority.)
 *
 * This returns a non-zero value which represents a semaphore ID, or a negative
 * value which represents an error. This semaphore tirggers a software interrupt
 * by calling tk_sig_sem() with the returned semaphore ID.
 *
 * Deleting the semaphore terminates the handler task.
 */
int
tkn_spl_make_handler(void (*hand)(), int flags, void* arg)
{
	int level = flags & SOFTINT_LVLMASK;
	int mpsafe = flags & SOFTINT_MPSAFE;
	int ipl_level;

	switch(level) {
	case SOFTINT_BIO:
		ipl_level = IPL_BIO;
		break;
	case SOFTINT_CLOCK:
		ipl_level = IPL_CLOCK;
		break;
	case SOFTINT_SERIAL:
		ipl_level = IPL_SERIAL;
		break;
	case SOFTINT_NET:
		ipl_level = IPL_NET;
		break;
	default:
		ipl_level = level;
	}

	/*
	 * Allocate memory to store context information.
	 */
	struct tkn_spl_trampo *trampo;
	trampo = malloc(sizeof(struct tkn_spl_trampo), M_SOFTINTR, M_NOWAIT | M_ZERO);
	if (trampo == NULL)
		goto err_trampo;

	struct tkn_intrhand ih;

	ih.sem = tkn_cre_sem("Nsih");
	if (ih.sem < 0)
		goto err_sem;

	/*
	 * Create a task
	 */
	PRI prio;
	trampo->level = ipl_level;
	trampo->mpsafe = mpsafe;
	trampo->hand  = hand;
	trampo->arg   = arg;
	prio = tkn_spl_priority(ipl_level);
	ih.task = tkn_cre_tsk(spl_trampo, prio, TKN_SPLTRAMPO_STKSZ, trampo);
	if (ih.task < 0)
		goto err_task;
	trampo->ih = ih;

	/*
	 * Start the task
	 */
	ER er;
	er = tk_sta_tsk(ih.task, 0);
	if (er < 0)
		goto err_task_start;

	return ih.sem;

err_task_start:
	tk_del_tsk(ih.task);
err_task:
	tk_del_sem(ih.sem);
err_sem:
	free(trampo, M_RTABLE);
err_trampo:
	log(LOG_ERR, "%s: unexpected error\n", __func__);
	return -1;
}

int tkn_spl_delete_handler(int id)
{
	return tk_del_sem(id);
}

void* softint_establish(unsigned int flags, void (*func)(void*), void* arg)
{
	int ret = tkn_spl_make_handler(func, flags, arg);

	return (ret < 0 ? NULL : (void*)ret);
}

void softint_disestablish(void* handle)
{
	tk_del_sem((ID)handle);
}

void softint_schedule(void* handle)
{
	tkn_spl_submit_intr((ID)handle);
}
