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
 *	semaphore.c (T-Kernel/OS)
 *	Semaphore
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "wait.h"
#include "check.h"
#include <sys/rominfo.h>

#ifdef NUM_SEMID

EXPORT ID	max_semid;	/* Maximum semaphore ID */

/*
 * Semaphore control block
 */
typedef struct semaphore_control_block {
	QUEUE	wait_queue;	/* Semaphore wait queue */
	ID	semid;		/* Semaphore ID */
	void	*exinf;		/* Extended information */
	ATR	sematr;		/* Semaphore attribute */
	INT	semcnt;		/* Semaphore current count value */
	INT	maxsem;		/* Semaphore maximum count value */
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
} SEMCB;

LOCAL SEMCB	*semcb_table;	/* Semaphore control block */
LOCAL QUEUE	free_semcb;	/* FreeQue */

#define get_semcb(id)	( &semcb_table[INDEX_SEM(id)] )


/*
 * Initialization of semaphore control block
 */
EXPORT ER semaphore_initialize( void )
{
	SEMCB	*semcb, *end;
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXSEMID, &max_semid, 1);
	if ( n < 1 || NUM_SEMID < 1 ) {
		return E_SYS;
	}

	/* Semaphore control block */
	semcb_table = Imalloc((UINT)NUM_SEMID * sizeof(SEMCB));
	if ( semcb_table == NULL ) {
		return E_NOMEM;
	}

	/* Register all control blocks onto FeeQue */
	QueInit(&free_semcb);
	end = semcb_table + NUM_SEMID;
	for ( semcb = semcb_table; semcb < end; semcb++ ) {
		semcb->semid = 0;
		QueInsert(&semcb->wait_queue, &free_semcb);
	}

	return E_OK;
}


/*
 * Processing if the priority of wait task changes
 */
LOCAL void sem_chg_pri( TCB *tcb, INT oldpri )
{
	SEMCB	*semcb;
	QUEUE	*queue;
	TCB	*top;

	semcb = get_semcb(tcb->wid);
	if ( oldpri >= 0 ) {
		/* Reorder wait line */
		gcb_change_priority((GCB*)semcb, tcb);
	}

	if ( (semcb->sematr & TA_CNT) != 0 ) {
		return;
	}

	/* From the head task in a wait queue, allocate semaphore counts
	   and release wait state as much as possible */
	queue = semcb->wait_queue.next;
	while ( queue != &semcb->wait_queue ) {
		top = (TCB*)queue;
		queue = queue->next;

		/* Meet condition for releasing wait? */
		if ( semcb->semcnt < top->winfo.sem.cnt ) {
			break;
		}

		/* Release wait */
		wait_release_ok(top);

		semcb->semcnt -= top->winfo.sem.cnt;
	}
}

/*
 * Processing if the wait task is freed
 */
LOCAL void sem_rel_wai( TCB *tcb )
{
	sem_chg_pri(tcb, -1);
}

/*
 * Definition of semaphore wait specification
 */
LOCAL CONST WSPEC wspec_sem_tfifo = { TTW_SEM, NULL,        sem_rel_wai };
LOCAL CONST WSPEC wspec_sem_tpri  = { TTW_SEM, sem_chg_pri, sem_rel_wai };


/*
 * Create semaphore
 */
SYSCALL ID _tk_cre_sem( CONST T_CSEM *pk_csem )
{
#if CHK_RSATR
	const ATR VALID_SEMATR = {
		 TA_TPRI
		|TA_CNT
		|TA_NODISWAI
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	SEMCB	*semcb;
	ID	semid;
	ER	ercd;

	CHECK_RSATR(pk_csem->sematr, VALID_SEMATR);
	CHECK_PAR(pk_csem->isemcnt >= 0);
	CHECK_PAR(pk_csem->maxsem > 0);
	CHECK_PAR(pk_csem->maxsem >= pk_csem->isemcnt);

	BEGIN_CRITICAL_SECTION;
	/* Get control block from FreeQue */
	semcb = (SEMCB*)QueRemoveNext(&free_semcb);
	if ( semcb == NULL ) {
		ercd = E_LIMIT;
	} else {
		semid = ID_SEM(semcb - semcb_table);

		/* Initialize control block */
		QueInit(&semcb->wait_queue);
		semcb->semid = semid;
		semcb->exinf = pk_csem->exinf;
		semcb->sematr = pk_csem->sematr;
		semcb->semcnt = pk_csem->isemcnt;
		semcb->maxsem = pk_csem->maxsem;
#if USE_OBJECT_NAME
		if ( (pk_csem->sematr & TA_DSNAME) != 0 ) {
			strncpy((char*)semcb->name, (char*)pk_csem->dsname,
				OBJECT_NAME_LENGTH);
		}
#endif
		ercd = semid;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Delete semaphore
 */
SYSCALL ER _tk_del_sem( ID semid )
{
	SEMCB	*semcb;
	ER	ercd = E_OK;

	CHECK_SEMID(semid);

	semcb = get_semcb(semid);

	BEGIN_CRITICAL_SECTION;
	if ( semcb->semid == 0 ) {
		ercd = E_NOEXS;
	} else {
		/* Release wait state of task (E_DLT) */
		wait_delete(&semcb->wait_queue);

		/* Return to FreeQue */
		QueInsert(&semcb->wait_queue, &free_semcb);
		semcb->semid = 0;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Signal semaphore
 */
SYSCALL ER _tk_sig_sem( ID semid, INT cnt )
{
	SEMCB	*semcb;
	TCB	*tcb;
	QUEUE	*queue;
	ER	ercd = E_OK;
 
	CHECK_SEMID(semid);
	CHECK_PAR(cnt > 0);

	semcb = get_semcb(semid);

	BEGIN_CRITICAL_SECTION;
	if ( semcb->semid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( cnt > (semcb->maxsem - semcb->semcnt) ) {
		ercd = E_QOVR;
		goto error_exit;
	}

	/* Return semaphore counts */
	semcb->semcnt += cnt;

	/* Search task that frees wait */
	queue = semcb->wait_queue.next;
	while ( queue != &semcb->wait_queue ) {
		tcb = (TCB*)queue;
		queue = queue->next;

		/* Meet condition for Releasing wait? */
		if ( semcb->semcnt < tcb->winfo.sem.cnt ) {
			if ( (semcb->sematr & TA_CNT) == 0 ) {
				break;
			}
			continue;
		}

		/* Release wait */
		wait_release_ok(tcb);

		semcb->semcnt -= tcb->winfo.sem.cnt;
		if ( semcb->semcnt <= 0 ) {
			break;
		}
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Wait on semaphore
 */
SYSCALL ER _tk_wai_sem( ID semid, INT cnt, TMO tmout )
{
	return _tk_wai_sem_u(semid, cnt, to_usec_tmo(tmout));
}

SYSCALL ER _tk_wai_sem_u( ID semid, INT cnt, TMO_U tmout )
{
	SEMCB	*semcb;
	ER	ercd = E_OK;

	CHECK_SEMID(semid);
	CHECK_PAR(cnt > 0);
	CHECK_TMOUT(tmout);
	CHECK_DISPATCH();

	semcb = get_semcb(semid);

	BEGIN_CRITICAL_SECTION;
	if ( semcb->semid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
#if CHK_PAR
	if ( cnt > semcb->maxsem ) {
		ercd = E_PAR;
		goto error_exit;
	}
#endif

	/* Check wait disable */
	if ( is_diswai((GCB*)semcb, ctxtsk, TTW_SEM) ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	if ( ((semcb->sematr & TA_CNT) != 0
	      || gcb_top_of_wait_queue((GCB*)semcb, ctxtsk) == ctxtsk)
	  && semcb->semcnt >= cnt ) {
		/* Get semaphore count */
		semcb->semcnt -= cnt;

	} else {
		/* Ready for wait */
		ctxtsk->wspec = ( (semcb->sematr & TA_TPRI) != 0 )?
					&wspec_sem_tpri: &wspec_sem_tfifo;
		ctxtsk->wercd = &ercd;
		ctxtsk->winfo.sem.cnt = cnt;
		gcb_make_wait_with_diswai((GCB*)semcb, tmout);
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Refer semaphore state
 */
SYSCALL ER _tk_ref_sem( ID semid, T_RSEM *pk_rsem )
{
	SEMCB	*semcb;
	ER	ercd = E_OK;

	CHECK_SEMID(semid);

	semcb = get_semcb(semid);

	BEGIN_CRITICAL_SECTION;
	if ( semcb->semid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rsem->exinf  = semcb->exinf;
		pk_rsem->wtsk   = wait_tskid(&semcb->wait_queue);
		pk_rsem->semcnt = semcb->semcnt;
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
EXPORT ER semaphore_getname(ID id, UB **name)
{
	SEMCB	*semcb;
	ER	ercd = E_OK;

	CHECK_SEMID(id);

	BEGIN_DISABLE_INTERRUPT;
	semcb = get_semcb(id);
	if ( semcb->semid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (semcb->sematr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = semcb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer object usage state
 */
SYSCALL INT _td_lst_sem( ID list[], INT nent )
{
	SEMCB	*semcb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = semcb_table + NUM_SEMID;
	for ( semcb = semcb_table; semcb < end; semcb++ ) {
		if ( semcb->semid == 0 ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = semcb->semid;
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer object state
 */
SYSCALL ER _td_ref_sem( ID semid, TD_RSEM *rsem )
{
	SEMCB	*semcb;
	ER	ercd = E_OK;

	CHECK_SEMID(semid);

	semcb = get_semcb(semid);

	BEGIN_DISABLE_INTERRUPT;
	if ( semcb->semid == 0 ) {
		ercd = E_NOEXS;
	} else {
		rsem->exinf  = semcb->exinf;
		rsem->wtsk   = wait_tskid(&semcb->wait_queue);
		rsem->semcnt = semcb->semcnt;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Refer wait queue
 */
SYSCALL INT _td_sem_que( ID semid, ID list[], INT nent )
{
	SEMCB	*semcb;
	QUEUE	*q;
	ER	ercd;

	CHECK_SEMID(semid);

	semcb = get_semcb(semid);

	BEGIN_DISABLE_INTERRUPT;
	if ( semcb->semid == 0 ) {
		ercd = E_NOEXS;
	} else {
		INT	n = 0;
		for ( q = semcb->wait_queue.next; q != &semcb->wait_queue; q = q->next ) {
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
#endif /* NUM_SEMID */
