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
 *	eventflag.c (T-Kernel/OS)
 *	Event Flag
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "wait.h"
#include "check.h"
#include <sys/rominfo.h>

#ifdef NUM_FLGID

EXPORT ID	max_flgid;	/* Maximum event flag ID */

/*
 * Event flag control block
 */
typedef struct eventflag_control_block {
	QUEUE	wait_queue;	/* Event flag wait queue */
	ID	flgid;		/* Event flag ID */
	void	*exinf;		/* Extended information */
	ATR	flgatr;		/* Event flag attribute */
	UINT	flgptn;		/* Event flag current pattern */
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
} FLGCB;

LOCAL FLGCB	*flgcb_table;	/* Event flag control block */
LOCAL QUEUE	free_flgcb;	/* FreeQue */

#define get_flgcb(id)	( &flgcb_table[INDEX_FLG(id)] )


/*
 * Initialization of event flag control block
 */
EXPORT ER eventflag_initialize( void )
{
	FLGCB	*flgcb, *end;
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXFLGID, &max_flgid, 1);
	if ( n < 1 || NUM_FLGID < 1 ) {
		return E_SYS;
	}

	/* Create event flag control block */
	flgcb_table = Imalloc((UINT)NUM_FLGID * sizeof(FLGCB));
	if ( flgcb_table == NULL ) {
		return E_NOMEM;
	}

	/* Register all control blocks onto FreeQue */
	QueInit(&free_flgcb);
	end = flgcb_table + NUM_FLGID;
	for ( flgcb = flgcb_table; flgcb < end; flgcb++ ) {
		flgcb->flgid = 0;
		QueInsert(&flgcb->wait_queue, &free_flgcb);
	}

	return E_OK;
}

/*
 * Check for event flag wait release condition
 */
Inline BOOL eventflag_cond( FLGCB *flgcb, UINT waiptn, UINT wfmode )
{
	if ( (wfmode & TWF_ORW) != 0 ) {
		return ( (flgcb->flgptn & waiptn) != 0 );
	} else {
		return ( (flgcb->flgptn & waiptn) == waiptn );
	}
}

/*
 * Processing if the priority of wait task changes
 */
LOCAL void flg_chg_pri( TCB *tcb, INT oldpri )
{
	FLGCB	*flgcb;

	flgcb = get_flgcb(tcb->wid);
	gcb_change_priority((GCB*)flgcb, tcb);
}

/*
 * Definition of event flag wait specification
 */
LOCAL CONST WSPEC wspec_flg_tfifo = { TTW_FLG, NULL, NULL };
LOCAL CONST WSPEC wspec_flg_tpri  = { TTW_FLG, flg_chg_pri, NULL };


/*
 * Create event flag
 */
SYSCALL ID _tk_cre_flg( CONST T_CFLG *pk_cflg )
{
#if CHK_RSATR
	const ATR VALID_FLGATR = {
		 TA_TPRI
		|TA_WMUL
		|TA_NODISWAI
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	FLGCB	*flgcb;
	ID	flgid;
	ER	ercd;

	CHECK_RSATR(pk_cflg->flgatr, VALID_FLGATR);

	BEGIN_CRITICAL_SECTION;
	/* Get control block from FreeQue */
	flgcb = (FLGCB*)QueRemoveNext(&free_flgcb);
	if ( flgcb == NULL ) {
		ercd = E_LIMIT;
	} else {
		flgid = ID_FLG(flgcb - flgcb_table);

		/* Initialize control block */
		QueInit(&flgcb->wait_queue);
		flgcb->flgid = flgid;
		flgcb->exinf = pk_cflg->exinf;
		flgcb->flgatr = pk_cflg->flgatr;
		flgcb->flgptn = pk_cflg->iflgptn;
#if USE_OBJECT_NAME
		if ( (pk_cflg->flgatr & TA_DSNAME) != 0 ) {
			strncpy((char*)flgcb->name, (char*)pk_cflg->dsname,
				OBJECT_NAME_LENGTH);
		}
#endif
		ercd = flgid;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Delete event flag
 */
SYSCALL ER _tk_del_flg( ID flgid )
{
	FLGCB	*flgcb;
	ER	ercd = E_OK;

	CHECK_FLGID(flgid);

	flgcb = get_flgcb(flgid);

	BEGIN_CRITICAL_SECTION;
	if ( flgcb->flgid == 0 ) {
		ercd = E_NOEXS;
	} else {
		/* Release wait state of task (E_DLT) */
		wait_delete(&flgcb->wait_queue);

		/* Return to FreeQue */
		QueInsert(&flgcb->wait_queue, &free_flgcb);
		flgcb->flgid = 0;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Event flag set
 */
SYSCALL ER _tk_set_flg( ID flgid, UINT setptn )
{
	FLGCB	*flgcb;
	TCB	*tcb;
	QUEUE	*queue;
	UINT	wfmode, waiptn;
	ER	ercd = E_OK;

	CHECK_FLGID(flgid);

	flgcb = get_flgcb(flgid);

	BEGIN_CRITICAL_SECTION;
	if ( flgcb->flgid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}

	/* Set event flag */
	flgcb->flgptn |= setptn;

	/* Search task which should be released */
	queue = flgcb->wait_queue.next;
	while ( queue != &flgcb->wait_queue ) {
		tcb = (TCB*)queue;
		queue = queue->next;

		/* Meet condition for release wait? */
		waiptn = tcb->winfo.flg.waiptn;
		wfmode = tcb->winfo.flg.wfmode;
		if ( eventflag_cond(flgcb, waiptn, wfmode) ) {

			/* Release wait */
			*tcb->winfo.flg.p_flgptn = flgcb->flgptn;
			wait_release_ok(tcb);

			/* Clear event flag */
			if ( (wfmode & TWF_BITCLR) != 0 ) {
				if ( (flgcb->flgptn &= ~waiptn) == 0 ) {
					break;
				}
			}
			if ( (wfmode & TWF_CLR) != 0 ) {
				flgcb->flgptn = 0;
				break;
			}
		}
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Clear event flag
 */
SYSCALL ER _tk_clr_flg( ID flgid, UINT clrptn )
{
	FLGCB	*flgcb;
	ER	ercd = E_OK;

	CHECK_FLGID(flgid);

	flgcb = get_flgcb(flgid);

	BEGIN_CRITICAL_SECTION;
	if ( flgcb->flgid == 0 ) {
		ercd = E_NOEXS;
	} else {
		flgcb->flgptn &= clrptn;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Event flag wait
 */
SYSCALL ER _tk_wai_flg( ID flgid, UINT waiptn, UINT wfmode, UINT *p_flgptn, TMO tmout )
{
	return _tk_wai_flg_u(flgid, waiptn, wfmode, p_flgptn, to_usec_tmo(tmout));
}

SYSCALL ER _tk_wai_flg_u( ID flgid, UINT waiptn, UINT wfmode, UINT *p_flgptn, TMO_U tmout )
{
	FLGCB	*flgcb;
	ER	ercd = E_OK;

	CHECK_FLGID(flgid);
	CHECK_PAR(waiptn != 0);
	CHECK_PAR((wfmode & ~(TWF_ORW|TWF_CLR|TWF_BITCLR)) == 0);
	CHECK_TMOUT(tmout);
	CHECK_DISPATCH();

	flgcb = get_flgcb(flgid);

	BEGIN_CRITICAL_SECTION;
	if ( flgcb->flgid == 0 ) {
		vd_printf("0\n");
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (flgcb->flgatr & TA_WMUL) == 0 && !isQueEmpty(&flgcb->wait_queue) ) {
		/* Disable multiple tasks wait */
		ercd = E_OBJ;
		goto error_exit;
	}

	/* Check wait disable */
	if ( is_diswai((GCB*)flgcb, ctxtsk, TTW_FLG) ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	/* Meet condition for release wait? */
	if ( eventflag_cond(flgcb, waiptn, wfmode) ) {
		*p_flgptn = flgcb->flgptn;

		/* Clear event flag */
		if ( (wfmode & TWF_BITCLR) != 0 ) {
			flgcb->flgptn &= ~waiptn;
		}
		if ( (wfmode & TWF_CLR) != 0 ) {
			flgcb->flgptn = 0;
		}
	} else {
		/* Ready for wait */
		ctxtsk->wspec = ( (flgcb->flgatr & TA_TPRI) != 0 )?
					&wspec_flg_tpri: &wspec_flg_tfifo;
		ctxtsk->wercd = &ercd;
		ctxtsk->winfo.flg.waiptn = waiptn;
		ctxtsk->winfo.flg.wfmode = wfmode;
		ctxtsk->winfo.flg.p_flgptn = p_flgptn;
		gcb_make_wait_with_diswai((GCB*)flgcb, tmout);
	}

    error_exit:
#if 0
	vd_printf("_cpsr_ : 0x%08X ", _cpsr_);
	vd_printf("!isDI(_cpsr_) : %d ", !isDI(_cpsr_));
	vd_printf("ctxtsk != schedtsk : %d\n", ctxtsk != schedtsk);
	vd_printf("!isTaskIndependent() : %d ", !isTaskIndependent());
	vd_printf("!dispatch_disabled : %d\n", !dispatch_disabled);

#endif
#if 1
if ( !isDI(_cpsr_)
 && ctxtsk != schedtsk
  && !isTaskIndependent()
 && !dispatch_disabled ) {
 	//vd_printf("start dispatch\n");
					dispatch();
				}
	//vd_printf("end dispatch\n");
	
				enaint(_cpsr_); }
#else
	END_CRITICAL_SECTION;
#endif

	return ercd;
}

/*
 * Check event flag state
 */
SYSCALL ER _tk_ref_flg( ID flgid, T_RFLG *pk_rflg )
{
	FLGCB	*flgcb;
	ER	ercd = E_OK;

	CHECK_FLGID(flgid);

	flgcb = get_flgcb(flgid);

	BEGIN_CRITICAL_SECTION;
	if ( flgcb->flgid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rflg->exinf = flgcb->exinf;
		pk_rflg->wtsk = wait_tskid(&flgcb->wait_queue);
		pk_rflg->flgptn = flgcb->flgptn;
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
EXPORT ER eventflag_getname(ID id, UB **name)
{
	FLGCB	*flgcb;
	ER	ercd = E_OK;

	CHECK_FLGID(id);

	BEGIN_DISABLE_INTERRUPT;
	flgcb = get_flgcb(id);
	if ( flgcb->flgid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (flgcb->flgatr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = flgcb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer event flag usage state
 */
SYSCALL INT _td_lst_flg( ID list[], INT nent )
{
	FLGCB	*flgcb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = flgcb_table + NUM_FLGID;
	for ( flgcb = flgcb_table; flgcb < end; flgcb++ ) {
		if ( flgcb->flgid == 0 ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = flgcb->flgid;
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer event flag state
 */
SYSCALL ER _td_ref_flg( ID flgid, TD_RFLG *pk_rflg )
{
	FLGCB	*flgcb;
	ER	ercd = E_OK;

	CHECK_FLGID(flgid);

	flgcb = get_flgcb(flgid);

	BEGIN_DISABLE_INTERRUPT;
	if ( flgcb->flgid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rflg->exinf = flgcb->exinf;
		pk_rflg->wtsk = wait_tskid(&flgcb->wait_queue);
		pk_rflg->flgptn = flgcb->flgptn;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Refer event flag wait queue
 */
SYSCALL INT _td_flg_que( ID flgid, ID list[], INT nent )
{
	FLGCB	*flgcb;
	QUEUE	*q;
	ER	ercd = E_OK;

	CHECK_FLGID(flgid);

	flgcb = get_flgcb(flgid);

	BEGIN_DISABLE_INTERRUPT;
	if ( flgcb->flgid == 0 ) {
		ercd = E_NOEXS;
	} else {
		INT n = 0;
		for ( q = flgcb->wait_queue.next; q != &flgcb->wait_queue; q = q->next ) {
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
#endif /* NUM_FLGID */
