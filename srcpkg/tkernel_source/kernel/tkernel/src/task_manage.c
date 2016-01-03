/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	task_manage.c (T-Kernel/OS)
 *	Task Management Function
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "wait.h"
#include "check.h"
#include <cpu.h>
#include <tm/tmonitor.h>
#include <bk/bprocess.h>
#include <tstdlib/list.h>


IMPORT const T_CTSK c_init_task;

/*
 * Create task
 */
static id = 0;
SYSCALL ID _tk_cre_tsk P1( CONST T_CTSK *pk_ctsk )
{
#if CHK_RSATR
	const ATR VALID_TSKATR = {	/* Valid value of task attribute */
		 TA_HLNG
		|TA_SSTKSZ
		|TA_USERSTACK
		|TA_TASKSPACE
		|TA_RESID
		|TA_RNG3
		|TA_FPU
		|TA_COP0
		|TA_COP1
		|TA_COP2
		|TA_COP3
		|TA_GP
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	TCB	*tcb;
	INT	stksz, sstksz, sysmode, resid;
	void	*stack = NULL, *sstack;
	ER	ercd;
#ifdef _BTRON_
	struct process *current;
#endif

	CHECK_RSATR(pk_ctsk->tskatr, VALID_TSKATR);
	CHECK_PRI(pk_ctsk->itskpri);
	CHECK_NOCOP(pk_ctsk->tskatr);
#if USE_SINGLE_STACK
	CHECK_NOSPT((pk_ctsk->tskatr & TA_USERSTACK) == 0);
#endif
#if CHK_PAR
	if ( (pk_ctsk->tskatr & TA_USERSTACK) != 0 ) {
		CHECK_PAR((pk_ctsk->tskatr & TA_RNG3) != TA_RNG0);
		CHECK_PAR(pk_ctsk->stksz == 0);
	} else {
		CHECK_PAR(pk_ctsk->stksz >= 0);
	}
	if ( (pk_ctsk->tskatr & TA_TASKSPACE) != 0 ) {
		CHECK_PAR(pk_ctsk->lsid >= 0 && pk_ctsk->lsid <= MAX_LSID);
	}
#endif

	if ( (pk_ctsk->tskatr & TA_RESID) != 0 ) {
		CHECK_RESID(pk_ctsk->resid);
		resid = pk_ctsk->resid;
	} else {
		resid = SYS_RESID; /* System resource group */
	}

	if ( (pk_ctsk->tskatr & TA_SSTKSZ) != 0 ) {
		CHECK_PAR(pk_ctsk->sstksz >= MIN_SYS_STACK_SIZE);
		sstksz = pk_ctsk->sstksz;
	} else {
		sstksz = default_sstksz;
	}
	if ( (pk_ctsk->tskatr & TA_RNG3) == TA_RNG0 ) {
		sysmode = 1;
		sstksz += pk_ctsk->stksz;
		stksz = 0;
	} else {
		sysmode = 0;
#if USE_SINGLE_STACK
		sstksz += pk_ctsk->stksz;
		stksz = 0;
#else
		stksz = pk_ctsk->stksz;
#endif
	}

	/* Adjust stack size by 8 bytes */
	sstksz = (sstksz + 7) / 8 * 8;
	stksz  = (stksz  + 7) / 8 * 8;

	/* Allocate system stack area */
#ifdef _STD_X86_
	sstack = IAmalloc((UINT)KERNEL_STACK_SIZE, TA_RNG0);
	sstksz = KERNEL_STACK_SIZE;
#else
	sstack = IAmalloc((UINT)sstksz, TA_RNG0);
#endif
	if ( sstack == NULL ) {
	vd_printf("failed IAmalloc1\n");
		return E_NOMEM;
	}

	if ( stksz > 0 ) {
		/* Allocate user stack area */
		stack = IAmalloc((UINT)stksz, pk_ctsk->tskatr);
		if ( stack == NULL ) {
			IAfree(sstack, TA_RNG0);
			vd_printf("failed IAmalloc2\n");
			return E_NOMEM;
		}
	}

	BEGIN_CRITICAL_SECTION;
	/* Get control block from FreeQue */
	tcb = (TCB*)QueRemoveNext(&free_tcb);
	if ( tcb == NULL ) {
		ercd = E_LIMIT;
		goto error_exit;
	}

	/* Initialize control block */
	tcb->exinf     = pk_ctsk->exinf;
	tcb->tskatr    = pk_ctsk->tskatr;
	tcb->task      = pk_ctsk->task;
	tcb->ipriority = (UB)int_priority(pk_ctsk->itskpri);
	tcb->resid     = resid;
	tcb->stksz     = stksz;
	tcb->sstksz    = sstksz;

#if USE_OBJECT_NAME
	//if ( (pk_ctsk->tskatr & TA_DSNAME) != 0 ) {
		tcb->name[0] = id + 0x30;
		tcb->name[1] = NULL;
		if(pk_ctsk->dsname[0])
		strncpy((char*)tcb->name, (char*)pk_ctsk->dsname, OBJECT_NAME_LENGTH);
	//}
#endif
#if TA_GP
	/* Set global pointer */
	if ( (pk_ctsk->tskatr & TA_GP) != 0 ) {
		gp = pk_ctsk->gp;
	}
	tcb->gp = gp;
#endif

	/* Set stack pointer */
	tcb->isstack = (VB*)sstack + sstksz - RESERVE_SSTACK(tcb->tskatr);
	
	if ( stksz > 0 ) {
#ifdef _STD_X86_
		//tcb->istack = (VB*)stack + stksz - RESERVE_SSTACK(tcb->tskatr);
#else
		tcb->istack = (VB*)stack + stksz;
#endif
	} else {
#ifdef _STD_X86_
		if ((pk_ctsk->tskatr & TA_RNG3) == TA_RNG0) {
			tcb->istack = tcb->isstack;
		} else {
			//tcb->istack = pk_ctsk->stkptr;
		}
#else
		tcb->istack = pk_ctsk->stkptr;
#endif
	}

	/* Set initial value of task operation mode */
	tcb->isysmode = (B)sysmode;
	tcb->sysmode  = (H)sysmode;

	/* Set initial value of task space */
	if ( (pk_ctsk->tskatr & TA_TASKSPACE) != 0 ) {
		tcb->tskctxb.uatb = pk_ctsk->uatb;
		tcb->tskctxb.lsid = pk_ctsk->lsid;
	} else {
		tcb->tskctxb.uatb = NULL;
		tcb->tskctxb.lsid = 0;		/* Task Space */
	}

	/* make it to DORMANT state */
	make_dormant(tcb);

	ercd = tcb->tskid;
	
#ifdef _BTRON_
	if (pk_ctsk != &c_init_task) {
		current = get_current();
		tcb->proc = current;
		init_list(&tcb->task_node);

		add_list_tail(&tcb->task_node, &current->list_tasks);
	}
#endif

    error_exit:
	END_CRITICAL_SECTION;

	if ( ercd < E_OK ) {
		vd_printf("failed create task\n");
		IAfree(sstack, TA_RNG0);
		if ( stksz > 0 ) {
			IAfree(stack, pk_ctsk->tskatr);
		}
	}

	return ercd;
}

/*
 * Task deletion
 *	Call from critical section
 */
LOCAL void _del_tsk( TCB *tcb )
{
	void	*stack;

	/* Free system stack */
	stack = (VB*)tcb->isstack + RESERVE_SSTACK(tcb->tskatr) - tcb->sstksz;
	IAfree(stack, TA_RNG0);

	if ( tcb->stksz > 0 ) {
		/* Free user stack */
#ifdef _STD_X86_
		//stack = (VB*)tcb->istack - tcb->stksz + RESERVE_SSTACK(tcb->tskatr);
#else
		stack = (VB*)tcb->istack - tcb->stksz;
#endif
		IAfree(stack, tcb->tskatr);
	}

	/* Return control block to FreeQue */
	QueInsert(&tcb->tskque, &free_tcb);
	tcb->state = TS_NONEXIST;
}

/*
 * Delete task
 */
SYSCALL ER _tk_del_tsk( ID tskid )
{
	TCB	*tcb;
	TSTAT	state;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	state = (TSTAT)tcb->state;
	if ( state != TS_DORMANT ) {
		ercd = ( state == TS_NONEXIST )? E_NOEXS: E_OBJ;
	} else {
		_del_tsk(tcb);
	}
	
#ifdef _BTRON_
	tcb->proc = NULL;
	
	del_list(&tcb->task_node);
#endif
	
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Start task
 */
SYSCALL ER _tk_sta_tsk( ID tskid, INT stacd )
{
	TCB	*tcb;
	TSTAT	state;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	state = (TSTAT)tcb->state;
	if ( state != TS_DORMANT ) {
		ercd = ( state == TS_NONEXIST )? E_NOEXS: E_OBJ;
	} else {
		setup_stacd(tcb, stacd);
		make_ready(tcb);
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Task finalization
 *	Call from critical section
 */
LOCAL void _ter_tsk( TCB *tcb )
{
	TSTAT	state;

	if ( tcb->svclocked != NULL ) {
		/* Unlock all extended SVC locks */
		AllUnlockSVC(tcb);
	}

	state = (TSTAT)tcb->state;
	if ( state == TS_READY ) {
		make_non_ready(tcb);

	} else if ( (state & TS_WAIT) != 0 ) {
		wait_cancel(tcb);
		if ( tcb->wspec->rel_wai_hook != NULL ) {
			(*tcb->wspec->rel_wai_hook)(tcb);
		}
	}

#ifdef NUM_MTXID
	/* signal mutex */
	signal_all_mutex(tcb);
#endif

#ifdef _BTRON_
	tcb->proc = NULL;
	
	del_list(&tcb->task_node);
#endif

	cleanup_context(tcb);
}

/*
 * End its own task
 */
SYSCALL void _tk_ext_tsk( void )
{
#ifdef DORMANT_STACK_SIZE
	/* To avoid destroying stack used in 'make_dormant',
	   allocate the dummy area on the stack. */
	volatile VB _dummy[DORMANT_STACK_SIZE];
#endif

	/* Check context error */
#if CHK_CTX2
	if ( in_indp() ) {
#if USE_KERNEL_MESSAGE
		tm_putstring((UB*)"tk_ext_tsk was called in the task independent\n");
#endif
		tm_monitor(); /* To monitor */
	}
#endif
#if CHK_CTX1
	if ( in_ddsp() ) {
#if USE_KERNEL_MESSAGE
		tm_putstring((UB*)"tk_ext_tsk was called in the dispatch disabled\n");
#endif
	}
#endif

	DISABLE_INTERRUPT;
	_ter_tsk(ctxtsk);
	make_dormant(ctxtsk);

	force_dispatch();
	/* No return */

#ifdef DORMANT_STACK_SIZE
	/* for WARNING */
	_dummy[0] = 0;
#endif
}

/*
 * End and delete its own task
 */
SYSCALL void _tk_exd_tsk( void )
{
	/* Check context error */
#if CHK_CTX2
	if ( in_indp() ) {
#if USE_KERNEL_MESSAGE
		tm_putstring((UB*)"tk_exd_tsk was called in the task independent\n");
#endif
		tm_monitor(); /* To monitor */
	}
#endif
#if CHK_CTX1
	if ( in_ddsp() ) {
#if USE_KERNEL_MESSAGE
		tm_putstring((UB*)"tk_exd_tsk was called in the dispatch disabled\n");
#endif
	}
#endif

	DISABLE_INTERRUPT;
	_ter_tsk(ctxtsk);
	_del_tsk(ctxtsk);

	force_dispatch();
	/* No return */
}

/*
 * Termination of other task
 */
SYSCALL ER _tk_ter_tsk( ID tskid )
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
	} else if ( tcb->klocked ) {
		/* Normally, it does not become this state.
		 * When the state is page-in wait in the virtual memory
		 * system and when trying to terminate any task,
		 * it becomes this state.
		 */
		ercd = E_OBJ;
	} else {
		_ter_tsk(tcb);
		make_dormant(tcb);
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Change task priority
 */
SYSCALL ER _tk_chg_pri( ID tskid, PRI tskpri )
{
	TCB	*tcb;
	INT	priority;
	ER	ercd;

	CHECK_TSKID_SELF(tskid);
	CHECK_PRI_INI(tskpri);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
		goto error_exit;
	}

	/* Conversion priority to internal expression */
	if ( tskpri == TPRI_INI ) {
		priority = tcb->ipriority;
	} else {
		priority = int_priority(tskpri);
	}

#ifdef NUM_MTXID
	/* Mutex priority change limit */
	ercd = chg_pri_mutex(tcb, priority);
	if ( ercd < E_OK ) {
		goto error_exit;
	}

	tcb->bpriority = (UB)priority;
	priority = ercd;
#else
	tcb->bpriority = priority;
#endif

	/* Change priority */
	change_task_priority(tcb, priority);

	ercd = E_OK;
    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Rotate ready queue
 */
SYSCALL ER _tk_rot_rdq( PRI tskpri )
{
	CHECK_PRI_RUN(tskpri);

	BEGIN_CRITICAL_SECTION;
	if ( tskpri == TPRI_RUN ) {
		if ( in_indp() ) {
			rotate_ready_queue_run();
		} else {
			rotate_ready_queue(ctxtsk->priority);
		}
	} else {
		rotate_ready_queue(int_priority(tskpri));
	}
	END_CRITICAL_SECTION;

	return E_OK;
}

/*
 * Change slice time
 */
SYSCALL ER _tk_chg_slt( ID tskid, RELTIM slicetime )
{
	return _tk_chg_slt_u(tskid, to_usec(slicetime));
}

SYSCALL ER _tk_chg_slt_u( ID tskid, RELTIM_U slicetime )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		tcb->slicetime = slicetime;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Refer task ID at execution
 */
SYSCALL ID _tk_get_tid( void )
{
	return ( ctxtsk == NULL )? 0: ctxtsk->tskid;
}

/*
 * Refer task state
 */
SYSCALL ER _tk_ref_tsk( ID tskid, T_RTSK *pk_rtsk )
{
	T_RTSK_U lrtsk;
	ER	ercd;

	ercd = _tk_ref_tsk_u(tskid, &lrtsk);

	pk_rtsk->exinf	   = lrtsk.exinf;
	pk_rtsk->tskpri	   = lrtsk.tskpri;
	pk_rtsk->tskbpri   = lrtsk.tskbpri;
	pk_rtsk->tskstat   = lrtsk.tskstat;
	pk_rtsk->tskwait   = lrtsk.tskwait;
	pk_rtsk->wid	   = lrtsk.wid;
	pk_rtsk->wupcnt	   = lrtsk.wupcnt;
	pk_rtsk->suscnt	   = lrtsk.suscnt;
	pk_rtsk->slicetime = to_msec(lrtsk.slicetime_u);
	pk_rtsk->waitmask  = lrtsk.waitmask;
	pk_rtsk->texmask   = lrtsk.texmask;
	pk_rtsk->tskevent  = lrtsk.tskevent;

	return ercd;
}

SYSCALL ER _tk_ref_tsk_u( ID tskid, T_RTSK_U *pk_rtsk )
{
	TCB	*tcb;
	TSTAT	state;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	memset(pk_rtsk, 0, sizeof(*pk_rtsk));

	BEGIN_CRITICAL_SECTION;
	state = (TSTAT)tcb->state;
	if ( state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		if ( ( state == TS_READY ) && ( tcb == ctxtsk ) ) {
			pk_rtsk->tskstat = TTS_RUN;
		} else {
			pk_rtsk->tskstat = (UINT)state << 1;
		}
		if ( (state & TS_WAIT) != 0 ) {
			pk_rtsk->tskwait = tcb->wspec->tskwait;
			pk_rtsk->wid     = tcb->wid;
			if ( tcb->nodiswai ) {
				pk_rtsk->tskstat |= TTS_NODISWAI;
			}
		}
		pk_rtsk->exinf	     = tcb->exinf;
		pk_rtsk->tskpri	     = ext_tskpri(tcb->priority);
		pk_rtsk->tskbpri     = ext_tskpri(tcb->bpriority);
		pk_rtsk->wupcnt	     = tcb->wupcnt;
		pk_rtsk->suscnt	     = tcb->suscnt;
		pk_rtsk->slicetime_u = tcb->slicetime;
		pk_rtsk->waitmask    = tcb->waitmask;
		pk_rtsk->texmask     = tcb->texmask;
		pk_rtsk->tskevent    = tcb->tskevt;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Get task statistic information
 */
SYSCALL ER _tk_inf_tsk( ID tskid, T_ITSK *pk_itsk, BOOL clr )
{
	T_ITSK_U litsk;
	ER	ercd;

	ercd = _tk_inf_tsk_u(tskid, &litsk, clr);

	pk_itsk->stime = to_msec(litsk.stime_u);
	pk_itsk->utime = to_msec(litsk.utime_u);

	return ercd;
}

SYSCALL ER _tk_inf_tsk_u( ID tskid, T_ITSK_U *pk_itsk, BOOL clr )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		pk_itsk->stime_u = tcb->stime;
		pk_itsk->utime_u = tcb->utime;
		if ( clr ) {
			tcb->stime = 0;
			tcb->utime = 0;
		}
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Get task resource group
 */
SYSCALL ID _tk_get_rid( ID tskid )
{
	TCB	*tcb;
	ER	ercd;


	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		ercd = tcb->resid;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Set task resource group
 */
SYSCALL ID _tk_set_rid( ID tskid, ID resid )
{
	TCB	*tcb;
	ER	ercd;

	CHECK_TSKID_SELF(tskid);
	CHECK_RESID(resid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		ercd = tcb->resid;
		tcb->resid = resid;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Release wait
 */
SYSCALL ER _tk_rel_wai( ID tskid )
{
	TCB	*tcb;
	TSTAT	state;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	state = (TSTAT)tcb->state;
	if ( (state & TS_WAIT) == 0 ) {
		ercd = ( state == TS_NONEXIST )? E_NOEXS: E_OBJ;
	} else {
		wait_release_ng(tcb, E_RLWAI);
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Disable task wait
 */
#define WAIT_PATTERN	( TTW_SLP  | TTW_DLY  | TTW_SEM | TTW_FLG  \
					      | TTW_MBX | TTW_MTX  \
			| TTW_SMBF | TTW_RMBF | TTW_CAL | TTW_ACP  \
			| TTW_RDV  | TTW_MPF  | TTW_MPL            \
			| TTW_EV1  | TTW_EV2  | TTW_EV3 | TTW_EV4  \
			| TTW_EV5  | TTW_EV6  | TTW_EV7 | TTW_EV8  \
			| TTX_SVC )

SYSCALL INT _tk_dis_wai( ID tskid, UINT waitmask )
{
	TCB	*tcb;
	UINT	tskwait = 0;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);
	CHECK_PAR(((waitmask & ~WAIT_PATTERN) == 0)&&((waitmask & WAIT_PATTERN) != 0));

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
		goto error_exit;
	}

	/* Update wait disable mask */
	tcb->waitmask = waitmask;

	if ( (tcb->state & TS_WAIT) != 0 ) {
		tskwait = tcb->wspec->tskwait;
		if ( (tskwait & waitmask) != 0 && !tcb->nodiswai ) {
			/* Free wait */
			wait_release_ng(tcb, E_DISWAI);
			tskwait = 0;
		}
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ( ercd < E_OK )? (INT)ercd: (INT)tskwait;
}

/*
 * Enable task wait
 */
SYSCALL ER _tk_ena_wai( ID tskid )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		tcb->waitmask = 0;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */
/*
 *	Debug support function
 */
#if USE_DBGSPT

/*
 * Get object name from control block
 */
#if USE_OBJECT_NAME
EXPORT ER task_getname(ID id, UB **name)
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(id);

	BEGIN_DISABLE_INTERRUPT;
	tcb = get_tcb_self(id);
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (tcb->tskatr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = tcb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer task usage state
 */
SYSCALL INT _td_lst_tsk( ID list[], INT nent )
{
	TCB	*tcb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = tcb_table + NUM_TSKID;
	for ( tcb = tcb_table; tcb < end; tcb++ ) {
		if ( tcb->state == TS_NONEXIST ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = tcb->tskid;
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer task state
 */
SYSCALL ER _td_ref_tsk( ID tskid, TD_RTSK *pk_rtsk )
{
	TD_RTSK_U	lrtsk;
	ER		ercd;

	ercd = _td_ref_tsk_u(tskid, &lrtsk);

	pk_rtsk->exinf	   = lrtsk.exinf;
	pk_rtsk->tskpri	   = lrtsk.tskpri;
	pk_rtsk->tskbpri   = lrtsk.tskbpri;
	pk_rtsk->tskstat   = lrtsk.tskstat;
	pk_rtsk->tskwait   = lrtsk.tskwait;
	pk_rtsk->wid	   = lrtsk.wid;
	pk_rtsk->wupcnt	   = lrtsk.wupcnt;
	pk_rtsk->suscnt	   = lrtsk.suscnt;
	pk_rtsk->slicetime = to_msec(lrtsk.slicetime_u);
	pk_rtsk->waitmask  = lrtsk.waitmask;
	pk_rtsk->texmask   = lrtsk.texmask;
	pk_rtsk->tskevent  = lrtsk.tskevent;
	pk_rtsk->task	   = lrtsk.task;
	pk_rtsk->stksz	   = lrtsk.stksz;
	pk_rtsk->sstksz	   = lrtsk.sstksz;
	pk_rtsk->istack	   = lrtsk.istack;
	pk_rtsk->isstack   = lrtsk.isstack;

	return ercd;
}

SYSCALL ER _td_ref_tsk_u( ID tskid, TD_RTSK_U *pk_rtsk )
{
	TCB	*tcb;
	TSTAT	state;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	memset(pk_rtsk, 0, sizeof(*pk_rtsk));

	BEGIN_DISABLE_INTERRUPT;
	state = (TSTAT)tcb->state;
	if ( state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		if ( ( state == TS_READY ) && ( tcb == ctxtsk ) ) {
			pk_rtsk->tskstat = TTS_RUN;
		} else {
			pk_rtsk->tskstat = (UINT)state << 1;
		}
		if ( (state & TS_WAIT) != 0 ) {
			pk_rtsk->tskwait = tcb->wspec->tskwait;
			pk_rtsk->wid     = tcb->wid;
			if ( ctxtsk->nodiswai ) {
				pk_rtsk->tskstat |= TTS_NODISWAI;
			}
		}
		pk_rtsk->exinf	     = tcb->exinf;
		pk_rtsk->tskpri	     = ext_tskpri(tcb->priority);
		pk_rtsk->tskbpri     = ext_tskpri(tcb->bpriority);
		pk_rtsk->wupcnt	     = tcb->wupcnt;
		pk_rtsk->suscnt	     = tcb->suscnt;
		pk_rtsk->slicetime_u = tcb->slicetime;
		pk_rtsk->waitmask    = tcb->waitmask;
		pk_rtsk->texmask     = tcb->texmask;
		pk_rtsk->tskevent    = tcb->tskevt;

		pk_rtsk->task      = tcb->task;
		pk_rtsk->stksz     = tcb->stksz;
		pk_rtsk->sstksz    = tcb->sstksz - RESERVE_SSTACK(tcb->tskatr);
		pk_rtsk->istack    = tcb->istack;
		pk_rtsk->isstack   = tcb->isstack;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Get task statistic information
 */
SYSCALL ER _td_inf_tsk( ID tskid, TD_ITSK *pk_itsk, BOOL clr )
{
	TD_ITSK_U	litsk;
	ER		ercd;

	ercd = _td_inf_tsk_u(tskid, &litsk, clr);

	pk_itsk->stime = to_msec(litsk.stime_u);
	pk_itsk->utime = to_msec(litsk.utime_u);

	return ercd;
}

SYSCALL ER _td_inf_tsk_u( ID tskid, TD_ITSK_U *pk_itsk, BOOL clr )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_DISABLE_INTERRUPT;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		pk_itsk->stime_u = tcb->stime;
		pk_itsk->utime_u = tcb->utime;
		if ( clr ) {
			tcb->stime = 0;
			tcb->utime = 0;
		}
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_task_sstack_base
 Input		:struct task *task
 		 < task to get its kernel stack base pointer >
 Output		:void
 Return		:unsigned long
 		 < base pointer of kernel stack >
 Description	:get base pointer of kernel stack
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT unsigned long get_task_sstack_base(struct task *task)
{
	unsigned long stackp = (unsigned long)task->isstack;
	
	stackp = stackp - task->sstksz + RESERVE_SSTACK(task->tskatr);
	
	return(stackp);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_task
 Input		:void
 Output		:void
 Return		:struct task*
		 < allocated task >
 Description	:allocate a empty task
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct task* alloc_task(void)
{
	struct task	*tcb;
	struct task	*current_task;
	void		*sstack;
	unsigned long	sstksz;
	
	/* -------------------------------------------------------------------- */
	/* allocate kernel stack						*/
	/* -------------------------------------------------------------------- */
	sstack = IAmalloc((UINT)KERNEL_STACK_SIZE, TA_RNG0);
	
	if (!sstack) {
		return(NULL);
	}
	
	sstksz = KERNEL_STACK_SIZE;
	
	current_task = get_current_task();
	
	// user stack is allocated on exec
	
	/* -------------------------------------------------------------------- */
	/* allocate task control block						*/
	/* -------------------------------------------------------------------- */
	BEGIN_CRITICAL_SECTION {
		tcb = (TCB*)QueRemoveNext(&free_tcb);
	} END_CRITICAL_SECTION;
	
	if (!tcb) {
		goto fail_tcb;
	}
	
	//memset((void*)tcb, 0x00, sizeof(struct task));

	tcb->tskatr	= current_task->tskatr;
	tcb->sstksz 	= sstksz;

	tcb->isstack	= (VB*)sstack + sstksz - RESERVE_SSTACK(tcb->tskatr);
	tcb->stksz	= current_task->stksz;
	tcb->istack	= current_task->istack;

	tcb->exinf	= current_task->exinf;
	tcb->task	= NULL;
	tcb->resid	= current_task->resid;
	tcb->reqdct	= current_task->reqdct;
	tcb->isysmode	= current_task->isysmode;
	tcb->sysmode	= current_task->sysmode;
	tcb->ipriority	= current_task->ipriority;
	tcb->bpriority	= current_task->bpriority;
	tcb->priority	= current_task->priority;
	
	tcb->state	= current_task->state;
	tcb->nodiswai	= FALSE;
	tcb->klockwait	= FALSE;
	tcb->klocked	= FALSE;
	
	tcb->texhdr	= current_task->texhdr;
	tcb->texmask	= current_task->texmask;
	
	tcb->texflg	= current_task->texflg;
	tcb->execssid	= current_task->execssid;
	
	tcb->state	= TS_DORMANT;
	
	tcb->tskid = get_task_id(tcb);
	
	/* -------------------------------------------------------------------- */
	/* process management			 				*/
	/* -------------------------------------------------------------------- */
	/* *proc must be set by caller						*/
	init_list(&tcb->task_node);
	tcb->set_child_tid = NULL;
	tcb->clear_child_tid = NULL;
	
	
	return(tcb);
	
fail_tcb:
	IAfree(sstack, TA_RNG0);
	return(NULL);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_task
 Input		:struct task *task
		 < to be freed >
 Output		:void
 Return		:void
 Description	:free a task
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER free_task(struct task *task)
{
	TSTAT	state;
	ER	ercd = E_OK;
	
	_ter_tsk(task);
	make_dormant(task);
	
	_del_tsk(task);
	
	return(ercd);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_task_self
 Input		:struct task *me
		 < to be freed >
 Output		:void
 Return		:void
 Description	:free me!
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER free_task_self(struct task *me)
{
	TSTAT	state;

	if ( me->svclocked != NULL ) {
		/* Unlock all extended SVC locks */
		AllUnlockSVC(me);
	}
	
	state = (TSTAT)me->state;
	if ( state == TS_READY ) {
		make_non_ready(me);
	} else if ( (state & TS_WAIT) != 0 ) {
		wait_cancel(me);
		if ( me->wspec->rel_wai_hook != NULL ) {
			(*me->wspec->rel_wai_hook)(me);
		}
	}

#ifdef NUM_MTXID
	/* signal mutex */
	signal_all_mutex(me);
#endif

	del_list(&me->task_node);
	cleanup_context(me);
	
	make_dormant(me);
	_del_tsk(me);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif /* USE_DBGSPT */
