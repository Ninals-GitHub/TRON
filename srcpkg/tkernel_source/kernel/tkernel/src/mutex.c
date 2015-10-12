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
 *	mutex.c (T-Kernel/OS)
 *	Mutex
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "wait.h"
#include "check.h"
#include <sys/rominfo.h>

#ifdef NUM_MTXID

EXPORT ID	max_mtxid;	/* Maximum mutex ID */

#ifndef __mtxcb__
#define __mtxcb__
typedef struct mutex_control_block	MTXCB;
#endif

/*
 * Mutex control block
 */
struct mutex_control_block {
	QUEUE	wait_queue;	/* Mutex wait queue */
	ID	mtxid;		/* Mutex ID */
	void	*exinf;		/* Extended information */
	ATR	mtxatr;		/* Mutex attribute */
	UB	ceilpri;	/* Highest priority limit of mutex */
	TCB	*mtxtsk;	/* Mutex get task */
	MTXCB	*mtxlist;	/* Mutex get list */
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
};

LOCAL MTXCB	*mtxcb_table;	/* Mutex control block */
LOCAL QUEUE	free_mtxcb;	/* FreeQue */

#define get_mtxcb(id)	( &mtxcb_table[INDEX_MTX(id)] )


/*
 * Initialization of mutex control block
 */
EXPORT ER mutex_initialize(void)
{
	MTXCB	*mtxcb, *end;
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXMTXID, &max_mtxid, 1);
	if ( n < 1 || NUM_MTXID < 1 ) {
		return E_SYS;
	}

	/* Create mutex control block */
	mtxcb_table = Imalloc((UINT)NUM_MTXID * sizeof(MTXCB));
	if ( mtxcb_table == NULL ) {
		return E_NOMEM;
	}

	/* Register all control blocks onto FeeQue */
	QueInit(&free_mtxcb);
	end = mtxcb_table + NUM_MTXID;
	for( mtxcb = mtxcb_table; mtxcb < end; mtxcb++ ) {
		mtxcb->mtxid = 0;
		QueInsert(&mtxcb->wait_queue, &free_mtxcb);
	}

	return E_OK;
}


/*
 * If there is a mutex 'mtxcb' with the task of lock wait, it is TRUE
 */
#define mtx_waited(mtxcb)	( !isQueEmpty(&(mtxcb)->wait_queue) )

/*
 * Return the highest priority in the task of lock wait at mutex 'mtxcb'
 */
#define mtx_head_pri(mtxcb)	( ((TCB*)(mtxcb)->wait_queue.next)->priority )

/*
 * Release the lock and delete it from list, and then adjust the
 * priority of task.
 * Set the highest priority between listed below:
 *	(A) The highest priority in all mutexes in which 'tcb' task locks.
 *	(B) The base priority of 'tcb' task.
 */
LOCAL void release_mutex( TCB *tcb, MTXCB *relmtxcb )
{
	MTXCB	*mtxcb, **prev;
	INT	newpri, pri;

	/* (B) The base priority of task */
	newpri = tcb->bpriority;

	/* (A) The highest priority in mutex which is locked */
	pri = newpri;
	prev = &tcb->mtxlist;
	while ( (mtxcb = *prev) != NULL ) {
		if ( mtxcb == relmtxcb ) {
			/* Delete from list */
			*prev = mtxcb->mtxlist;
			continue;
		}

		switch ( mtxcb->mtxatr & TA_CEILING ) {
		  case TA_CEILING:
			pri = mtxcb->ceilpri;
			break;
		  case TA_INHERIT:
			if ( mtx_waited(mtxcb) ) {
				pri = mtx_head_pri(mtxcb);
			}
			break;
		  default: /* TA_TFIFO, TA_TPRI */
			/* nothing to do */
			break;
		}
		if ( newpri > pri ) {
			newpri = pri;
		}

		prev = &mtxcb->mtxlist;
	}

	if ( newpri != tcb->priority ) {
		/* Change priority of lock get task */
		change_task_priority(tcb, newpri);
	}
}

/*
 * Reset priority of lock get task (For TA_INHERIT only)
 */
#define reset_priority(tcb)	release_mutex((tcb), NULL)

/*
 * Free mutex when task is terminated
 *	Free all mutexes which the task holds.
 *	Do not need to handle mutex list and priority of terminated task.
 *	
 */
EXPORT void signal_all_mutex( TCB *tcb )
{
	MTXCB	*mtxcb, *next_mtxcb;
	TCB	*next_tcb;

	next_mtxcb = tcb->mtxlist;
	while ( (mtxcb = next_mtxcb) != NULL ) {
		next_mtxcb = mtxcb->mtxlist;

		if ( mtx_waited(mtxcb) ) {
			next_tcb = (TCB*)mtxcb->wait_queue.next;

			/* Wake wait task */
			wait_release_ok(next_tcb);

			/* Change mutex get task */
			mtxcb->mtxtsk = next_tcb;
			mtxcb->mtxlist = next_tcb->mtxlist;
			next_tcb->mtxlist = mtxcb;

			if ( (mtxcb->mtxatr & TA_CEILING) == TA_CEILING ) {
				if ( next_tcb->priority > mtxcb->ceilpri ) {
					/* Raise the priority for the task
					   that got lock to the highest
					   priority limit */
					change_task_priority(next_tcb,
							mtxcb->ceilpri);
				}
			}
		} else {
			/* No wait task */
			mtxcb->mtxtsk = NULL;
		}
	}
}

/*
 * Limit the priority change by mutex at task priority change
 *    1.If the 'tcb' task locks mutex, cannot set lower priority than the
 *	highest priority in all mutexes which hold lock. In such case,
 *	return the highest priority of locked mutex.
 *    2.If mutex with TA_CEILING attribute is locked or waiting to be locked,
 *	cannot set higher priority than the lowest within the highest
 *	priority limit of mutex with TA_CEILING attribute.
 *	In this case, return E_ILUSE.
 *    3.Other than above, return the 'priority'.
 */
EXPORT INT chg_pri_mutex( TCB *tcb, INT priority )
{
	MTXCB	*mtxcb;
	INT	hi_pri, low_pri, pri;

	hi_pri  = priority;
	low_pri = int_priority(MIN_PRI);

	/* Mutex lock wait */
	if ( (tcb->state & TS_WAIT) != 0 && (tcb->wspec->tskwait & TTW_MTX) != 0 ) {
		mtxcb = get_mtxcb(tcb->wid);
		if ( (mtxcb->mtxatr & TA_CEILING) == TA_CEILING ) {
			pri = mtxcb->ceilpri;
			if ( pri > low_pri ) {
				low_pri = pri;
			}
		}
	}

	/* Locked Mutex */
	pri = hi_pri;
	for ( mtxcb = tcb->mtxlist; mtxcb != NULL; mtxcb = mtxcb->mtxlist ) {
		switch ( mtxcb->mtxatr & TA_CEILING ) {
		  case TA_CEILING:
			pri = mtxcb->ceilpri;
			if ( pri > low_pri ) {
				low_pri = pri;
			}
			break;
		  case TA_INHERIT:
			if ( mtx_waited(mtxcb) ) {
				pri = mtx_head_pri(mtxcb);
			}
			break;
		  default: /* TA_TFIFO, TA_TPRI */
			/* nothing to do */
			break;
		}
		if ( pri < hi_pri ) {
			hi_pri = pri;
		}
	}

	if ( priority < low_pri ) {
		return E_ILUSE;
	}
	return hi_pri;
}


/*
 * Processing if the priority of wait task changes
 */
LOCAL void mtx_chg_pri( TCB *tcb, INT oldpri )
{
	MTXCB	*mtxcb;
	TCB	*mtxtsk;

	mtxcb = get_mtxcb(tcb->wid);
	gcb_change_priority((GCB*)mtxcb, tcb);

	if ( (mtxcb->mtxatr & TA_CEILING) == TA_INHERIT ) {
		mtxtsk = mtxcb->mtxtsk;
		if ( mtxtsk->priority > tcb->priority ) {
			/* Since the highest priority of the lock wait task
			   became higher, raise the lock get task priority
			   higher */
			change_task_priority(mtxtsk, tcb->priority);

		} else if ( mtxtsk->priority == oldpri ) {
			/* Since the highest priority of the lock wait task
			   might become lower, adjust this priority */
			reset_priority(mtxtsk);
		}
	}
}

/*
 * Processing if the wait task is released (For TA_INHERIT only)
 */
LOCAL void mtx_rel_wai( TCB *tcb )
{
	MTXCB	*mtxcb;
	TCB	*mtxtsk;

	mtxcb = get_mtxcb(tcb->wid);
	mtxtsk = mtxcb->mtxtsk;

	if ( mtxtsk->priority == tcb->priority ) {
		/* Since the highest priority of the lock wait task might
		   become lower, adjust this priority */
		reset_priority(mtxtsk);
	}
}

/*
 * Definition of mutex wait specification
 */
LOCAL CONST WSPEC wspec_mtx_tfifo   = { TTW_MTX, NULL, NULL };
LOCAL CONST WSPEC wspec_mtx_tpri    = { TTW_MTX, mtx_chg_pri, NULL };
LOCAL CONST WSPEC wspec_mtx_inherit = { TTW_MTX, mtx_chg_pri, mtx_rel_wai };


/*
 * Create mutex
 */
SYSCALL ID _tk_cre_mtx( CONST T_CMTX *pk_cmtx )
{
#if CHK_RSATR
	const ATR VALID_MTXATR = {
		 TA_CEILING
		|TA_NODISWAI
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	MTXCB	*mtxcb;
	ID	mtxid;
	INT	ceilpri;
	ER	ercd;

	CHECK_RSATR(pk_cmtx->mtxatr, VALID_MTXATR);

	if ( (pk_cmtx->mtxatr & TA_CEILING) == TA_CEILING ) {
		CHECK_PRI(pk_cmtx->ceilpri);
		ceilpri = int_priority(pk_cmtx->ceilpri);
	} else {
		ceilpri = 0;
	}

	BEGIN_CRITICAL_SECTION;
	/* Get control block from FreeQue */
	mtxcb = (MTXCB*)QueRemoveNext(&free_mtxcb);
	if ( mtxcb == NULL ) {
		ercd = E_LIMIT;
	} else {
		mtxid = ID_MTX(mtxcb - mtxcb_table);

		/* Initialize control block */
		QueInit(&mtxcb->wait_queue);
		mtxcb->mtxid   = mtxid;
		mtxcb->exinf   = pk_cmtx->exinf;
		mtxcb->mtxatr  = pk_cmtx->mtxatr;
		mtxcb->ceilpri = ceilpri;
		mtxcb->mtxtsk  = NULL;
		mtxcb->mtxlist = NULL;
#if USE_OBJECT_NAME
		if ( (pk_cmtx->mtxatr & TA_DSNAME) != 0 ) {
			strncpy((char*)mtxcb->name, (char*)pk_cmtx->dsname,
				(UINT)OBJECT_NAME_LENGTH);
		}
#endif
		ercd = mtxid;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Delete mutex
 */
SYSCALL ER _tk_del_mtx( ID mtxid )
{
	MTXCB	*mtxcb;
	ER	ercd = E_OK;

	CHECK_MTXID(mtxid);

	mtxcb = get_mtxcb(mtxid);

	BEGIN_CRITICAL_SECTION;
	if ( mtxcb->mtxid == 0 ) {
		ercd = E_NOEXS;
	} else {
		/* If there is a task that holds mutex to delete,
		 * delete the mutex from the list
		 * and adjust the task priority if necessary.
		 */
		if ( mtxcb->mtxtsk != NULL ) {
			release_mutex(mtxcb->mtxtsk, mtxcb);
		}

		/* Free wait state of task (E_DLT) */
		wait_delete(&mtxcb->wait_queue);

		/* Return to FreeQue */
		QueInsert(&mtxcb->wait_queue, &free_mtxcb);
		mtxcb->mtxid = 0;
	}
	END_CRITICAL_SECTION;

	return ercd;
}


/*
 * Lock mutex
 */
SYSCALL ER _tk_loc_mtx( ID mtxid, TMO tmout )
{
	return _tk_loc_mtx_u(mtxid, to_usec_tmo(tmout));
}

SYSCALL ER _tk_loc_mtx_u( ID mtxid, TMO_U tmout )
{
	MTXCB	*mtxcb;
	TCB	*mtxtsk;
	ATR	mtxatr;
	ER	ercd = E_OK;

	CHECK_MTXID(mtxid);
	CHECK_TMOUT(tmout);
	CHECK_DISPATCH();

	mtxcb = get_mtxcb(mtxid);

	BEGIN_CRITICAL_SECTION;
	if ( mtxcb->mtxid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( mtxcb->mtxtsk == ctxtsk ) {
		ercd = E_ILUSE;  /* Multiplexed lock */
		goto error_exit;
	}

	mtxatr = mtxcb->mtxatr & TA_CEILING;
	if ( mtxatr == TA_CEILING ) {
		if ( ctxtsk->bpriority < mtxcb->ceilpri ) {
			/* Violation of highest priority limit */
			ercd = E_ILUSE;
			goto error_exit;
		}
	}

	/* Check wait disable */
	if ( is_diswai((GCB*)mtxcb, ctxtsk, TTW_MTX) ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	mtxtsk = mtxcb->mtxtsk;
	if ( mtxtsk == NULL ) {
		/* Get lock */
		mtxcb->mtxtsk = ctxtsk;
		mtxcb->mtxlist = ctxtsk->mtxlist;
		ctxtsk->mtxlist = mtxcb;

		if ( mtxatr == TA_CEILING ) {
			if ( ctxtsk->priority > mtxcb->ceilpri ) {
				/* Raise its own task to the highest
				   priority limit */
				change_task_priority(ctxtsk, mtxcb->ceilpri);
			}
		}
	} else {
		ercd = E_TMOUT;
		if ( tmout == TMO_POL ) {
			goto error_exit;
		}

		if ( mtxatr == TA_INHERIT ) {
			if ( mtxtsk->priority > ctxtsk->priority ) {
				/* Raise the priority of task during
				   locking to the same priority as its
				   own task */
				change_task_priority(mtxtsk, ctxtsk->priority);
			}
		}

		/* Ready for wait */
		ctxtsk->wspec = ( mtxatr == TA_TFIFO   )? &wspec_mtx_tfifo:
				( mtxatr == TA_INHERIT )? &wspec_mtx_inherit:
							  &wspec_mtx_tpri;
		ctxtsk->wercd = &ercd;
		ctxtsk->wid = mtxcb->mtxid;
		make_wait(tmout, mtxcb->mtxatr);
		if ( mtxatr == TA_TFIFO ) {
			QueInsert(&ctxtsk->tskque, &mtxcb->wait_queue);
		} else {
			queue_insert_tpri(ctxtsk, &mtxcb->wait_queue);
		}
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Unlock mutex
 */
SYSCALL ER _tk_unl_mtx( ID mtxid )
{
	MTXCB	*mtxcb;	
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_MTXID(mtxid);
	CHECK_INTSK();

	mtxcb = get_mtxcb(mtxid);

	BEGIN_CRITICAL_SECTION;
	if ( mtxcb->mtxid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( mtxcb->mtxtsk != ctxtsk ) {
		ercd = E_ILUSE;  /* This is not locked by its own task */
		goto error_exit;
	}

	/* Delete the mutex from the list,
	   and adjust its own task priority if necessary. */
	release_mutex(ctxtsk, mtxcb);

	if ( mtx_waited(mtxcb) ) {
		tcb = (TCB*)mtxcb->wait_queue.next;

		/* Release wait */
		wait_release_ok(tcb);

		/* Change mutex get task */
		mtxcb->mtxtsk = tcb;
		mtxcb->mtxlist = tcb->mtxlist;
		tcb->mtxlist = mtxcb;

		if ( (mtxcb->mtxatr & TA_CEILING) == TA_CEILING ) {
			if ( tcb->priority > mtxcb->ceilpri ) {
				/* Raise the priority of the task that
				   got lock to the highest priority limit */
				change_task_priority(tcb, mtxcb->ceilpri);
			}
		}
	} else {
		/* No wait task */
		mtxcb->mtxtsk = NULL;
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}


/*
 * Refer mutex state
 */
SYSCALL ER _tk_ref_mtx( ID mtxid, T_RMTX *pk_rmtx )
{
	MTXCB	*mtxcb;
	ER	ercd = E_OK;

	CHECK_MTXID(mtxid);

	mtxcb = get_mtxcb(mtxid);

	BEGIN_CRITICAL_SECTION;
	if ( mtxcb->mtxid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rmtx->exinf = mtxcb->exinf;
		pk_rmtx->htsk = ( mtxcb->mtxtsk != NULL )?
					mtxcb->mtxtsk->tskid: 0;
		pk_rmtx->wtsk = wait_tskid(&mtxcb->wait_queue);
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */
/*
 *	Debugger support function
 */
#if USE_DBGSPT

/*
 * Get object name from control block
 */
#if USE_OBJECT_NAME
EXPORT ER mutex_getname(ID id, UB **name)
{
	MTXCB	*mtxcb;
	ER	ercd = E_OK;

	CHECK_MTXID(id);

	BEGIN_DISABLE_INTERRUPT;
	mtxcb = get_mtxcb(id);
	if ( mtxcb->mtxid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (mtxcb->mtxatr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = mtxcb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer mutex usage state
 */
SYSCALL INT _td_lst_mtx( ID list[], INT nent )
{
	MTXCB	*mtxcb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = mtxcb_table + NUM_MTXID;
	for ( mtxcb = mtxcb_table; mtxcb < end; mtxcb++ ) {
		if ( mtxcb->mtxid == 0 ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = mtxcb->mtxid;
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer mutex state
 */
SYSCALL ER _td_ref_mtx( ID mtxid, TD_RMTX *pk_rmtx )
{
	MTXCB	*mtxcb;
	ER	ercd = E_OK;

	CHECK_MTXID(mtxid);

	mtxcb = get_mtxcb(mtxid);

	BEGIN_DISABLE_INTERRUPT;
	if ( mtxcb->mtxid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rmtx->exinf = mtxcb->exinf;
		pk_rmtx->htsk = ( mtxcb->mtxtsk != NULL )?
					mtxcb->mtxtsk->tskid: 0;
		pk_rmtx->wtsk = wait_tskid(&mtxcb->wait_queue);
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Refer mutex wait queue
 */
SYSCALL INT _td_mtx_que( ID mtxid, ID list[], INT nent )
{
	MTXCB	*mtxcb;
	QUEUE	*q;
	ER	ercd = E_OK;

	CHECK_MTXID(mtxid);

	mtxcb = get_mtxcb(mtxid);

	BEGIN_DISABLE_INTERRUPT;
	if ( mtxcb->mtxid == 0 ) {
		ercd = E_NOEXS;
	} else {
		INT n = 0;
		for ( q = mtxcb->wait_queue.next; q != &mtxcb->wait_queue; q = q->next ) {
			if ( n++ < nent ) {
				*list++ = ((TCB*)q)->tskid;
			}
		}
		ercd = n;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

#endif /* USE_DBGSPT */
#endif /* NUM_MTXID */
