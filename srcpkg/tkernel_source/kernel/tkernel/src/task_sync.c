/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by T-Engine Forum at 2012/10/24.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	task_sync.c (T-Kernel/OS)
 *	Task with Synchronize Function
 */

#include <limits.h>
#include <tk/kernel.h>
#include <tk/task.h>
#include "wait.h"
#include "check.h"

/*
 * Suspend task
 */
SYSCALL ER _tk_sus_tsk( ID tskid )
{
	TCB	*tcb;
	TSTAT	state;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	state = (TSTAT)tcb->state;
	if ( !task_alive(state) ) {
		ercd = ( state == TS_NONEXIST )? E_NOEXS: E_OBJ;
		goto error_exit;
	}
	if ( tcb == ctxtsk && dispatch_disabled >= DDS_DISABLE ) {
		ercd = E_CTX;
		goto error_exit;
	}
	if ( tcb->suscnt == INT_MAX ) {
		ercd = E_QOVR;
		goto error_exit;
	}

	/* Update suspend request count */
	++tcb->suscnt;

	/* Move to forced wait state */
	if ( state == TS_READY ) {
		make_non_ready(tcb);
		tcb->state = TS_SUSPEND;

	} else if ( state == TS_WAIT ) {
		tcb->state = TS_WAITSUS;
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Resume task
 */
SYSCALL ER _tk_rsm_tsk( ID tskid )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	switch ( tcb->state ) {
	  case TS_NONEXIST:
		ercd = E_NOEXS;
		break;

	  case TS_DORMANT:
	  case TS_READY:
	  case TS_WAIT:
		ercd = E_OBJ;
		break;

	  case TS_SUSPEND:
		if ( --tcb->suscnt == 0 ) {
			make_ready(tcb);
		}
		break;
	  case TS_WAITSUS:
		if ( --tcb->suscnt == 0 ) {
			tcb->state = TS_WAIT;
		}
		break;

	  default:
		ercd = E_SYS;
		break;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Force resume task
 */
SYSCALL ER _tk_frsm_tsk( ID tskid )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	switch ( tcb->state ) {
	  case TS_NONEXIST:
		ercd = E_NOEXS;
		break;

	  case TS_DORMANT:
	  case TS_READY:
	  case TS_WAIT:
		ercd = E_OBJ;
		break;

	  case TS_SUSPEND:
		tcb->suscnt = 0;
		make_ready(tcb);
		break;
	  case TS_WAITSUS:
		tcb->suscnt = 0;
		tcb->state = TS_WAIT;
		break;

	  default:
		ercd = E_SYS;
		break;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Definition of task wait specification
 */
LOCAL CONST WSPEC wspec_slp = { TTW_SLP, NULL, NULL };

/*
 * Move its own task state to wait state
 */
SYSCALL ER _tk_slp_tsk( TMO tmout )
{
	return _tk_slp_tsk_u(to_usec_tmo(tmout));
}

SYSCALL ER _tk_slp_tsk_u( TMO_U tmout )
{
	ER	ercd = E_OK;

	CHECK_TMOUT(tmout);
	CHECK_DISPATCH();

	BEGIN_CRITICAL_SECTION;
	/* Check wait disable */
	if ( (ctxtsk->waitmask & TTW_SLP) != 0 ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	if ( ctxtsk->wupcnt > 0 ) {
		ctxtsk->wupcnt--;
	} else {
		ercd = E_TMOUT;
		if ( tmout != TMO_POL ) {
			ctxtsk->wspec = &wspec_slp;
			ctxtsk->wid = 0;
			ctxtsk->wercd = &ercd;
			make_wait(tmout, TA_NULL);
			QueInit(&ctxtsk->tskque);
		}
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}


/*
 * Wakeup task
 */
SYSCALL ER _tk_wup_tsk( ID tskid )
{
	TCB	*tcb;
	TSTAT	state;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	state = (TSTAT)tcb->state;
	if ( !task_alive(state) ) {
		ercd = ( state == TS_NONEXIST )? E_NOEXS: E_OBJ;

	} else if ( (state & TS_WAIT) != 0 && tcb->wspec == &wspec_slp ) {
		wait_release_ok(tcb);

	} else if ( tcb->wupcnt == INT_MAX ) {
		ercd = E_QOVR;
	} else {
		++tcb->wupcnt;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Cancel wakeup request
 */
SYSCALL INT _tk_can_wup( ID tskid )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	switch ( tcb->state ) {
	  case TS_NONEXIST:
		ercd = E_NOEXS;
		break;
	  case TS_DORMANT:
		ercd = E_OBJ;
		break;

	  default:
		ercd = tcb->wupcnt;
		tcb->wupcnt = 0;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */

#define toTTW(evtmask)		( (UINT)(evtmask) << 16 )
#define toEVT(tskwait)		( (UB)((tskwait) >> 16) )

/*
 * Wait for task event
 */
SYSCALL INT _tk_wai_tev( INT waiptn, TMO tmout )
{
	return _tk_wai_tev_u(waiptn, to_usec_tmo(tmout));
}

SYSCALL INT _tk_wai_tev_u( INT waiptn, TMO_U tmout )
{
	WSPEC	wspec;
	ER	ercd;

	CHECK_TMOUT(tmout);
	CHECK_PAR((((UINT)waiptn & ~0x000000ffU) == 0)&&(((UINT)waiptn & 0x000000FFU) != 0));
	CHECK_DISPATCH();

	BEGIN_CRITICAL_SECTION;
	/* Check wait disable */
	if ( (ctxtsk->waitmask & toTTW(waiptn)) != 0 ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	if ( (ctxtsk->tskevt & waiptn) != 0 ) {
		ercd = ctxtsk->tskevt;
		ctxtsk->tskevt &= ~waiptn;
	} else {
		ercd = E_TMOUT;
		if ( tmout != TMO_POL ) {
			wspec.tskwait = toTTW(waiptn);
			wspec.chg_pri_hook = NULL;
			wspec.rel_wai_hook = NULL;
			ctxtsk->wspec = &wspec;
			ctxtsk->wid = 0;
			ctxtsk->wercd = &ercd;
			make_wait(tmout, TA_NULL);
			QueInit(&ctxtsk->tskque);
		}
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Send task event
 */
SYSCALL ER _tk_sig_tev( ID tskid, INT tskevt )
{
	UINT	evtmsk;
	TCB	*tcb;
	TSTAT	state;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);
	CHECK_PAR(tskevt >= 1 && tskevt <= 8);

	evtmsk = (UINT)(1 << (tskevt - 1));
	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	state = (TSTAT)tcb->state;
	if ( !task_alive(state) ) {
		ercd = ( state == TS_NONEXIST )? E_NOEXS: E_OBJ;
		goto error_exit;
	}

	if ( (state & TS_WAIT) != 0 && (tcb->wspec->tskwait & toTTW(evtmsk)) != 0 ) {
		wait_release_ok_ercd(tcb, (ER)(tcb->tskevt | evtmsk));
	} else {
		tcb->tskevt |= evtmsk;
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
#ifdef	_BTRON_
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_bk_slp_tsk
 Input		:struct task *task
 		 < task to sleep >
 Output		:void
 Return		:ER
 		 < result >
 Description	:move a task state to wait state for btron
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER _bk_slp_tsk(struct task *task)
{
	ER	ercd;
	
	ercd = E_OK;
	
	if (0 < task->wupcnt) {
		task->wupcnt--;
	} else {
		task->wspec = &wspec_slp;
		task->wid = 0;
		task->wercd = &ercd;
		make_wait_task(task, TMO_FEVR, TA_NULL);
		QueInit(&task->tskque);
	}
	
	return(ercd);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_bk_wup_tsk
 Input		:struct task *task
 		 < task to wake up >
 Output		:void
 Return		:ER
 		 < result >
 Description	:wake up a task for btron
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER _bk_wup_tsk(struct task *task)
{
	ER	ercd = E_OK;
	TSTAT	state;
	
	state = (TSTAT)task->state;
	if ( !task_alive(state) ) {
		ercd = ( state == TS_NONEXIST )? E_NOEXS: E_OBJ;

	} else if ( (state & TS_WAIT) != 0 && task->wspec == &wspec_slp ) {
		wait_release_ok(task);

	} else if ( task->wupcnt == INT_MAX ) {
		ercd = E_QOVR;
	} else {
		++task->wupcnt;
	}
	
	return(ercd);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_bk_sus_tsk
 Input		:struct task *task
 		 < task to suspend >
 Output		:void
 Return		:ER
 		 < result >
 Description	:suspend a task for btron
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER _bk_sus_tsk(struct task *task)
{
	TSTAT	state;
	ER	ercd = E_OK;
	
	state = (TSTAT)task->state;
	if ( !task_alive(state) ) {
		ercd = ( state == TS_NONEXIST )? E_NOEXS: E_OBJ;
		goto error_exit;
	}
	if ( task == ctxtsk && dispatch_disabled >= DDS_DISABLE ) {
		ercd = E_CTX;
		goto error_exit;
	}
	if ( task->suscnt == INT_MAX ) {
		ercd = E_QOVR;
		goto error_exit;
	}

	/* Update suspend request count */
	++task->suscnt;

	/* Move to forced wait state */
	if ( state == TS_READY ) {
		make_non_ready(task);
		task->state = TS_SUSPEND;

	} else if ( state == TS_WAIT ) {
		task->state = TS_WAITSUS;
	}

    error_exit:

	return ercd;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_bk_rsm_tsk
 Input		:struct task *task
 		 < task to resume from suspend >
 Output		:void
 Return		:ER
 		 < result >
 Description	:resume a task for btron
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER _bk_rsm_tsk(struct task *task)
{
	ER	ercd = E_OK;

	switch ( task->state ) {
	  case TS_NONEXIST:
		ercd = E_NOEXS;
		break;

	  case TS_DORMANT:
	  case TS_READY:
	  case TS_WAIT:
		ercd = E_OBJ;
		break;

	  case TS_SUSPEND:
		if ( --task->suscnt == 0 ) {
			make_ready(task);
		}
		break;
	  case TS_WAITSUS:
		if ( --task->suscnt == 0 ) {
			task->state = TS_WAIT;
		}
		break;

	  default:
		ercd = E_SYS;
		break;
	}

	return ercd;
}

#endif	// _BTRON_
