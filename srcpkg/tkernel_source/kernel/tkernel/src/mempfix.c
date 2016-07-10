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
 *	mempfix.c (T-Kernel/OS)
 *	Fixed Size Memory Pool
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include <tk/wait.h>
#include "check.h"
#include <sys/rominfo.h>

#ifdef NUM_MPFID

EXPORT ID	max_mpfid;	/* Maximum fixed size memory pool ID */

/*
 * Fixed size memory pool control block
 */
typedef struct free_list {
	struct free_list *next;
} FREEL;

typedef struct fix_memorypool_control_block {
	QUEUE	wait_queue;	/* Memory pool wait queue */
	ID	mpfid;		/* Fixed size memory pool ID */
	void	*exinf;		/* Extended information */
	ATR	mpfatr;		/* Memory pool attribute */
	INT	mpfcnt;		/* Number of blocks in whole memory pool */
	INT	blfsz;		/* Fixed size memory block size */
	INT	mpfsz;		/* Whole memory pool size */
	INT	frbcnt;		/* Number of blocks in free area */
	void	*mempool;	/* Top address of memory pool */
	void	*unused;	/* Top address of unused area */
	FREEL	*freelist;	/* Free block list */
	OBJLOCK	lock;		/* Lock for object exclusive access */
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
} MPFCB;

LOCAL MPFCB	*mpfcb_table;	/* Fixed size memory pool control block */
LOCAL QUEUE	free_mpfcb;	/* FreeQue */

#define get_mpfcb(id)	( &mpfcb_table[INDEX_MPF(id)] )


/*
 * Initialization of fixed size memory pool control block
 */
EXPORT ER fix_memorypool_initialize( void )
{
	MPFCB	*mpfcb, *end;
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXMPFID, &max_mpfid, 1);
	if ( n < 1 || NUM_MPFID < 1 ) {
		return E_SYS;
	}

	/* Create fixed size memory pool control block */
	mpfcb_table = Imalloc((UINT)NUM_MPFID * sizeof(MPFCB));
	if ( mpfcb_table == NULL ) {
		return E_NOMEM;
	}

	/* Register all control blocks onto FreeQue */
	QueInit(&free_mpfcb);
	end = mpfcb_table + NUM_MPFID;
	for ( mpfcb = mpfcb_table; mpfcb < end; mpfcb++ ) {
		mpfcb->mpfid = 0;
		InitOBJLOCK(&mpfcb->lock);
		QueInsert(&mpfcb->wait_queue, &free_mpfcb);
	}

	return E_OK;
}


#define MINSIZE		( sizeof(FREEL) )
#define MINSZ(sz)	( ((sz) + (MINSIZE-1)) & ~(MINSIZE-1) )

/*
 * Return end address in memory pool area
 */
Inline void* mempool_end( MPFCB *mpfcb )
{
	return (VB*)mpfcb->mempool + mpfcb->mpfsz;
}


/*
 * Processing if the priority of wait task changes
 */
LOCAL void mpf_chg_pri( TCB *tcb, INT oldpri )
{
	MPFCB	*mpfcb;

	mpfcb = get_mpfcb(tcb->wid);
	gcb_change_priority((GCB*)mpfcb, tcb);
}

/*
 * Definition of fixed size memory pool wait specification
 */
LOCAL CONST WSPEC wspec_mpf_tfifo = { TTW_MPF, NULL, NULL };
LOCAL CONST WSPEC wspec_mpf_tpri  = { TTW_MPF, mpf_chg_pri, NULL };


/*
 * Create fixed size memory pool
 */
SYSCALL ID _tk_cre_mpf( CONST T_CMPF *pk_cmpf )
{
#if CHK_RSATR
	const ATR VALID_MPFATR = {
		 TA_TPRI
		|TA_RNG3
		|TA_NORESIDENT
		|TA_NODISWAI
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	MPFCB	*mpfcb;
	ID	mpfid;
	INT	blfsz, mpfsz;
	void	*mempool;

	CHECK_RSATR(pk_cmpf->mpfatr, VALID_MPFATR);
	CHECK_PAR(pk_cmpf->mpfcnt > 0);
	CHECK_PAR(pk_cmpf->blfsz > 0);
	CHECK_DISPATCH();

	blfsz = (INT)MINSZ(pk_cmpf->blfsz);
	mpfsz = blfsz * pk_cmpf->mpfcnt;

	/* Allocate memory for memory pool */
	mempool = IAmalloc((UINT)mpfsz, pk_cmpf->mpfatr);
	if ( mempool == NULL ) {
		return E_NOMEM;
	}

	/* Get control block from FreeQue */
	//DISABLE_INTERRUPT;
	BEGIN_DISABLE_INTERRUPT;
	mpfcb = (MPFCB*)QueRemoveNext(&free_mpfcb);
	//ENABLE_INTERRUPT;
	END_DISABLE_INTERRUPT;
	if ( mpfcb == NULL ) {
		IAfree(mempool, pk_cmpf->mpfatr);
		return E_LIMIT;
	}

	LockOBJ(&mpfcb->lock);
	mpfid = ID_MPF(mpfcb - mpfcb_table);

	/* Initialize control block */
	QueInit(&mpfcb->wait_queue);
	mpfcb->exinf    = pk_cmpf->exinf;
	mpfcb->mpfatr   = pk_cmpf->mpfatr;
	mpfcb->mpfcnt   = mpfcb->frbcnt = pk_cmpf->mpfcnt;
	mpfcb->blfsz    = blfsz;
	mpfcb->mpfsz    = mpfsz;
	mpfcb->unused   = mpfcb->mempool = mempool;
	mpfcb->freelist = NULL;
#if USE_OBJECT_NAME
	if ( (pk_cmpf->mpfatr & TA_DSNAME) != 0 ) {
		strncpy((char*)mpfcb->name, (char*)pk_cmpf->dsname, OBJECT_NAME_LENGTH);
	}
#endif

	mpfcb->mpfid    = mpfid;  /* Set ID after completion */
	UnlockOBJ(&mpfcb->lock);

	return mpfid;
}

/*
 * Delete fixed size memory pool
 */
SYSCALL ER _tk_del_mpf( ID mpfid )
{
	MPFCB	*mpfcb;
	void	*mempool = NULL;
	ATR	memattr = 0;
	ER	ercd = E_OK;

	CHECK_MPFID(mpfid);
	CHECK_DISPATCH();

	mpfcb = get_mpfcb(mpfid);

	LockOBJ(&mpfcb->lock);
	if ( mpfcb->mpfid == 0 ) {
		ercd = E_NOEXS;
	} else {
		//DISABLE_INTERRUPT;
		BEGIN_DISABLE_INTERRUPT;
		mempool = mpfcb->mempool;
		memattr = mpfcb->mpfatr;

		/* Release wait state of task (E_DLT) */
		wait_delete(&mpfcb->wait_queue);

		/* Return to FreeQue */
		QueInsert(&mpfcb->wait_queue, &free_mpfcb);
		mpfcb->mpfid = 0;
		//ENABLE_INTERRUPT;
		END_DISABLE_INTERRUPT;
	}
	UnlockOBJ(&mpfcb->lock);

	if ( mempool != NULL ) {
		IAfree(mempool, memattr);
	}

	return ercd;
}

/*
 * Get fixed size memory block
 */
SYSCALL ER _tk_get_mpf( ID mpfid, void **p_blf, TMO tmout )
{
	return _tk_get_mpf_u(mpfid, p_blf, to_usec_tmo(tmout));
}

SYSCALL ER _tk_get_mpf_u( ID mpfid, void **p_blf, TMO_U tmout )
{
	MPFCB	*mpfcb;
	FREEL	*free;
	ER	ercd = E_OK;

	CHECK_MPFID(mpfid);
	CHECK_TMOUT(tmout);
	CHECK_DISPATCH();

	mpfcb = get_mpfcb(mpfid);

	LockOBJ(&mpfcb->lock);
	if ( mpfcb->mpfid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}

	/* Check wait disable */
	if ( is_diswai((GCB*)mpfcb, ctxtsk, TTW_MPF) ) {
		ercd = E_DISWAI;
		goto error_exit;
	}

	/* If there is no space, ready for wait */
	if ( mpfcb->frbcnt <= 0 ) {
		goto wait_mpf;
	}

	/* Get memory block */
	if ( mpfcb->freelist != NULL ) {
		free = mpfcb->freelist;
		mpfcb->freelist = free->next;
		*p_blf = free;
	} else {
		*p_blf = mpfcb->unused;
		mpfcb->unused = (VB*)mpfcb->unused + mpfcb->blfsz;
	}
	mpfcb->frbcnt--;

    error_exit:
	UnlockOBJ(&mpfcb->lock);

	return ercd;

    wait_mpf:
	/* Ready for wait */
	BEGIN_CRITICAL_SECTION;
	ctxtsk->wspec = ( (mpfcb->mpfatr & TA_TPRI) != 0 )?
				&wspec_mpf_tpri: &wspec_mpf_tfifo;
	ctxtsk->wercd = &ercd;
	ctxtsk->winfo.mpf.p_blf = p_blf;
	gcb_make_wait_with_diswai((GCB*)mpfcb, tmout);

	UnlockOBJ(&mpfcb->lock);
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Return fixed size memory block
 */
SYSCALL ER _tk_rel_mpf( ID mpfid, void *blf )
{
	MPFCB	*mpfcb;
	TCB	*tcb;
	FREEL	*free;
	ER	ercd = E_OK;

	CHECK_MPFID(mpfid);
	CHECK_DISPATCH();

	mpfcb = get_mpfcb(mpfid);

	LockOBJ(&mpfcb->lock);
	if ( mpfcb->mpfid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
#if CHK_PAR
	if ( blf < mpfcb->mempool || blf >= mempool_end(mpfcb) || (((VB*)blf - (VB*)mpfcb->mempool) % mpfcb->blfsz) != 0 ) {
		ercd = E_PAR;
		goto error_exit;
	}
#endif

	//DISABLE_INTERRUPT;
	BEGIN_DISABLE_INTERRUPT;
	if ( !isQueEmpty(&mpfcb->wait_queue) ) {
		/* Send memory block to waiting task,
		   and then release the task */
		tcb = (TCB*)mpfcb->wait_queue.next;
		*tcb->winfo.mpf.p_blf = blf;
		wait_release_ok(tcb);
		//ENABLE_INTERRUPT;
	} else {
		//ENABLE_INTERRUPT;
		/* Free memory block */
		free = (FREEL*)blf;
		free->next = mpfcb->freelist;
		mpfcb->freelist = free;
		mpfcb->frbcnt++;
	}
	END_DISABLE_INTERRUPT;

    error_exit:
	UnlockOBJ(&mpfcb->lock);

	return ercd;
}

/*
 * Check fixed size pool state
 */
SYSCALL ER _tk_ref_mpf( ID mpfid, T_RMPF *pk_rmpf )
{
	MPFCB	*mpfcb;
	ER	ercd = E_OK;

	CHECK_MPFID(mpfid);
	CHECK_DISPATCH();

	mpfcb = get_mpfcb(mpfid);

	LockOBJ(&mpfcb->lock);
	if ( mpfcb->mpfid == 0 ) {
		ercd = E_NOEXS;
	} else {
		//DISABLE_INTERRUPT;
		BEGIN_DISABLE_INTERRUPT;
		pk_rmpf->wtsk = wait_tskid(&mpfcb->wait_queue);
		//ENABLE_INTERRUPT;
		END_DISABLE_INTERRUPT;
		pk_rmpf->exinf = mpfcb->exinf;
		pk_rmpf->frbcnt = mpfcb->frbcnt;
	}
	UnlockOBJ(&mpfcb->lock);

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
EXPORT ER fix_memorypool_getname(ID id, UB **name)
{
	MPFCB	*mpfcb;
	ER	ercd = E_OK;

	CHECK_MPFID(id);

	BEGIN_DISABLE_INTERRUPT;
	mpfcb = get_mpfcb(id);
	if ( mpfcb->mpfid == 0 ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (mpfcb->mpfatr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = mpfcb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer fixed size memory pool usage state
 */
SYSCALL INT _td_lst_mpf( ID list[], INT nent )
{
	MPFCB	*mpfcb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = mpfcb_table + NUM_MPFID;
	for ( mpfcb = mpfcb_table; mpfcb < end; mpfcb++ ) {
		if ( mpfcb->mpfid == 0 ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = ID_MPF(mpfcb - mpfcb_table);
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer fixed size memory pool state
 */
SYSCALL ER _td_ref_mpf( ID mpfid, TD_RMPF *pk_rmpf )
{
	MPFCB	*mpfcb;
	ER	ercd = E_OK;

	CHECK_MPFID(mpfid);

	mpfcb = get_mpfcb(mpfid);

	BEGIN_DISABLE_INTERRUPT;
	if ( mpfcb->mpfid == 0 ) {
		ercd = E_NOEXS;
	} else if ( isLockedOBJ(&mpfcb->lock) ) {
		ercd = E_CTX;
	} else {
		pk_rmpf->wtsk = wait_tskid(&mpfcb->wait_queue);
		pk_rmpf->exinf = mpfcb->exinf;
		pk_rmpf->frbcnt = mpfcb->frbcnt;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Refer fixed size memory wait queue
 */
SYSCALL INT _td_mpf_que( ID mpfid, ID list[], INT nent )
{
	MPFCB	*mpfcb;
	QUEUE	*q;
	ER	ercd = E_OK;

	CHECK_MPFID(mpfid);

	mpfcb = get_mpfcb(mpfid);

	BEGIN_DISABLE_INTERRUPT;
	if ( mpfcb->mpfid == 0 ) {
		ercd = E_NOEXS;
	} else {
		INT n = 0;
		for ( q = mpfcb->wait_queue.next; q != &mpfcb->wait_queue; q = q->next ) {
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
#endif /* NUM_MPFID */
