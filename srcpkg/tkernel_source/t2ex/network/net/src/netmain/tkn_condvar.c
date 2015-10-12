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
 *	@(#)tkn_condvar.c
 *
 */

#include <tk/tkernel.h>
#include "rominfo.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/condvar.h>
#include <sys/_queue.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/malloc.h>

#include <sys/lwp.h>

#include <tk/util.h>
#include "tkn.h"

IMPORT ER lock_mutex(__volatile kmutex_t* mtx, TMO tmo);
IMPORT ER unlock_mutex(__volatile kmutex_t* mtx);
IMPORT INT mtx_oldspl;
IMPORT INT mtx_count;


#define SIGNAL_SINGLE		(1)
#define SIGNAL_BROADCAST	(2)

#define DEFAULT_MAX_COUNT ((UINT)128)
#define INDEX_OFFSET 5

#define INDEX(x)	(((x) >> INDEX_OFFSET) - 1)
#define NO(x)		((x) & 0x1fU)
#define ID(x, y)	((((x) + 1) << INDEX_OFFSET) | (y))

typedef struct {
	ID flgid;
	UW used_bitmap;
} CondvarFlag;

LOCAL UINT max_condvar_count;
LOCAL UINT flag_count;
LOCAL CondvarFlag* condvar_flag;

EXPORT ER tkn_condvar_init(void)
{
	LockTKN();

	/*
	 * Calculation of the number of condition variables
	 *  - tkn_initialize() needs 37 condition variables.
	 *  - a socket needs 4 condition variables and the maximum number of
	 *    sockets is (maxfiles - NDFDFILE).
	 */
	max_condvar_count = 37 + (maxfiles - NDFDFILE) * 4;
	flag_count = (max_condvar_count + 31) / 32;

	condvar_flag = malloc(sizeof(CondvarFlag)*flag_count, M_KMEM, M_NOWAIT | M_ZERO);

	UnlockTKN();

	return (condvar_flag == NULL) ? E_NOMEM : E_OK;
}

ER tkn_condvar_finish(void)
{
	int index;
	ER ercd  = E_OK;

	LockTKN();

	for(index = 0; index < flag_count; index++) {
		if ( condvar_flag[index].used_bitmap != 0 ) {
			ercd = tk_del_flg(condvar_flag[index].flgid);
			if ( ercd < E_OK ) {
				break;
			}
		}
	}

	if ( ercd == E_OK ) {
		free(condvar_flag, M_KMEM);
	}

	UnlockTKN();

	return ercd;
}

LOCAL int init_eventflag( kcondvar_t* cv )
{
	int index;
	int no;

	for(index = 0; index < flag_count; index++) {
		if ( condvar_flag[index].used_bitmap != 0xffffffffU ) {
			break;
		}
	}

	if ( index >= flag_count ) {
		return ENOMEM;
	}

	if ( condvar_flag[index].used_bitmap == 0 ) {
		T_CFLG	cflg;
		ER ercd;

		SetOBJNAME(cflg.exinf, "Ncv");
		cflg.flgatr  = TA_TPRI | TA_WMUL;
		ercd = tk_cre_flg(&cflg);
		if ( ercd < E_OK ) {
			return ENOMEM;
		}
		condvar_flag[index].flgid = ercd;
	}

	for(no = 0; no < sizeof(UW)*8; no++) {
		if ( (condvar_flag[index].used_bitmap & (1U << no)) == 0 ) {
			break;
		}
	}

	if ( no == sizeof(UW)*8 ) {
		return ENOENT;
	}

	condvar_flag[index].used_bitmap |= 1U << no;

	cv->flgid = ID(index, no);
	cv->signal = 0;
	cv->waitcnt = 0;

	return 0;
}

LOCAL void release_eventflag(__volatile kcondvar_t* cv)
{
	int index = INDEX(cv->flgid);
	int no = NO(cv->flgid);
	int flgid = condvar_flag[index].flgid;
	int ptn = 0x1U << no;

	cv->signal = SIGNAL_BROADCAST;
	tk_set_flg(flgid, ptn);

	condvar_flag[index].used_bitmap &= ~(1U << no);

	if ( condvar_flag[index].used_bitmap == 0 ) {
		ER ercd = tk_del_flg(flgid);
		if ( ercd < E_OK ) {
			panic("release_eventflag: %d\n", MERCD(ercd));
		}
	}

	cv->flgid = 0;
}

/* ------------------------------------------------------------------------ */

int tkn_cv_init(kcondvar_t *cv, const char *msg)
{
	int error;
	(void)msg;

	LockTKN();

#ifdef DEBUG
	unsigned long old_id = cv->flgid;
#endif
	error = init_eventflag(cv);
#ifdef DEBUG
	printf("condvar init %d(at %p) by %d, old id = %lu\n", cv->flgid, cv, tk_get_tid(), old_id);
#endif

	UnlockTKN();

	return error;
}

void tkn_cv_destroy(kcondvar_t *cv)
{
	LockTKN();

	if ( cv->flgid != 0 ) {
#ifdef DEBUG
		printf("condvar destroy %d(at %p) by %d\n", cv->flgid, cv, tk_get_tid());
#endif
		release_eventflag(cv);
	}

	UnlockTKN();
}

LOCAL int tkn_docvwait(kcondvar_t *cv, kmutex_t *mtx, int ticks, bool catch)
{
#ifdef DIAGNOSTIC
	extern int hz;
#endif

	UINT flgptn = 0;
	int waiting = 1;
	ER ercd;
	int error = 0;
	SYSTIM t1, t2;
	lwp_t *lwp = NULL;
	int index;
	int no;
	ID flgid;
	UINT ptn;
	int oldspl, s;

	LockTKN();

	if ( cv->flgid == 0 ) {
		panic( "docvwait: not initialized.\n");
	}

	if (ticks == 0) {
		ticks = TMO_FEVR;
	} else {
		KASSERT(hz == 100);
		tk_get_otm( &t1 );
	}

	/* Allow so_break() to release a waiting state. */
	if ( catch != 0 ) {
		lwp = curlwp;
		lwp->is_waiting = 1;
	}

	index = INDEX(cv->flgid);
	no = NO(cv->flgid);
	flgid = condvar_flag[index].flgid;
	ptn = 0x1U << no;
	cv->waitcnt++;

	/* Clear flags in the case that flags are set before waiting */
	tk_clr_flg(flgid, ~ptn);

	while(waiting) {
		ercd = unlock_mutex(mtx);

		if ( mtx->type == MUTEX_SPIN ) {
			if ( --mtx_count == 0 ) {
				s = mtx_oldspl;
				mtx_oldspl = 0;
			}
		}
		UnlockTKN();

		oldspl = tkn_spl_unlock(IPL_NONE);

		ercd = tk_wai_flg(flgid, ptn, TWF_ANDW | TWF_BITCLR, &flgptn, ticks);

		s = tkn_spl_lock(oldspl);

		LockTKN();
		if ( mtx->type == MUTEX_SPIN ) {
			if ( mtx_count++ == 0 ) {
				mtx_oldspl = s;
			}
		}

		lock_mutex(mtx, TMO_FEVR);

		if ( cv->signal == SIGNAL_SINGLE ) {
			waiting = 0;
			cv->waitcnt--;
			cv->signal = 0;
		} else if ( cv->signal == SIGNAL_BROADCAST ) {
			waiting = 0;
			cv->waitcnt--;
			if ( cv->waitcnt == 0 ) {		/* No task is waiting. */
				cv->signal = 0;
			} else {
				tk_set_flg(flgid, ptn);
			}
		} else if ( ercd == E_TMOUT ) {			/* Timeout */
			error = EWOULDBLOCK;
			waiting = 0;
			cv->waitcnt--;
		} else if ( catch != 0 && ercd == E_DISWAI ) {	/* so_break() */
			error = EINTR;
			waiting = 0;
			cv->waitcnt--;
		} else if ( ticks != TMO_FEVR ) {		/* Calcurate time remaining */
			tk_get_otm( &t2 );
			ticks -= (int)(t2.lo - t1.lo);
			t1 = t2;
		}
	}

	if( catch != 0 ) {
		lwp->is_waiting = 0;
	}

	UnlockTKN();

	return error;
}

void tkn_cv_wait(kcondvar_t *cv, kmutex_t *mtx)
{
	tkn_docvwait(cv, mtx, 0, false);
}

int tkn_cv_wait_sig(kcondvar_t *cv, kmutex_t *mtx)
{
	return tkn_docvwait(cv, mtx, 0, true);
}

int tkn_cv_timedwait(kcondvar_t *cv, kmutex_t *mtx, int ticks)
{
	return tkn_docvwait(cv, mtx, ticks, false);
}

int tkn_cv_timedwait_sig(kcondvar_t *cv, kmutex_t *mtx, int ticks)
{
	return tkn_docvwait(cv, mtx, ticks, true);
}

LOCAL void tkn_docv_signal(kcondvar_t *cv, int signal)
{
	UINT index;
	UINT no;
	ID flgid;
	UINT ptn;

	LockTKN();

	if ( cv->flgid == 0 ) panic("docv_signal: not initialized.\n");

	index = INDEX(cv->flgid);
	no = NO(cv->flgid);
	flgid = condvar_flag[index].flgid;
	ptn = 0x1U << no;

	cv->signal = signal;
	tk_set_flg(flgid, ptn);

	UnlockTKN();
}

void tkn_cv_signal(kcondvar_t *cv)
{
	tkn_docv_signal(cv, SIGNAL_SINGLE);
}

void tkn_cv_broadcast(kcondvar_t *cv)
{
	tkn_docv_signal(cv, SIGNAL_BROADCAST);
}

bool tkn_cv_has_waiters(kcondvar_t *cv)
{
	bool has_waiters;

	LockTKN();

	if ( cv->flgid == 0 ) panic("cv_has_waiters: not initialized.\n");

	has_waiters = (cv->waitcnt != 0) ? true : false;

	UnlockTKN();

	return has_waiters;
}
