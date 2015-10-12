/*	$NetBSD: sleepq.c,v 1.1 2008/10/10 13:14:41 pooka Exp $	*/

/*
 * Copyright (c) 2008 Antti Kantee.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/condvar.h>
#include <sys/mutex.h>
#ifndef T2EX
#include <sys/queue.h>
#else
#include <sys/_queue.h>
#endif
#include <sys/sleepq.h>
#include <sys/syncobj.h>

/*
 * Flimsy and minimalistic sleepq implementation.  This is implemented
 * only for the use of callouts in kern_timeout.c.  locking etc is
 * completely incorrect, horrible, etc etc etc.
 */

syncobj_t sleep_syncobj;
static kcondvar_t sq_cv;
static kmutex_t sq_mtx;

#ifdef T2EX
extern kmutex_t spc_lock;
static int initialized_count = 0;
#endif

#ifndef T2EX
void
#else
int
#endif
sleepq_init(sleepq_t *sq)
{
#ifdef T2EX
	int error;
#endif

	TAILQ_INIT(sq);

#ifndef T2EX
	cv_init(&sq_cv, "sleepq"); /* XXX */
	mutex_init(&sq_mtx, MUTEX_DEFAULT, IPL_NONE); /* multi-XXX */
#else
	if ( initialized_count > 0 ) {
		initialized_count++;
		return 0;
	}

	error = cv_init(&sq_cv, "sleepq");
	if ( error != 0 ) {
		return error;
	}
	error = mutex_init(&sq_mtx, MUTEX_DEFAULT, IPL_NONE);
	if ( error != 0 ) {
		cv_destroy(&sq_cv);
		return error;
	}
	initialized_count++;
#endif

#ifdef T2EX
	return 0;
#endif
}

#ifdef T2EX
void
sleepq_finish(sleepq_t *sq)
{
	if ( --initialized_count == 0 ) {
		mutex_destroy(&sq_mtx);
		cv_destroy(&sq_cv);
	}
}
#endif

void
sleepq_enqueue(sleepq_t *sq, wchan_t wchan, const char *wmesg, syncobj_t *sobj)
{
	struct lwp *l = curlwp;

#ifndef T2EX
	if (__predict_false(sobj != &sleep_syncobj || strcmp(wemsg, "callout"))) {
#else
	if (__predict_false(sobj != &sleep_syncobj || (strcmp(wmesg, "callout") != 0 && strcmp(wmesg, "select") != 0 && strcmp(wmesg, "pollsock") != 0))) {
#endif
		panic("sleepq: unsupported enqueue");
	}

	/*
	 * Remove an LWP from a sleep queue if the LWP was deleted while in
	 * the waiting state.
	 */
	if ( l->l_sleepq != NULL && (l->l_stat & LSSLEEP) != 0 ) {
		sleepq_remove(l->l_sleepq, l);
	}

#ifndef T2EX
	l->l_syncobj = sobj;
#endif
	l->l_wchan = wchan;
	l->l_sleepq = sq;
#ifndef T2EX
	l->l_wmesg = wmesg;
	l->l_slptime = 0;
#endif
	l->l_stat = LSSLEEP;
#ifndef T2EX
	l->l_sleeperr = 0;
#endif

	TAILQ_INSERT_TAIL(sq, l, l_sleepchain);
}

int
sleepq_block(int timo, bool hatch)
{
	struct lwp *l = curlwp;
	int error = 0;

	//KASSERT(timo == 0 && !hatch);

	if (timo != 0) {
		callout_schedule(&l->l_timeout_ch, timo);
	}

#ifdef T2EX
	if ( l->l_mutex != NULL ) {
		mutex_exit(l->l_mutex);
	}
#endif

	mutex_enter(&sq_mtx);
	while (l->l_wchan) {
		if ( hatch ) {
			error = cv_timedwait_sig( &sq_cv, &sq_mtx, timo );
		}
		else {
			error = cv_timedwait( &sq_cv, &sq_mtx, timo );
		}

		if (error == EINTR) {
			if (l->l_wchan) {
				TAILQ_REMOVE(l->l_sleepq, l, l_sleepchain);
				l->l_wchan = NULL;
				l->l_sleepq = NULL;
			}
		}
	}
	mutex_exit(&sq_mtx);

#ifdef T2EX
	l->l_mutex = &spc_lock;
#endif

	if (timo != 0) {
		/*
		 * Even if the callout appears to have fired, we need to
		 * stop it in order to synchronise with other CPUs.
		 */
		if (callout_halt(&l->l_timeout_ch, NULL)) {
			error = EWOULDBLOCK;
		}
	}

	return error;
}

#ifdef T2EX
lwp_t *
sleepq_wake(sleepq_t *sq, wchan_t wchan, u_int expected, kmutex_t *mp)
{
	struct lwp *l;
	bool found = false;

	TAILQ_FOREACH(l, sq, l_sleepchain) {
		if (l->l_wchan == wchan) {
			found = true;
			l->l_wchan = NULL;
		}
	}
	if (found)
		cv_broadcast(&sq_cv);

	mutex_spin_exit(mp);
	return NULL;
}
#else
/*
 * sleepq_wake:
 *
 *	Wake zero or more LWPs blocked on a single wait channel.
 */
lwp_t *
sleepq_wake(sleepq_t *sq, wchan_t wchan, u_int expected, kmutex_t *mp)
{
	lwp_t *l, *next;
	int swapin = 0;

	KASSERT(mutex_owned(mp));

	for (l = TAILQ_FIRST(sq); l != NULL; l = next) {
		KASSERT(l->l_sleepq == sq);
		KASSERT(l->l_mutex == mp);
		next = TAILQ_NEXT(l, l_sleepchain);
		if (l->l_wchan != wchan)
			continue;
		swapin |= sleepq_remove(sq, l);
		if (--expected == 0)
			break;
	}

	mutex_spin_exit(mp);

#if 0
	/*
	 * If there are newly awakend threads that need to be swapped in,
	 * then kick the swapper into action.
	 */
	if (swapin)
		uvm_kick_scheduler();
#endif

	return l;
}
#endif

/*
 * XXX: used only by callout, therefore here
 *
 * We don't fudge around with the lwp mutex at all, therefore
 * this is enough.
 */
kmutex_t *
lwp_lock_retry(struct lwp *l, kmutex_t *old)
{

	return old;
}

#ifdef T2EX
/*
 * sleepq_remove:
 *
 *	Remove an LWP from a sleep queue and wake it up.  Return non-zero if
 *	the LWP is swapped out; if so the caller needs to awaken the swapper
 *	to bring the LWP into memory.
 */
int
sleepq_remove(sleepq_t *sq, lwp_t *l)
{

	KASSERT(lwp_locked(l, NULL));

	TAILQ_REMOVE(sq, l, l_sleepchain);

#ifndef T2EX
	l->l_syncobj = NULL;
#endif
	l->l_wchan = NULL;
	l->l_sleepq = NULL;
#ifndef T2EX
	l->l_wmesg = NULL;
	l->l_slptime = 0;
#endif
	l->l_stat &= ~LSSLEEP;

	cv_broadcast(&sq_cv);

	return 0;
}

/*
 * sleepq_unsleep:
 *
 *	Remove an LWP from its sleep queue and set it runnable again.
 *	sleepq_unsleep() is called with the LWP's mutex held, and will
 *	always release it.
 */
u_int
sleepq_unsleep(lwp_t *l, bool cleanup)
{
	sleepq_t *sq = l->l_sleepq;
	kmutex_t *mp = l->l_mutex;
	int swapin;

#ifndef T2EX
	KASSERT(lwp_locked(l, mp));
#endif
	KASSERT(l->l_wchan != NULL);

	swapin = sleepq_remove(sq, l);

	if (cleanup) {
		mutex_spin_exit(mp);
	}

	return swapin;
}

#ifndef T2EX
u_int
lwp_unsleep(lwp_t *l, bool cleanup)
{

	KASSERT(mutex_owned(l->l_mutex));

	return (*l->l_syncobj->sobj_unsleep)(l, cleanup);
}
#endif

/*
 * sleepq_timeout:
 *
 *	Entered via the callout(9) subsystem to time out an LWP that is on a
 *	sleep queue.
 */
void
sleepq_timeout(void *arg)
{
	lwp_t *l = arg;

	/*
	 * Lock the LWP.  Assuming it's still on the sleep queue, its
	 * current mutex will also be the sleep queue mutex.
	 */
	lwp_lock(l);

	if (l->l_wchan == NULL) {
		/* Somebody beat us to it. */
		lwp_unlock(l);
		return;
	}

	//lwp_unsleep(l, true);
	sleepq_unsleep(l, true);
}

/* General purpose sleep table, used by ltsleep() and condition variables. */
sleeptab_t	sleeptab;

/*
 * sleeptab_init:
 *
 *	Initialize a sleep table.
 */
#ifndef T2EX
void
#else
int
#endif
sleeptab_init(sleeptab_t *st)
{
	sleepq_t *sq;
	int i;
#ifdef T2EX
	int error;
#endif

	for (i = 0; i < SLEEPTAB_HASH_SIZE; i++) {
		sq = &st->st_queues[i].st_queue;
#ifndef T2EX
		mutex_init(&st->st_queues[i].st_mutex, MUTEX_DEFAULT,
		    IPL_SCHED);
		sleepq_init(sq);
#else
		error = mutex_init(&st->st_queues[i].st_mutex, MUTEX_DEFAULT,
		    IPL_SCHED);
		if ( error != 0 ) {
			goto err_ret1_1;
		}
		error = sleepq_init(sq);
		if ( error != 0 ) {
			goto err_ret1_2;
		}
#endif
	}

#ifdef T2EX
	return 0;

err_ret1_2:
	for(int j = 0; j < i; j++) {
		mutex_destroy(&st->st_queues[j].st_mutex);
		sleepq_finish(&st->st_queues[i].st_queue);
	}
	goto err_ret0;
err_ret1_1:
	for(int j = 0; j < i; j++) {
		mutex_destroy(&st->st_queues[j].st_mutex);
		if ( j != i-1 ) {
			sleepq_finish(&st->st_queues[i].st_queue);
		}
	}
err_ret0:
	return error;
#endif
}

#ifdef T2EX
int
sleeptab_finish(sleeptab_t *st)
{
	sleepq_t *sq;
	int i;

	for (i = 0; i < SLEEPTAB_HASH_SIZE; i++) {
		sq = &st->st_queues[i].st_queue;

		sleepq_finish(sq);
		mutex_destroy(&st->st_queues[i].st_mutex);
	}

	return 0;
}
#endif

/*
 * sleepq_abort:
 *
 *	After a panic or during autoconfiguration, lower the interrupt
 *	priority level to give pending interrupts a chance to run, and
 *	then return.  Called if sleepq_dontsleep() returns non-zero, and
 *	always returns zero.
 */
int
sleepq_abort(kmutex_t *mtx, int unlock)
{
#ifndef T2EX
	extern int safepri;
	int s;

	s = splhigh();
	splx(safepri);
	splx(s);
#endif
	if (mtx != NULL && unlock != 0)
		mutex_exit(mtx);

	return 0;
}
#endif /* T2EX */
