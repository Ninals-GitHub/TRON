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
 *	rendezvous.c (T-Kernel/OS)
 *	Rendezvous
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include <tk/wait.h>
#include "check.h"
#include <sys/rominfo.h>

#ifdef NUM_PORID

EXPORT ID	max_porid;	/* Maximum rendezvous port ID */

/*
 * Rendezvous port control block
 */
typedef struct port_control_block {
	QUEUE	call_queue;	/* Port call wait queue */
	ID	porid;		/* Port ID */
	void	*exinf;		/* Extended information */
	ATR	poratr;		/* Port attribute */
	QUEUE	accept_queue;	/* Port accept wait queue */
	INT	maxcmsz;	/* Maximum length of call message */
	INT	maxrmsz;	/* Maximum length of reply message */
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
} PORCB;

LOCAL PORCB	*porcb_table;	/* Rendezvous port control block */
LOCAL QUEUE	free_porcb;	/* FreeQue */

#define get_porcb(id)	( &porcb_table[INDEX_POR(id)] )


/*
 * Initialization of port control block
 */
EXPORT ER rendezvous_initialize( void )
{
	PORCB	*porcb, *end;
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXPORID, &max_porid, 1);
	if ( n < 1 || NUM_PORID < 1 ) {
		return E_SYS;
	}

	/* Create port control block */
	porcb_table = Imalloc((UINT)NUM_PORID * sizeof(PORCB));
	if ( porcb_table == NULL ) {
		return E_NOMEM;
	}

	/* Register all control blocks onto FeeQue */
	QueInit(&free_porcb);
	end = porcb_table + NUM_PORID;
	for ( porcb = porcb_table; porcb < end; porcb++ ) {
		porcb->porid = 0;
		QueInsert(&porcb->call_queue, &free_porcb);
	}

	return E_OK;
}


#define RDVNO_SHIFT	16

/*
 * Create rendezvous number
 */
Inline RNO gen_rdvno( TCB *tcb )
{
	RNO	rdvno;

	rdvno = tcb->wrdvno;
	tcb->wrdvno += (1 << RDVNO_SHIFT);

	return rdvno;
}

/*
 * Get task ID from rendezvous number
 */
Inline ID get_tskid_rdvno( RNO rdvno )
{
	return (ID)((UINT)rdvno & ((1 << RDVNO_SHIFT) - 1));
}

/*
 * Check validity of rendezvous number
 */
#define CHECK_RDVNO(rdvno) {					\
	if ( !CHK_TSKID(get_tskid_rdvno(rdvno)) ) {		\
		return E_OBJ;					\
	}							\
}


/*
 * Processing if the priority of send wait task changes
 */
LOCAL void cal_chg_pri( TCB *tcb, INT oldpri )
{
	PORCB	*porcb;

	porcb = get_porcb(tcb->wid);
	gcb_change_priority((GCB*)porcb, tcb);
}

/*
 * Definition of rendezvous wait specification
 */
LOCAL CONST WSPEC wspec_cal_tfifo = { TTW_CAL, NULL, NULL };
LOCAL CONST WSPEC wspec_cal_tpri  = { TTW_CAL, cal_chg_pri, NULL };
LOCAL CONST WSPEC wspec_acp       = { TTW_ACP, NULL, NULL };
LOCAL CONST WSPEC wspec_rdv       = { TTW_RDV, NULL, NULL };


/*
 * Create rendezvous port
 */
SYSCALL ID _tk_cre_por( CONST T_CPOR *pk_cpor )
{
#if CHK_RSATR
	const ATR VALID_PORATR = {
		 TA_TPRI
		|TA_NODISWAI
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	PORCB	*porcb;
	ID	porid;
	ER	ercd;

	CHECK_RSATR(pk_cpor->poratr, VALID_PORATR);
	CHECK_PAR(pk_cpor->maxcmsz >= 0);
	CHECK_PAR(pk_cpor->maxrmsz >= 0);
	CHECK_INTSK();

	BEGIN_CRITICAL_SECTION;
	/* Get control block from FreeQue */
	porcb = (PORCB*)QueRemoveNext(&free_porcb);
	if ( porcb == NULL ) {
		ercd = E_LIMIT;
	} else {
		porid = ID_POR(porcb - porcb_table);

		/* Initialize control block */
		QueInit(&porcb->call_queue);
		porcb->porid = porid;
		porcb->exinf = pk_cpor->exinf;
		porcb->poratr = pk_cpor->poratr;
		QueInit(&porcb->accept_queue);
		porcb->maxcmsz = pk_cpor->maxcmsz;
		porcb->maxrmsz = pk_cpor->maxrmsz;
#if USE_OBJECT_NAME
		if ( (pk_cpor->poratr & TA_DSNAME) != 0 ) {
			strncpy((char*)porcb->name, (char*)pk_cpor->dsname,
				OBJECT_NAME_LENGTH);
		}
#endif
		ercd = porid;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Delete rendezvous port
 */
SYSCALL ER _tk_del_por( ID porid )
{
	PORCB	*porcb;
	ER	ercd = E_OK;

	CHECK_PORID(porid);
	CHECK_INTSK();

	porcb = get_porcb(porid);

	BEGIN_CRITICAL_SECTION;
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
	} else {
		/* Release wait state of task (E_DLT) */
		wait_delete(&porcb->call_queue);
		wait_delete(&porcb->accept_queue);

		/* Return to FreeQue */
		QueInsert(&porcb->call_queue, &free_porcb);
		porcb->porid = 0;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Call rendezvous
 */
SYSCALL INT _tk_cal_por( ID porid, UINT calptn, void *msg, INT cmsgsz, TMO tmout )
{
	return _tk_cal_por_u(porid, calptn, msg, cmsgsz, to_usec_tmo(tmout));
}

SYSCALL INT _tk_cal_por_u( ID porid, UINT calptn, void *msg, INT cmsgsz, TMO_U tmout )
{
	PORCB	*porcb;
	TCB	*tcb;
	QUEUE	*queue;
	RNO	rdvno;
	INT	rmsgsz;
	ER	ercd = E_OK;

	CHECK_PORID(porid);
	CHECK_PAR(calptn != 0);
	CHECK_PAR(cmsgsz >= 0);
	CHECK_TMOUT(tmout);
	CHECK_DISPATCH();

	porcb = get_porcb(porid);

	BEGIN_CRITICAL_SECTION;
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
#if CHK_PAR
	if ( cmsgsz > porcb->maxcmsz ) {
		ercd = E_PAR;
		goto error_exit;
	}
#endif

	/* Search accept wait task */
	queue = porcb->accept_queue.next;
	while ( queue != &porcb->accept_queue ) {
		tcb = (TCB*)queue;
		queue = queue->next;
		if ( (calptn & tcb->winfo.acp.acpptn) == 0 ) {
			continue;
		}

		/* Check rendezvous call wait disable */
		if ( is_diswai((GCB*)porcb, ctxtsk, TTW_CAL) ) {
			ercd = E_DISWAI;
			goto error_exit;
		}

		/* Send message */
		rdvno = gen_rdvno(ctxtsk);
		if ( cmsgsz > 0 ) {
			memcpy(tcb->winfo.acp.msg, msg, (UINT)cmsgsz);
		}
		*tcb->winfo.acp.p_rdvno = rdvno;
		*tcb->winfo.acp.p_cmsgsz = cmsgsz;
		wait_release_ok(tcb);

		/* Check rendezvous end wait disable */
		if ( is_diswai((GCB*)porcb, ctxtsk, TTW_RDV) ) {
			ercd = E_DISWAI;
			goto error_exit;
		}

		/* Ready for rendezvous end wait */
		ercd = E_TMOUT;
		ctxtsk->wspec = &wspec_rdv;
		ctxtsk->wid = 0;
		ctxtsk->wercd = &ercd;
		ctxtsk->winfo.rdv.rdvno = rdvno;
		ctxtsk->winfo.rdv.msg = msg;
		ctxtsk->winfo.rdv.maxrmsz = porcb->maxrmsz;
		ctxtsk->winfo.rdv.p_rmsgsz = &rmsgsz;
		make_wait(TMO_FEVR, porcb->poratr);
		QueInit(&ctxtsk->tskque);

		goto error_exit;
	}

	/* Ready for rendezvous call wait */
	ctxtsk->wspec = ( (porcb->poratr & TA_TPRI) != 0 )?
					&wspec_cal_tpri: &wspec_cal_tfifo;
	ctxtsk->wercd = &ercd;
	ctxtsk->winfo.cal.calptn = calptn;
	ctxtsk->winfo.cal.msg = msg;
	ctxtsk->winfo.cal.cmsgsz = cmsgsz;
	ctxtsk->winfo.cal.p_rmsgsz = &rmsgsz;
	gcb_make_wait_with_diswai((GCB*)porcb, tmout);

    error_exit:
	END_CRITICAL_SECTION;

	return ( ercd < E_OK )? ercd: rmsgsz;
}

/*
 * Accept rendezvous
 */
SYSCALL INT _tk_acp_por( ID porid, UINT acpptn, RNO *p_rdvno, void *msg, TMO tmout )
{
	return _tk_acp_por_u(porid, acpptn, p_rdvno, msg, to_usec_tmo(tmout));
}

SYSCALL INT _tk_acp_por_u( ID porid, UINT acpptn, RNO *p_rdvno, void *msg, TMO_U tmout )
{
	PORCB	*porcb;
	TCB	*tcb;
	QUEUE	*queue;
	RNO	rdvno;
	INT	cmsgsz;
	ER	ercd = E_OK;

	CHECK_PORID(porid);
	CHECK_PAR(acpptn != 0);
	CHECK_TMOUT(tmout);
	CHECK_DISPATCH();

	porcb = get_porcb(porid);
 
	BEGIN_CRITICAL_SECTION;
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}

	/* Search call wait task */
	queue = porcb->call_queue.next;
	while ( queue != &porcb->call_queue ) {
		tcb = (TCB*)queue;
		queue = queue->next;
		if ( (acpptn & tcb->winfo.cal.calptn) == 0 ) {
			continue;
		}

		/* Check rendezvous accept wait disable */
		if ( is_diswai((GCB*)porcb, ctxtsk, TTW_ACP) ) {
			ercd = E_DISWAI;
			goto error_exit;
		}

		/* Receive message */
		*p_rdvno = rdvno = gen_rdvno(tcb);
		cmsgsz = tcb->winfo.cal.cmsgsz;
		if ( cmsgsz > 0 ) {
			memcpy(msg, tcb->winfo.cal.msg, (UINT)cmsgsz);
		}

		/* Check rendezvous end wait disable */
		if ( is_diswai((GCB*)porcb, tcb, TTW_RDV) ) {
			wait_release_ng(tcb, E_DISWAI);
			goto error_exit;
		}
		wait_cancel(tcb);

		/* Make the other task at rendezvous end wait state */
		tcb->wspec = &wspec_rdv;
		tcb->wid = 0;
		tcb->winfo.rdv.rdvno = rdvno;
		tcb->winfo.rdv.msg = tcb->winfo.cal.msg;
		tcb->winfo.rdv.maxrmsz = porcb->maxrmsz;
		tcb->winfo.rdv.p_rmsgsz = tcb->winfo.cal.p_rmsgsz;
		timer_insert(&tcb->wtmeb, TMO_FEVR,
					(CBACK)wait_release_tmout, tcb);
		QueInit(&tcb->tskque);

		goto error_exit;
	}

	/* Check rendezvous accept wait disable */
	if ( is_diswai((GCB*)porcb, ctxtsk, TTW_ACP) ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	ercd = E_TMOUT;
	if ( tmout != TMO_POL ) {
		/* Ready for rendezvous accept wait */
		ctxtsk->wspec = &wspec_acp;
		ctxtsk->wid = porid;
		ctxtsk->wercd = &ercd;
		ctxtsk->winfo.acp.acpptn = acpptn;
		ctxtsk->winfo.acp.msg = msg;
		ctxtsk->winfo.acp.p_rdvno = p_rdvno;
		ctxtsk->winfo.acp.p_cmsgsz = &cmsgsz;
		make_wait(tmout, porcb->poratr);
		QueInsert(&ctxtsk->tskque, &porcb->accept_queue);
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ( ercd < E_OK )? ercd: cmsgsz;
}

/*
 * Forward Rendezvous to Other Port
 */
SYSCALL ER _tk_fwd_por( ID porid, UINT calptn, RNO rdvno, void *msg, INT cmsgsz )
{
	PORCB	*porcb;
	TCB	*caltcb, *tcb;
	QUEUE	*queue;
	RNO	new_rdvno;
	ER	ercd = E_OK;

	CHECK_PORID(porid);
	CHECK_PAR(calptn != 0);
	CHECK_RDVNO(rdvno);
	CHECK_PAR(cmsgsz >= 0);
	CHECK_INTSK();

	porcb = get_porcb(porid);
	caltcb = get_tcb(get_tskid_rdvno(rdvno));

	BEGIN_CRITICAL_SECTION;
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
#if CHK_PAR
	if ( cmsgsz > porcb->maxcmsz ) {
		ercd = E_PAR;
		goto error_exit;
	}
#endif
	if ( (caltcb->state & TS_WAIT) == 0
	  || caltcb->wspec != &wspec_rdv
	  || rdvno != caltcb->winfo.rdv.rdvno ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	if ( porcb->maxrmsz > caltcb->winfo.rdv.maxrmsz ) {
		ercd = E_OBJ;
		goto error_exit;
	}
#if CHK_PAR
	if ( cmsgsz > caltcb->winfo.rdv.maxrmsz ) {
		ercd = E_PAR;
		goto error_exit;
	}
#endif

	/* Search accept wait task */
	queue = porcb->accept_queue.next;
	while ( queue != &porcb->accept_queue ) {
		tcb = (TCB*)queue;
		queue = queue->next;
		if ( (calptn & tcb->winfo.acp.acpptn) == 0 ) {
			continue;
		}

		/* Check rendezvous accept wait disable */
		if ( is_diswai((GCB*)porcb, caltcb, TTW_CAL) ) {
			wait_release_ng(caltcb, E_DISWAI);
			ercd = E_DISWAI;
			goto error_exit;
		}

		/* Send message */
		new_rdvno = gen_rdvno(caltcb);
		if ( cmsgsz > 0 ) {
			memcpy(tcb->winfo.acp.msg, msg, (UINT)cmsgsz);
		}
		*tcb->winfo.acp.p_rdvno = new_rdvno;
		*tcb->winfo.acp.p_cmsgsz = cmsgsz;
		wait_release_ok(tcb);

		/* Check rendezvous end wait disable */
		if ( is_diswai((GCB*)porcb, caltcb, TTW_RDV) ) {
			wait_release_ng(caltcb, E_DISWAI);
			ercd = E_DISWAI;
			goto error_exit;
		}

		/* Change rendezvous end wait of the other task */
		caltcb->winfo.rdv.rdvno = new_rdvno;
		caltcb->winfo.rdv.msg = caltcb->winfo.cal.msg;
		caltcb->winfo.rdv.maxrmsz = porcb->maxrmsz;
		caltcb->winfo.rdv.p_rmsgsz = caltcb->winfo.cal.p_rmsgsz;
		caltcb->nodiswai = ( (porcb->poratr & TA_NODISWAI) != 0 )? TRUE: FALSE;
		goto error_exit;
	}

	/* Check rendezvous accept wait disable */
	if ( is_diswai((GCB*)porcb, caltcb, TTW_CAL) ) {
		wait_release_ng(caltcb, E_DISWAI);
		ercd = E_DISWAI;
		goto error_exit;
	}

	/* Change the other task to rendezvous call wait */
	caltcb->wspec = ( (porcb->poratr & TA_TPRI) != 0 )?
				&wspec_cal_tpri: &wspec_cal_tfifo;
	caltcb->wid = porid;
	caltcb->winfo.cal.calptn = calptn;
	caltcb->winfo.cal.msg = caltcb->winfo.rdv.msg;
	caltcb->winfo.cal.cmsgsz = cmsgsz;
	caltcb->winfo.cal.p_rmsgsz = caltcb->winfo.rdv.p_rmsgsz;
	caltcb->nodiswai = ( (porcb->poratr & TA_NODISWAI) != 0 )? TRUE: FALSE;
	timer_insert(&caltcb->wtmeb, TMO_FEVR,
			(CBACK)wait_release_tmout, caltcb);
	if ( (porcb->poratr & TA_TPRI) != 0 ) {
		queue_insert_tpri(caltcb, &porcb->call_queue);
	} else {
		QueInsert(&caltcb->tskque, &porcb->call_queue);
	}

	if ( cmsgsz > 0 ) {
		memcpy(caltcb->winfo.cal.msg, msg, (UINT)cmsgsz);
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Reply rendezvous
 */
SYSCALL ER _tk_rpl_rdv( RNO rdvno, void *msg, INT rmsgsz )
{
	TCB	*caltcb;
	ER	ercd = E_OK;

	CHECK_RDVNO(rdvno);
	CHECK_PAR(rmsgsz >= 0);
	CHECK_INTSK();

	caltcb = get_tcb(get_tskid_rdvno(rdvno));

	BEGIN_CRITICAL_SECTION;
	if ( (caltcb->state & TS_WAIT) == 0
	  || caltcb->wspec != &wspec_rdv
	  || rdvno != caltcb->winfo.rdv.rdvno ) {
		ercd = E_OBJ;
		goto error_exit;
	}
#if CHK_PAR
	if ( rmsgsz > caltcb->winfo.rdv.maxrmsz ) {
		ercd = E_PAR;
		goto error_exit;
	}
#endif

	/* Send message */
	if ( rmsgsz > 0 ) {
		memcpy(caltcb->winfo.rdv.msg, msg, (UINT)rmsgsz);
	}
	*caltcb->winfo.rdv.p_rmsgsz = rmsgsz;
	wait_release_ok(caltcb);

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Refer rendezvous port
 */
SYSCALL ER _tk_ref_por( ID porid, T_RPOR *pk_rpor )
{
	PORCB	*porcb;
	ER	ercd = E_OK;

	CHECK_PORID(porid);

	porcb = get_porcb(porid);

	BEGIN_CRITICAL_SECTION;
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rpor->exinf = porcb->exinf;
		pk_rpor->wtsk = wait_tskid(&porcb->call_queue);
		pk_rpor->atsk = wait_tskid(&porcb->accept_queue);
		pk_rpor->maxcmsz = porcb->maxcmsz;
		pk_rpor->maxrmsz = porcb->maxrmsz;
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
EXPORT ER rendezvous_getname(ID id, UB **name)
{
	PORCB	*porcb;
	ER	ercd = E_OK;

	CHECK_PORID(id);

	BEGIN_DISABLE_INTERRUPT;
	porcb = get_porcb(id);
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (porcb->poratr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = porcb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer rendezvous port usage state
 */
SYSCALL INT _td_lst_por( ID list[], INT nent )
{
	PORCB	*porcb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = porcb_table + NUM_PORID;
	for ( porcb = porcb_table; porcb < end; porcb++ ) {
		if ( porcb->porid == 0 ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = porcb->porid;
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer rendezvous port
 */
SYSCALL ER _td_ref_por( ID porid, TD_RPOR *pk_rpor )
{
	PORCB	*porcb;
	ER	ercd = E_OK;

	CHECK_PORID(porid);

	porcb = get_porcb(porid);

	BEGIN_DISABLE_INTERRUPT;
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rpor->exinf = porcb->exinf;
		pk_rpor->wtsk = wait_tskid(&porcb->call_queue);
		pk_rpor->atsk = wait_tskid(&porcb->accept_queue);
		pk_rpor->maxcmsz = porcb->maxcmsz;
		pk_rpor->maxrmsz = porcb->maxrmsz;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Refer rendezvous call wait queue
 */
SYSCALL INT _td_cal_que( ID porid, ID list[], INT nent )
{
	PORCB	*porcb;
	QUEUE	*q;
	ER	ercd = E_OK;

	CHECK_PORID(porid);

	porcb = get_porcb(porid);

	BEGIN_DISABLE_INTERRUPT;
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
	} else {
		INT n = 0;
		for ( q = porcb->call_queue.next; q != &porcb->call_queue; q = q->next ) {
			if ( n++ < nent ) {
				*list++ = ((TCB*)q)->tskid;
			}
		}
		ercd = n;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Refer rendezvous accept wait queue
 */
SYSCALL INT _td_acp_que( ID porid, ID list[], INT nent )
{
	PORCB	*porcb;
	QUEUE	*q;
	ER	ercd = E_OK;

	CHECK_PORID(porid);

	porcb = get_porcb(porid);

	BEGIN_DISABLE_INTERRUPT;
	if ( porcb->porid == 0 ) {
		ercd = E_NOEXS;
	} else {
		INT n = 0;
		for ( q = porcb->accept_queue.next; q != &porcb->accept_queue; q = q->next ) {
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
#endif /* NUM_PORID */
