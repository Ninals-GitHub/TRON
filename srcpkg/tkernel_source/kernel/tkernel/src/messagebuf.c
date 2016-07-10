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
 *	messagebuf.c (T-Kernel/OS)
 *	Message Buffer
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include <tk/wait.h>
#include "check.h"
#include <sys/rominfo.h>

#ifdef NUM_MBFID

EXPORT ID	max_mbfid;	/* Maximum message buffer ID */

/*
 * Message buffer control block
 *
 *	Because Receive wait task (TTW_MBF) and Send wait task (TTW_SMBF)
 *	do not co-exist for one message buffer, the wait queue may be
 *	allowed to share.
 *	However, when the size of message buffer is 0, it is difficult
 *	to judge the wait queue if it is for receive or send,
 *	therefore do not use this method.
 */
typedef struct messagebuffer_control_block {
	QUEUE	send_queue;	/* Message buffer send wait queue */
	ID	mbfid;		/* message buffer ID */
	void	*exinf;		/* Extended information */
	ATR	mbfatr;		/* Message buffer attribute */
	QUEUE	recv_queue;	/* Message buffer receive wait queue */
	INT	bufsz;		/* Message buffer size */
	INT	maxmsz;		/* Maximum length of message */
	INT	frbufsz;	/* Free buffer size */
	INT	head;		/* First message store address */
	INT	tail;		/* Next to the last message store address */
	VB	*buffer;	/* Message buffer address */
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
} MBFCB;

LOCAL MBFCB	*mbfcb_table;	/* Message buffer control block */
LOCAL QUEUE	free_mbfcb;	/* FreeQue */

#define get_mbfcb(id)	( &mbfcb_table[INDEX_MBF(id)] )


/*
 * Initialization of message buffer control block
 */
EXPORT ER messagebuffer_initialize( void )
{
	MBFCB	*mbfcb, *end;
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXMBFID, &max_mbfid, 1);
	if ( n < 1 || NUM_MBFID < 1 ) {
		return E_SYS;
	}

	/* Create message buffer control block */
	mbfcb_table = Imalloc((UINT)NUM_MBFID * sizeof(MBFCB));
	if ( mbfcb_table == NULL ) {
		return E_NOMEM;
	}

	/* Register all control blocks onto FeeQue */
	QueInit(&free_mbfcb);
	end = mbfcb_table + NUM_MBFID;
	for ( mbfcb = mbfcb_table; mbfcb < end; mbfcb++ ) {
		mbfcb->mbfid = 0;
		QueInsert(&mbfcb->send_queue, &free_mbfcb);
	}

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * Message header format
 */
typedef INT		HEADER;
#define HEADERSZ	(sizeof(HEADER))

#define ROUNDSIZE	(sizeof(HEADER))
#define ROUNDSZ(sz)	(((UINT)(sz) + (ROUNDSIZE-1)) & ~(ROUNDSIZE-1))

/*
 * Check message buffer free space
 *	If 'msgsz' message is able to be stored, return TRUE.
 */
Inline BOOL mbf_free( MBFCB *mbfcb, INT msgsz )
{
	return ( HEADERSZ + (UINT)msgsz <= (UINT)mbfcb->frbufsz );
}

/*
 * If message buffer is empty, return TRUE.
 */
Inline BOOL mbf_empty( MBFCB *mbfcb )
{
	return ( mbfcb->frbufsz == mbfcb->bufsz );
}

/*
 * Store the message to message buffer.
 */
LOCAL void msg_to_mbf( MBFCB *mbfcb, CONST void *msg, INT msgsz )
{
	INT	tail = mbfcb->tail;
	VB	*buffer = mbfcb->buffer;
	INT	remsz;

	mbfcb->frbufsz -= (INT)(HEADERSZ + ROUNDSZ(msgsz));

	*(HEADER*)&buffer[tail] = msgsz;
	tail += HEADERSZ;
	if ( tail >= mbfcb->bufsz ) {
		tail = 0;
	}

	if ( (remsz = mbfcb->bufsz - tail) < msgsz ) {
		memcpy(&buffer[tail], msg, (UINT)remsz);
		msg = (VB*)msg + remsz;
		msgsz -= remsz;
		tail = 0;
	}
	memcpy(&buffer[tail], msg, (UINT)msgsz);
	tail += (INT)ROUNDSZ(msgsz);
	if ( tail >= mbfcb->bufsz ) {
		tail = 0;
	}

	mbfcb->tail = tail;
}

/*
 * Get a message from message buffer.
 * Return the message size.
 */
LOCAL INT mbf_to_msg( MBFCB *mbfcb, void *msg )
{
	INT	head = mbfcb->head;
	VB	*buffer = mbfcb->buffer;
	INT	msgsz, actsz;
	INT	remsz;

	actsz = msgsz = *(HEADER*)&buffer[head];
	mbfcb->frbufsz += (INT)(HEADERSZ + ROUNDSZ(msgsz));

	head += (INT)HEADERSZ;
	if ( head >= mbfcb->bufsz ) {
		head = 0;
	}

	if ( (remsz = mbfcb->bufsz - head) < msgsz ) {
		memcpy(msg, &buffer[head], (UINT)remsz);
		msg = (VB*)msg + remsz;
		msgsz -= remsz;
		head = 0;
	}
	memcpy(msg, &buffer[head], (UINT)msgsz);
	head += (INT)ROUNDSZ(msgsz);
	if ( head >= mbfcb->bufsz ) {
		head = 0;
	}

	mbfcb->head = head;

	return actsz;
}

/* ------------------------------------------------------------------------ */

/*
 * Accept message and release wait task,
 * as long as there are free message area.
 */
LOCAL void mbf_wakeup( MBFCB *mbfcb )
{
	TCB	*top;
	INT	msgsz;

	while ( !isQueEmpty(&mbfcb->send_queue) ) {
		top = (TCB*)mbfcb->send_queue.next;
		msgsz = top->winfo.smbf.msgsz;
		if ( !mbf_free(mbfcb, msgsz) ) {
			break;
		}

		/* Store a message from waiting task and release it */
		msg_to_mbf(mbfcb, top->winfo.smbf.msg, msgsz);
		wait_release_ok(top);
	}
}

/*
 * Processing if the priority of wait task changes
 */
LOCAL void mbf_chg_pri( TCB *tcb, INT oldpri )
{
	MBFCB	*mbfcb;

	mbfcb = get_mbfcb(tcb->wid);
	if ( oldpri >= 0 ) {
		/* Reorder wait queue */
		gcb_change_priority((GCB*)mbfcb, tcb);
	}

	/* If the new head task in a send wait queue is able to sent,
	   send its message */
	mbf_wakeup(mbfcb);
}

/*
 * Processing if the wait task is released
 */
LOCAL void mbf_rel_wai( TCB *tcb )
{
	mbf_chg_pri(tcb, -1);
}

/*
 * Definition of message buffer wait specification
 */
LOCAL CONST WSPEC wspec_smbf_tfifo = { TTW_SMBF, NULL,	mbf_rel_wai };
LOCAL CONST WSPEC wspec_smbf_tpri  = { TTW_SMBF, mbf_chg_pri,	mbf_rel_wai };
LOCAL CONST WSPEC wspec_rmbf       = { TTW_RMBF, NULL,	NULL	    };


/*
 * Create message buffer
 */
SYSCALL ID _tk_cre_mbf( CONST T_CMBF *pk_cmbf )
{
#if CHK_RSATR
	const ATR VALID_MBFATR = {
		 TA_TPRI
		|TA_NODISWAI
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	MBFCB	*mbfcb;
	ID	mbfid;
	INT	bufsz;
	VB	*msgbuf;
	ER	ercd;

	CHECK_RSATR(pk_cmbf->mbfatr, VALID_MBFATR);
	CHECK_PAR(pk_cmbf->bufsz >= 0);
	CHECK_PAR(pk_cmbf->maxmsz > 0);
	bufsz = (INT)ROUNDSZ(pk_cmbf->bufsz);

	if ( bufsz > 0 ) {
		msgbuf = Imalloc((UINT)bufsz);
		if ( msgbuf == NULL ) {
			return E_NOMEM;
		}
	} else {
		msgbuf = NULL;
	}

	BEGIN_CRITICAL_SECTION;
	/* Get control block from FreeQue */
	mbfcb = (MBFCB*)QueRemoveNext(&free_mbfcb);
	if ( mbfcb == NULL ) {
		ercd = E_LIMIT;
	} else {
		mbfid = ID_MBF(mbfcb - mbfcb_table);

		/* Initialize control block */
		QueInit(&mbfcb->send_queue);
		mbfcb->mbfid = mbfid;
		mbfcb->exinf = pk_cmbf->exinf;
		mbfcb->mbfatr = pk_cmbf->mbfatr;
		QueInit(&mbfcb->recv_queue);
		mbfcb->buffer = msgbuf;
		mbfcb->bufsz = mbfcb->frbufsz = bufsz;
		mbfcb->maxmsz = pk_cmbf->maxmsz;
		mbfcb->head = mbfcb->tail = 0;
#if USE_OBJECT_NAME
		if ( (pk_cmbf->mbfatr & TA_DSNAME) != 0 ) {
			strncpy((char*)mbfcb->name, (char*)pk_cmbf->dsname,
				OBJECT_NAME_LENGTH);
		}
#endif
		ercd = mbfid;
	}
	END_CRITICAL_SECTION;

	if ( ercd < E_OK && msgbuf != NULL ) {
		Ifree(msgbuf);
	}

	return ercd;
}

/*
 * Delete message buffer
 */
SYSCALL ER _tk_del_mbf( ID mbfid )
{
	MBFCB	*mbfcb;
	VB	*msgbuf = NULL;
	ER	ercd = E_OK;

	CHECK_MBFID(mbfid);

	mbfcb = get_mbfcb(mbfid);

	BEGIN_CRITICAL_SECTION;
	if ( mbfcb->mbfid == 0 ) {
		ercd = E_NOEXS;
	} else {
		msgbuf = mbfcb->buffer;

		/* Release wait state of task (E_DLT) */
		wait_delete(&mbfcb->recv_queue);
		wait_delete(&mbfcb->send_queue);

		/* Return to FreeQue */
		QueInsert(&mbfcb->send_queue, &free_mbfcb);
		mbfcb->mbfid = 0;
	}
	END_CRITICAL_SECTION;

	if ( msgbuf != NULL ) {
		Ifree(msgbuf);
	}

	return ercd;
}

/*
 * Send to message buffer
 */
SYSCALL ER _tk_snd_mbf( ID mbfid, CONST void *msg, INT msgsz, TMO tmout )
{
	return _tk_snd_mbf_u(mbfid, msg, msgsz, to_usec_tmo(tmout));
}

SYSCALL ER _tk_snd_mbf_u( ID mbfid, CONST void *msg, INT msgsz, TMO_U tmout )
{
	MBFCB	*mbfcb;
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_MBFID(mbfid);
	CHECK_PAR(msgsz > 0);
	CHECK_TMOUT(tmout);
	CHECK_DISPATCH_POL(tmout);

	mbfcb = get_mbfcb(mbfid);

	BEGIN_CRITICAL_SECTION;
	if ( mbfcb->mbfid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
#if CHK_PAR
	if ( msgsz > mbfcb->maxmsz ) {
		ercd = E_PAR;
		goto error_exit;
	}
#endif

	/* Check send wait disable */
	if ( !in_indp() && is_diswai((GCB*)mbfcb, ctxtsk, TTW_SMBF) ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	if ( !isQueEmpty(&mbfcb->recv_queue) ) {
		/* Send directly to the receive wait task */
		tcb = (TCB*)mbfcb->recv_queue.next;
		memcpy(tcb->winfo.rmbf.msg, msg, (UINT)msgsz);
		*tcb->winfo.rmbf.p_msgsz = msgsz;
		wait_release_ok(tcb);

	} else if ( (in_indp() || gcb_top_of_wait_queue((GCB*)mbfcb, ctxtsk) == ctxtsk)
		  &&(mbf_free(mbfcb, msgsz)) ) {
		/* Store the message to message buffer */
		msg_to_mbf(mbfcb, msg, msgsz);

	} else {
		ercd = E_TMOUT;
		if ( tmout != TMO_POL ) {
			/* Ready for send wait */
			ctxtsk->wspec = ( (mbfcb->mbfatr & TA_TPRI) != 0 )?
					&wspec_smbf_tpri: &wspec_smbf_tfifo;
			ctxtsk->wercd = &ercd;
			ctxtsk->winfo.smbf.msg = msg;
			ctxtsk->winfo.smbf.msgsz = msgsz;
			gcb_make_wait_with_diswai((GCB*)mbfcb, tmout);
		}
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Receive from message buffer
 */
SYSCALL INT _tk_rcv_mbf( ID mbfid, void *msg, TMO tmout )
{
	return _tk_rcv_mbf_u(mbfid, msg, to_usec_tmo(tmout));
}

SYSCALL INT _tk_rcv_mbf_u( ID mbfid, void *msg, TMO_U tmout )
{
	MBFCB	*mbfcb;
	TCB	*tcb;
	INT	rcvsz;
	ER	ercd = E_OK;

	CHECK_MBFID(mbfid);
	CHECK_TMOUT(tmout);
	CHECK_DISPATCH();

	mbfcb = get_mbfcb(mbfid);

	BEGIN_CRITICAL_SECTION;
	if (mbfcb->mbfid == 0) {
		ercd = E_NOEXS;
		goto error_exit;
	}

	/* Check receive wait disable */
	if ( is_diswai((GCB*)mbfcb, ctxtsk, TTW_RMBF) ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	if ( !mbf_empty(mbfcb) ) {
		/* Read from message buffer */
		rcvsz = mbf_to_msg(mbfcb, msg);

		/* Accept message from sending task(s) */
		mbf_wakeup(mbfcb);

	} else if ( !isQueEmpty(&mbfcb->send_queue) ) {
		/* Receive directly from send wait task */
		tcb = (TCB*)mbfcb->send_queue.next;
		rcvsz = tcb->winfo.smbf.msgsz;
		memcpy(msg, tcb->winfo.smbf.msg, (UINT)rcvsz);
		wait_release_ok(tcb);
		mbf_wakeup(mbfcb);
	} else {
		ercd = E_TMOUT;
		if ( tmout != TMO_POL ) {
			/* Ready for receive wait */
			ctxtsk->wspec = &wspec_rmbf;
			ctxtsk->wid = mbfid;
			ctxtsk->wercd = &ercd;
			ctxtsk->winfo.rmbf.msg = msg;
			ctxtsk->winfo.rmbf.p_msgsz = &rcvsz;
			make_wait(tmout, mbfcb->mbfatr);
			QueInsert(&ctxtsk->tskque, &mbfcb->recv_queue);
		}
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ( ercd < E_OK )? ercd: rcvsz;
}

/*
 * Refer message buffer state
 */
SYSCALL ER _tk_ref_mbf( ID mbfid, T_RMBF *pk_rmbf )
{
	MBFCB	*mbfcb;
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_MBFID(mbfid);

	mbfcb = get_mbfcb(mbfid);

	BEGIN_CRITICAL_SECTION;
	if ( mbfcb->mbfid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rmbf->exinf = mbfcb->exinf;
		pk_rmbf->wtsk = wait_tskid(&mbfcb->recv_queue);
		pk_rmbf->stsk = wait_tskid(&mbfcb->send_queue);
		if ( !mbf_empty(mbfcb) ) {
			pk_rmbf->msgsz = *(HEADER*)&mbfcb->buffer[mbfcb->head];
		} else {
			if ( !isQueEmpty(&mbfcb->send_queue) ) {
				tcb = (TCB*)mbfcb->send_queue.next;
				pk_rmbf->msgsz = tcb->winfo.smbf.msgsz;
			} else {
				pk_rmbf->msgsz = 0;
			}
		}
		pk_rmbf->frbufsz = mbfcb->frbufsz;
		pk_rmbf->maxmsz = mbfcb->maxmsz;
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
EXPORT ER messagebuffer_getname(ID id, UB **name)
{
	MBFCB	*mbfcb;
	ER	ercd = E_OK;

	CHECK_MBFID(id);

	BEGIN_DISABLE_INTERRUPT;
	mbfcb = get_mbfcb(id);
	if ( mbfcb->mbfid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (mbfcb->mbfatr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = mbfcb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer message buffer usage state
 */
SYSCALL INT _td_lst_mbf( ID list[], INT nent )
{
	MBFCB	*mbfcb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = mbfcb_table + NUM_MBFID;
	for ( mbfcb = mbfcb_table; mbfcb < end; mbfcb++ ) {
		if ( mbfcb->mbfid == 0 ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = mbfcb->mbfid;
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer message buffer state
 */
SYSCALL ER _td_ref_mbf( ID mbfid, TD_RMBF *pk_rmbf )
{
	MBFCB	*mbfcb;
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_MBFID(mbfid);

	mbfcb = get_mbfcb(mbfid);

	BEGIN_DISABLE_INTERRUPT;
	if ( mbfcb->mbfid == 0 ) {
		ercd = E_NOEXS;
	} else {
		pk_rmbf->exinf = mbfcb->exinf;
		pk_rmbf->wtsk = wait_tskid(&mbfcb->recv_queue);
		pk_rmbf->stsk = wait_tskid(&mbfcb->send_queue);
		if ( !mbf_empty(mbfcb) ) {
			pk_rmbf->msgsz = *(HEADER*)&mbfcb->buffer[mbfcb->head];
		} else {
			if ( !isQueEmpty(&mbfcb->send_queue) ) {
				tcb = (TCB*)mbfcb->send_queue.next;
				pk_rmbf->msgsz = tcb->winfo.smbf.msgsz;
			} else {
				pk_rmbf->msgsz = 0;
			}
		}
		pk_rmbf->frbufsz = mbfcb->frbufsz;
		pk_rmbf->maxmsz = mbfcb->maxmsz;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Refer message buffer send wait queue
 */
SYSCALL INT _td_smbf_que( ID mbfid, ID list[], INT nent )
{
	MBFCB	*mbfcb;
	QUEUE	*q;
	ER	ercd = E_OK;

	CHECK_MBFID(mbfid);

	mbfcb = get_mbfcb(mbfid);

	BEGIN_DISABLE_INTERRUPT;
	if ( mbfcb->mbfid == 0 ) {
		ercd = E_NOEXS;
	} else {
		INT n = 0;
		for ( q = mbfcb->send_queue.next; q != &mbfcb->send_queue; q = q->next ) {
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
 * Refer message buffer receive wait queue
 */
SYSCALL INT _td_rmbf_que( ID mbfid, ID list[], INT nent )
{
	MBFCB	*mbfcb;
	QUEUE	*q;
	ER	ercd = E_OK;

	CHECK_MBFID(mbfid);

	mbfcb = get_mbfcb(mbfid);

	BEGIN_DISABLE_INTERRUPT;
	if ( mbfcb->mbfid == 0 ) {
		ercd = E_NOEXS;
	} else {
		INT n = 0;
		for ( q = mbfcb->recv_queue.next; q != &mbfcb->recv_queue; q = q->next ) {
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
#endif /* NUM_MBFID */
