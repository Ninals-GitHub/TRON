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
 *	time_calls.c (T-Kernel/OS)
 *	Time Management Function
 */

#include <tk/kernel.h>
#include <tk/timer.h>
#include <tk/task.h>
#include <tk/wait.h>
#include "check.h"
#include "tkdev_timer.h"
#include <sys/rominfo.h>

/*
 * Set system clock
 */
SYSCALL ER _tk_set_tim( CONST SYSTIM *pk_tim )
{
	CHECK_PAR(pk_tim->hi >= 0);

	BEGIN_CRITICAL_SECTION;
	real_time_ofs = ll_sub(toLSYSTIM(pk_tim), current_time);
	END_CRITICAL_SECTION;

	return E_OK;
}

SYSCALL ER _tk_set_tim_u( SYSTIM_U tim_u )
{
	CHECK_PAR(tim_u >= 0);

	BEGIN_CRITICAL_SECTION;
	real_time_ofs = ll_sub(ltoll(tim_u), current_time);
	END_CRITICAL_SECTION;

	return E_OK;
}

/*
 * Refer system clock
 */
SYSCALL ER _tk_get_tim( SYSTIM *pk_tim )
{
	BEGIN_CRITICAL_SECTION;
	*pk_tim = toSYSTIM(real_time(), NULL);
	END_CRITICAL_SECTION;

	return E_OK;
}

SYSCALL ER _tk_get_tim_u( SYSTIM_U *tim_u, UINT *ofs )
{
	BEGIN_CRITICAL_SECTION;
	*tim_u = lltol(real_time());
	if ( ofs != NULL ) *ofs = get_hw_timer_nsec();
	END_CRITICAL_SECTION;

	return E_OK;
}

/*
 * Refer system operating time
 */
SYSCALL ER _tk_get_otm( SYSTIM *pk_tim )
{
	BEGIN_CRITICAL_SECTION;
	*pk_tim = toSYSTIM(current_time, NULL);
	END_CRITICAL_SECTION;

	return E_OK;
}

SYSCALL ER _tk_get_otm_u( SYSTIM_U *tim_u, UINT *ofs )
{
	BEGIN_CRITICAL_SECTION;
	*tim_u = lltol(current_time);
	if ( ofs != NULL ) *ofs = get_hw_timer_nsec();
	END_CRITICAL_SECTION;

	return E_OK;
}

#if USE_DBGSPT
/*
 * Refer system clock
 */
SYSCALL ER _td_get_tim( SYSTIM *tim, UINT *ofs )
{
	UINT	us;

	BEGIN_DISABLE_INTERRUPT;
	*tim = toSYSTIM(real_time(), &us);
	*ofs = us * 1000 + get_hw_timer_nsec();
	END_DISABLE_INTERRUPT;

	return E_OK;
}

SYSCALL ER _td_get_tim_u( SYSTIM_U *tim_u, UINT *ofs )
{
	BEGIN_DISABLE_INTERRUPT;
	*tim_u = lltol(real_time());
	*ofs = get_hw_timer_nsec();
	END_DISABLE_INTERRUPT;

	return E_OK;
}

/*
 * Refer system operating time
 */
SYSCALL ER _td_get_otm( SYSTIM *tim, UINT *ofs )
{
	UINT	us;

	BEGIN_DISABLE_INTERRUPT;
	*tim = toSYSTIM(current_time, &us);
	*ofs = us * 1000 + get_hw_timer_nsec();
	END_DISABLE_INTERRUPT;

	return E_OK;
}

SYSCALL ER _td_get_otm_u( SYSTIM_U *tim_u, UINT *ofs )
{
	BEGIN_DISABLE_INTERRUPT;
	*tim_u = lltol(current_time);
	*ofs = get_hw_timer_nsec();
	END_DISABLE_INTERRUPT;

	return E_OK;
}
#endif /* USE_DBGSPT */

/* ------------------------------------------------------------------------ */

/*
 * Definition of task delay wait specification
 */
LOCAL CONST WSPEC wspec_dly = { TTW_DLY, NULL, NULL };

/*
 * Task delay
 */
SYSCALL ER _tk_dly_tsk( RELTIM dlytim )
{
	return _tk_dly_tsk_u(to_usec(dlytim));
}

SYSCALL ER _tk_dly_tsk_u( RELTIM_U dlytim )
{
	ER	ercd = E_OK;

	CHECK_DISPATCH();

	if ( dlytim > 0 ) {
		BEGIN_CRITICAL_SECTION;
		/* Check wait disable */
		if ( (ctxtsk->waitmask & TTW_DLY) != 0 ) {
			ercd = E_DISWAI;
		} else {
			ctxtsk->wspec = &wspec_dly;
			ctxtsk->wid = 0;
			ctxtsk->wercd = &ercd;
			make_wait_reltim(dlytim, TA_NULL);
			QueInit(&ctxtsk->tskque);
		}
		END_CRITICAL_SECTION;
	}

	return ercd;
}

/* ------------------------------------------------------------------------ */
/*
 *	Cyclic handler
 */

#ifdef NUM_CYCID

EXPORT ID	max_cycid;	/* Maximum interval start ID */

/*
 * Cyclic handler control block
 */
typedef struct cyclic_handler_control_block {
	void	*exinf;		/* Extended information */
	ATR	cycatr;		/* Cyclic handler attribute */
	FP	cychdr;		/* Cyclic handler address */
	UINT	cycstat;	/* Cyclic handler state */
	RELTIM_U cyctim;	/* Cyclic time */
	TMEB	cyctmeb;	/* Timer event block */
#if TA_GP
	void	*gp;		/* Global pointer */
#endif
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
} CYCCB;

LOCAL CYCCB	*cyccb_table;	/* Cyclic handler control block */
LOCAL QUEUE	free_cyccb;	/* FreeQue */

#define get_cyccb(id)	( &cyccb_table[INDEX_CYC(id)] )


/*
 * Initialization of cyclic handler control block
 */
EXPORT ER cyclichandler_initialize( void )
{
	CYCCB	*cyccb, *end;
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXCYCID, &max_cycid, 1);
	if ( n < 1 || NUM_CYCID < 1 ) {
		return E_SYS;
	}

	/* Create cyclic handler control block */
	cyccb_table = Imalloc((UINT)NUM_CYCID * sizeof(CYCCB));
	if ( cyccb_table == NULL ) {
		return E_NOMEM;
	}

	/* Register all control blocks onto FeeQue */
	QueInit(&free_cyccb);
	end = cyccb_table + NUM_CYCID;
	for ( cyccb = cyccb_table; cyccb < end; cyccb++ ) {
		cyccb->cychdr = NULL; /* Unregistered handler */
		QueInsert((QUEUE*)cyccb, &free_cyccb);
	}

	return E_OK;
}


/*
 * Next startup time
 */
Inline LSYSTIM cyc_next_time( CYCCB *cyccb )
{
	LSYSTIM		tm;
	longlong	n;

	tm = ll_add(cyccb->cyctmeb.time, uitoll(cyccb->cyctim));

	if ( ll_cmp(tm, current_time) <= 0 ) {

		/* Adjust time to be later than current time */
		tm = ll_sub(current_time, cyccb->cyctmeb.time);
		n  = lui_div(tm, cyccb->cyctim);
		ll_inc(&n);
		tm = lui_mul(n, cyccb->cyctim);
		tm = ll_add(cyccb->cyctmeb.time, tm);
	}

	return tm;
}

LOCAL void call_cychdr( CYCCB* cyccb );

/*
 * Register timer event queue
 */
Inline void cyc_timer_insert( CYCCB *cyccb, LSYSTIM tm )
{
	timer_insert_abs(&cyccb->cyctmeb, tm, (CBACK)call_cychdr, cyccb);
}

/*
 * Cyclic handler routine
 */
LOCAL void call_cychdr( CYCCB *cyccb )
{
	/* Set next startup time */
	cyc_timer_insert(cyccb, cyc_next_time(cyccb));

	/* Execute cyclic handler / Enable interrupt nest */
#ifdef _STD_X86_
	BEGIN_ENABLE_INTERRUPT;
#else
	ENABLE_INTERRUPT_UPTO(TIMER_INTLEVEL);
#endif
	CallUserHandlerP1(cyccb->exinf, cyccb->cychdr, cyccb);
#ifdef _STD_X86_
	END_ENABLE_INTERRUPT;
#else
	DISABLE_INTERRUPT;
#endif
}

/*
 * Immediate call of cyclic handler
 */
LOCAL void immediate_call_cychdr( CYCCB *cyccb )
{
	/* Set next startup time */
	cyc_timer_insert(cyccb, cyc_next_time(cyccb));

	/* Execute cyclic handler in task-independent part
	   (Keep interrupt disabled) */
	ENTER_TASK_INDEPENDENT;
	CallUserHandlerP1(cyccb->exinf, cyccb->cychdr, cyccb);
	LEAVE_TASK_INDEPENDENT;
}


/*
 * Create cyclic handler
 */
SYSCALL ID _tk_cre_cyc P1( CONST T_CCYC *pk_ccyc )
{
	T_CCYC_U lccyc;

	lccyc.exinf    = pk_ccyc->exinf;
	lccyc.cycatr   = pk_ccyc->cycatr;
	lccyc.cychdr   = pk_ccyc->cychdr;
	lccyc.cyctim_u = to_usec(pk_ccyc->cyctim);
	lccyc.cycphs_u = to_usec(pk_ccyc->cycphs);
#if USE_OBJECT_NAME
	if ( (pk_ccyc->cycatr & TA_DSNAME) != 0 ) {
		strncpy((char*)lccyc.dsname, (char*)pk_ccyc->dsname, OBJECT_NAME_LENGTH);
	}
#endif

#if TA_GP
	lccyc.gp = pk_ccyc->gp;
	return _tk_cre_cyc_u(&lccyc, 0, 0, 0, 0, gp);
#else
	return _tk_cre_cyc_u(&lccyc);
#endif
}

SYSCALL ID _tk_cre_cyc_u P1( CONST T_CCYC_U *pk_ccyc )
{
#if CHK_RSATR
	const ATR VALID_CYCATR = {
		 TA_HLNG
		|TA_STA
		|TA_PHS
		|TA_GP
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	CYCCB	*cyccb;
	LSYSTIM	tm;
	ER	ercd = E_OK;

	CHECK_RSATR(pk_ccyc->cycatr, VALID_CYCATR);
	CHECK_PAR(pk_ccyc->cychdr != NULL);
	CHECK_PAR(pk_ccyc->cyctim_u > 0);

	BEGIN_CRITICAL_SECTION;
	/* Get control block from FreeQue */
	cyccb = (CYCCB*)QueRemoveNext(&free_cyccb);
	if ( cyccb == NULL ) {
		ercd = E_LIMIT;
		goto error_exit;
	}

	/* Initialize control block */
	cyccb->exinf   = pk_ccyc->exinf;
	cyccb->cycatr  = pk_ccyc->cycatr;
	cyccb->cychdr  = pk_ccyc->cychdr;
	cyccb->cyctim  = pk_ccyc->cyctim_u;
#if USE_OBJECT_NAME
	if ( (pk_ccyc->cycatr & TA_DSNAME) != 0 ) {
		strncpy((char*)cyccb->name, (char*)pk_ccyc->dsname, OBJECT_NAME_LENGTH);
	}
#endif
#if TA_GP
	if ( (pk_ccyc->cycatr & TA_GP) != 0 ) {
		gp = pk_ccyc->gp;
	}
	cyccb->gp = gp;
#endif

	/* First startup time
	 *	To guarantee the start of handler after the specified time,
	 *	add adjust_time().
	 */
	tm = ll_add(current_time, uitoll(pk_ccyc->cycphs_u));
	tm = ll_add(tm, uitoll(adjust_time()));

	if ( (pk_ccyc->cycatr & TA_STA) != 0 ) {
		/* Start cyclic handler */
		cyccb->cycstat = TCYC_STA;

		if ( pk_ccyc->cycphs_u == 0 ) {
			/* Immediate execution */
			cyccb->cyctmeb.time = tm;
			immediate_call_cychdr(cyccb);
		} else {
			/* Register onto timer event queue */
			cyc_timer_insert(cyccb, tm);
		}
	} else {
		/* Initialize only counter */
		cyccb->cycstat = TCYC_STP;
		cyccb->cyctmeb.time = tm;
	}

	ercd = ID_CYC(cyccb - cyccb_table);

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Delete cyclic handler
 */
SYSCALL ER _tk_del_cyc( ID cycid )
{
	CYCCB	*cyccb;
	ER	ercd = E_OK;

	CHECK_CYCID(cycid);

	cyccb = get_cyccb(cycid);

	BEGIN_CRITICAL_SECTION;
	if ( cyccb->cychdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
	} else {
		if ( (cyccb->cycstat & TCYC_STA) != 0 ) {
			/* Delete timer event queue */
			timer_delete(&cyccb->cyctmeb);
		}

		/* Return to FreeQue */
		QueInsert((QUEUE*)cyccb, &free_cyccb);
		cyccb->cychdr = NULL; /* Unregistered handler */
	}
	END_CRITICAL_SECTION;

	return ercd;
}


/*
 * Start cyclic handler
 */
SYSCALL ER _tk_sta_cyc( ID cycid )
{
	CYCCB	*cyccb;
	LSYSTIM	tm;
	ER	ercd = E_OK;

	CHECK_CYCID(cycid);

	cyccb = get_cyccb(cycid);

	BEGIN_CRITICAL_SECTION;
	if ( cyccb->cychdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
		goto error_exit;
	}

	if ( (cyccb->cycatr & TA_PHS) != 0 ) {
		/* Continue cyclic phase */
		if ( (cyccb->cycstat & TCYC_STA) == 0 ) {
			/* Start cyclic handler */
			tm = cyccb->cyctmeb.time;
			if ( ll_cmp(tm, current_time) <= 0 ) {
				tm = cyc_next_time(cyccb);
			}
			cyc_timer_insert(cyccb, tm);
		}
	} else {
		/* Reset cyclic interval */
		if ( (cyccb->cycstat & TCYC_STA) != 0 ) {
			/* Stop once */
			timer_delete(&cyccb->cyctmeb);
		}

		/* FIRST ACTIVATION TIME
		 *	Adjust the first activation time with adjust_time().
		 */
		tm = ll_add(current_time, uitoll(cyccb->cyctim));
		tm = ll_add(tm, uitoll(adjust_time()));

		/* Start cyclic handler */
		cyc_timer_insert(cyccb, tm);
	}
	cyccb->cycstat |= TCYC_STA;

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Stop cyclic handler
 */
SYSCALL ER _tk_stp_cyc( ID cycid )
{
	CYCCB	*cyccb;
	ER	ercd = E_OK;

	CHECK_CYCID(cycid);

	cyccb = get_cyccb(cycid);

	BEGIN_CRITICAL_SECTION;
	if ( cyccb->cychdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
	} else {
		if ( (cyccb->cycstat & TCYC_STA) != 0 ) {
			/* Stop cyclic handler */
			timer_delete(&cyccb->cyctmeb);
		}
		cyccb->cycstat &= ~TCYC_STA;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Refer cyclic handler state
 */
SYSCALL ER _tk_ref_cyc( ID cycid, T_RCYC* pk_rcyc )
{
	T_RCYC_U lrcyc;
	ER	ercd;

	ercd = _tk_ref_cyc_u(cycid, &lrcyc);

	pk_rcyc->exinf	 = lrcyc.exinf;
	pk_rcyc->lfttim	 = to_msec(lrcyc.lfttim_u);
	pk_rcyc->cycstat = lrcyc.cycstat;

	return ercd;
}

SYSCALL ER _tk_ref_cyc_u( ID cycid, T_RCYC_U *pk_rcyc )
{
	CYCCB	*cyccb;
	LSYSTIM	tm;
	ER	ercd = E_OK;

	CHECK_CYCID(cycid);

	cyccb = get_cyccb(cycid);

	BEGIN_CRITICAL_SECTION;
	if ( cyccb->cychdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
	} else {
		tm = cyccb->cyctmeb.time;
		if ( (cyccb->cycstat & TCYC_STA) == 0 ) {
			if ( ll_cmp(tm, current_time) <= 0 ) {
				tm = cyc_next_time(cyccb);
			}
		}
		tm = ll_sub(tm, current_time);
		tm = ll_sub(tm, uitoll(adjust_time()));
		if ( ll_sign(tm) < 0 ) {
			tm = ltoll(0);
		}

		pk_rcyc->exinf    = cyccb->exinf;
		pk_rcyc->lfttim_u = (RELTIM_U)tm;
		pk_rcyc->cycstat  = cyccb->cycstat;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

#if USE_DBGSPT

/*
 * Get object name from control block
 */
#if USE_OBJECT_NAME
EXPORT ER cyclichandler_getname(ID id, UB **name)
{
	CYCCB	*cyccb;
	ER	ercd = E_OK;

	CHECK_CYCID(id);

	BEGIN_DISABLE_INTERRUPT;
	cyccb = get_cyccb(id);
	if ( cyccb->cychdr == NULL ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (cyccb->cycatr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = cyccb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer cyclic handler usage state
 */
SYSCALL INT _td_lst_cyc( ID list[], INT nent )
{
	CYCCB	*cyccb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = cyccb_table + NUM_CYCID;
	for ( cyccb = cyccb_table; cyccb < end; cyccb++ ) {
		/* Unregistered handler */
		if ( cyccb->cychdr == NULL ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = ID_CYC(cyccb - cyccb_table);
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer cyclic handler state
 */
SYSCALL ER _td_ref_cyc( ID cycid, TD_RCYC* pk_rcyc )
{
	TD_RCYC_U	lrcyc;
	ER		ercd;

	ercd = _td_ref_cyc_u(cycid, &lrcyc);

	pk_rcyc->exinf	 = lrcyc.exinf;
	pk_rcyc->lfttim	 = to_msec(lrcyc.lfttim_u);
	pk_rcyc->cycstat = lrcyc.cycstat;

	return ercd;
}

SYSCALL ER _td_ref_cyc_u( ID cycid, TD_RCYC_U *pk_rcyc )
{
	CYCCB	*cyccb;
	LSYSTIM	tm;
	ER	ercd = E_OK;

	CHECK_CYCID(cycid);

	cyccb = get_cyccb(cycid);

	BEGIN_DISABLE_INTERRUPT;
	if ( cyccb->cychdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
	} else {
		tm = cyccb->cyctmeb.time;
		if ( (cyccb->cycstat & TCYC_STA) == 0 ) {
			if ( ll_cmp(tm, current_time) <= 0 ) {
				tm = cyc_next_time(cyccb);
			}
		}
		tm = ll_sub(tm, current_time);
		tm = ll_sub(tm, uitoll(adjust_time()));
		if ( ll_sign(tm) < 0 ) {
			tm = ltoll(0);
		}

		pk_rcyc->exinf    = cyccb->exinf;
		pk_rcyc->lfttim_u = (RELTIM_U)tm;
		pk_rcyc->cycstat  = cyccb->cycstat;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_DBGSPT */
#endif /* NUM_CYCID */

/* ------------------------------------------------------------------------ */
/*
 *	Alarm handler
 */

#ifdef NUM_ALMID

EXPORT ID	max_almid;	/* Maximum alarm handler ID */

/*
 * Alarm handler control block
 */
typedef struct alarm_handler_control_block {
	void	*exinf;		/* Extended information */
	ATR	almatr;		/* Alarm handler attribute */
	FP	almhdr;		/* Alarm handler address */
	UINT	almstat;	/* Alarm handler state */
	TMEB	almtmeb;	/* Timer event block */
#if TA_GP
	void	*gp;		/* Global pointer */
#endif
#if USE_OBJECT_NAME
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif
} ALMCB;

LOCAL ALMCB	*almcb_table;	/* Alarm handler control block */
LOCAL QUEUE	free_almcb;	/* FreeQue */

#define get_almcb(id)	( &almcb_table[INDEX_ALM(id)] )


/*
 * Initialization of alarm handler control block
 */
EXPORT ER alarmhandler_initialize( void )
{
	ALMCB	*almcb, *end;
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXALMID, &max_almid, 1);
	if ( n < 1 || NUM_ALMID < 1 ) {
		return E_SYS;
	}

	/* Create alarm handler control block */
	almcb_table = Imalloc((UINT)NUM_ALMID * sizeof(ALMCB));
	if ( almcb_table == NULL ) {
		return E_NOMEM;
	}

	/* Register all control blocks onto FeeQue */
	QueInit(&free_almcb);
	end = almcb_table + NUM_ALMID;
	for ( almcb = almcb_table; almcb < end; almcb++ ) {
		almcb->almhdr = NULL; /* Unregistered handler */
		QueInsert((QUEUE*)almcb, &free_almcb);
	}

	return E_OK;
}


/*
 * Alarm handler start routine
 */
LOCAL void call_almhdr( ALMCB *almcb )
{
	almcb->almstat &= ~TALM_STA;

	/* Execute alarm handler/ Enable interrupt nesting */
	ENABLE_INTERRUPT_UPTO(TIMER_INTLEVEL);
	CallUserHandlerP1(almcb->exinf, almcb->almhdr, almcb);
	DISABLE_INTERRUPT;
}

/*
 * Alarm handler immediate call
 */
LOCAL void immediate_call_almhdr( ALMCB *almcb )
{
	almcb->almstat &= ~TALM_STA;

	/* Execute alarm handler in task-independent part
	   (Keep interrupt disabled) */
	ENTER_TASK_INDEPENDENT;
	CallUserHandlerP1(almcb->exinf, almcb->almhdr, almcb);
	LEAVE_TASK_INDEPENDENT;
}

/*
 * Register onto timer event queue
 */
Inline void alm_timer_insert( ALMCB *almcb, RELTIM_U reltim )
{
	LSYSTIM	tm;

	/* To guarantee to start the handler after the specified time,
	   add adjust_time() */
	tm = ll_add(current_time, uitoll(reltim));
	tm = ll_add(tm, uitoll(adjust_time()));

	timer_insert_abs(&almcb->almtmeb, tm, (CBACK)call_almhdr, almcb);
}


/*
 * Create alarm handler
 */
SYSCALL ID _tk_cre_alm P1( CONST T_CALM *pk_calm )
{
#if CHK_RSATR
	const ATR VALID_ALMATR = {
		 TA_HLNG
		|TA_GP
#if USE_OBJECT_NAME
		|TA_DSNAME
#endif
	};
#endif
	ALMCB	*almcb;
	ER	ercd = E_OK;

	CHECK_RSATR(pk_calm->almatr, VALID_ALMATR);
	CHECK_PAR(pk_calm->almhdr != NULL);

	BEGIN_CRITICAL_SECTION;
	/* Get control block from free queue */
	almcb = (ALMCB*)QueRemoveNext(&free_almcb);
	if ( almcb == NULL ) {
		ercd = E_LIMIT;
		goto error_exit;
	}

	/* Initialize control block */
	almcb->exinf   = pk_calm->exinf;
	almcb->almatr  = pk_calm->almatr;
	almcb->almhdr  = pk_calm->almhdr;
	almcb->almstat = TALM_STP;
#if USE_OBJECT_NAME
	if ( (pk_calm->almatr & TA_DSNAME) != 0 ) {
		strncpy((char*)almcb->name, (char*)pk_calm->dsname, OBJECT_NAME_LENGTH);
	}
#endif
#if TA_GP
	if ( (pk_calm->almatr & TA_GP) != 0 ) {
		gp = pk_calm->gp;
	}
	almcb->gp = gp;
#endif

	ercd = ID_ALM(almcb - almcb_table);

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Delete alarm handler
 */
SYSCALL ER _tk_del_alm( ID almid )
{
	ALMCB	*almcb;
	ER	ercd = E_OK;

	CHECK_ALMID(almid);

	almcb = get_almcb(almid);

	BEGIN_CRITICAL_SECTION;
	if ( almcb->almhdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
	} else {
		if ( (almcb->almstat & TALM_STA) != 0 ) {
			/* Delete from timer event queue */
			timer_delete(&almcb->almtmeb);
		}

		/* Return to FreeQue */
		QueInsert((QUEUE*)almcb, &free_almcb);
		almcb->almhdr = NULL; /* Unregistered handler */
	}
	END_CRITICAL_SECTION;

	return ercd;
}


/*
 * Start alarm handler
 */
SYSCALL ER _tk_sta_alm( ID almid, RELTIM almtim )
{
	return _tk_sta_alm_u(almid, to_usec(almtim));
}

SYSCALL ER _tk_sta_alm_u( ID almid, RELTIM_U almtim )
{
	ALMCB	*almcb;
	ER	ercd = E_OK;

	CHECK_ALMID(almid);

	almcb = get_almcb(almid);

	BEGIN_CRITICAL_SECTION;
	if ( almcb->almhdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
		goto error_exit;
	}

	if ( (almcb->almstat & TALM_STA) != 0 ) {
		/* Cancel current settings */
		timer_delete(&almcb->almtmeb);
	}

	if ( almtim > 0 ) {
		/* Register onto timer event queue */
		alm_timer_insert(almcb, almtim);
		almcb->almstat |= TALM_STA;
	} else {
		/* Immediate execution */
		immediate_call_almhdr(almcb);
	}

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Stop alarm handler
 */
SYSCALL ER _tk_stp_alm( ID almid )
{
	ALMCB	*almcb;
	ER	ercd = E_OK;

	CHECK_ALMID(almid);

	almcb = get_almcb(almid);

	BEGIN_CRITICAL_SECTION;
	if ( almcb->almhdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
	} else {
		if ( (almcb->almstat & TALM_STA) != 0 ) {
			/* Stop alarm handler address */
			timer_delete(&almcb->almtmeb);
			almcb->almstat &= ~TALM_STA;
		}
	}
	END_CRITICAL_SECTION;

	return ercd;
}


/*
 * Refer alarm handler state
 */
SYSCALL ER _tk_ref_alm( ID almid, T_RALM *pk_ralm )
{
	T_RALM_U lralm;
	ER	ercd;

	ercd = _tk_ref_alm_u(almid, &lralm);

	pk_ralm->exinf	 = lralm.exinf;
	pk_ralm->lfttim	 = to_msec(lralm.lfttim_u);
	pk_ralm->almstat = lralm.almstat;

	return ercd;
}

SYSCALL ER _tk_ref_alm_u( ID almid, T_RALM_U *pk_ralm )
{
	ALMCB	*almcb;
	LSYSTIM	tm;
	ER	ercd = E_OK;

	CHECK_ALMID(almid);

	almcb = get_almcb(almid);

	BEGIN_CRITICAL_SECTION;
	if ( almcb->almhdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
	} else {
		if ( (almcb->almstat & TALM_STA) != 0 ) {
			tm = almcb->almtmeb.time;
			tm = ll_sub(tm, current_time);
			tm = ll_sub(tm, uitoll(adjust_time()));
			if ( ll_sign(tm) < 0 ) {
				tm = ltoll(0);
			}
		} else {
			tm = ltoll(0);
		}

		pk_ralm->exinf    = almcb->exinf;
		pk_ralm->lfttim_u = (RELTIM_U)tm;
		pk_ralm->almstat  = almcb->almstat;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

#if USE_DBGSPT

/*
 * Get object name from control block
 */
#if USE_OBJECT_NAME
EXPORT ER alarmhandler_getname(ID id, UB **name)
{
	ALMCB	*almcb;
	ER	ercd = E_OK;

	CHECK_ALMID(id);

	BEGIN_DISABLE_INTERRUPT;
	almcb = get_almcb(id);
	if ( almcb->almhdr == NULL ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (almcb->almatr & TA_DSNAME) == 0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}
	*name = almcb->name;

    error_exit:
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_OBJECT_NAME */

/*
 * Refer alarm handler usage state
 */
SYSCALL INT _td_lst_alm( ID list[], INT nent )
{
	ALMCB	*almcb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = almcb_table + NUM_ALMID;
	for ( almcb = almcb_table; almcb < end; almcb++ ) {
		/* Unregistered handler */
		if ( almcb->almhdr == NULL ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = ID_ALM(almcb - almcb_table);
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer alarm handler state
 */
SYSCALL ER _td_ref_alm( ID almid, TD_RALM *pk_ralm )
{
	TD_RALM_U	lralm;
	ER		ercd;

	ercd = _td_ref_alm_u(almid, &lralm);

	pk_ralm->exinf	 = lralm.exinf;
	pk_ralm->lfttim	 = to_msec(lralm.lfttim_u);
	pk_ralm->almstat = lralm.almstat;

	return ercd;
}

SYSCALL ER _td_ref_alm_u( ID almid, TD_RALM_U *pk_ralm )
{
	ALMCB	*almcb;
	LSYSTIM	tm;
	ER	ercd = E_OK;

	CHECK_ALMID(almid);

	almcb = get_almcb(almid);

	BEGIN_DISABLE_INTERRUPT;
	if ( almcb->almhdr == NULL ) { /* Unregistered handler */
		ercd = E_NOEXS;
	} else {
		if ( (almcb->almstat & TALM_STA) != 0 ) {
			tm = almcb->almtmeb.time;
			tm = ll_sub(tm, current_time);
			tm = ll_sub(tm, uitoll(adjust_time()));
			if ( ll_sign(tm) < 0 ) {
				tm = ltoll(0);
			}
		} else {
			tm = ltoll(0);
		}

		pk_ralm->exinf	  = almcb->exinf;
		pk_ralm->lfttim_u = (RELTIM_U)tm;
		pk_ralm->almstat  = almcb->almstat;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_DBGSPT */
#endif /* NUM_ALMID */
