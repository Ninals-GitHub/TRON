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
 *	cpu_calls.c (EM1-D512)
 *	CPU-Dependent System Call
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "check.h"
#include "cpu_task.h"

#include <sys/sysinfo.h>
#include <tk/sysdef.h>
#include "cpu_insn.h"

/*
 * Dispatch enable/disable
 */
SYSCALL ER _tk_dis_dsp( void )
{
	CHECK_CTX(!in_loc());

	dispatch_disabled = DDS_DISABLE;

	return E_OK;
}

/*
 * Dispatch enable
 */
SYSCALL ER _tk_ena_dsp( void )
{
	CHECK_CTX(!in_loc());

	dispatch_disabled = DDS_ENABLE;
	if ( ctxtsk != schedtsk ) {
		dispatch();
	}

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * High level programming language
 */

/* High level programming language interrupt handler entry */
EXPORT FP hll_inthdr[N_INTVEC];

/* High level programming language routine (Interrupt) */
IMPORT void inthdr_startup();

/* High level programming language routine (Exception) */
IMPORT void exchdr_startup();

/* For default handler */
IMPORT void defaulthdr_startup();

/*
 * Interrupt handler definition
 */
SYSCALL ER _tk_def_int( UINT dintno, CONST T_DINT *pk_dint )
{
	FP	inthdr;

	CHECK_PAR(dintno < N_INTVEC);

	if ( pk_dint != NULL ) {
		/* Set interrupt handler */
		CHECK_RSATR(pk_dint->intatr, TA_HLNG);
		CHECK_PAR( !(dintno == EIT_FIQ
				&& (pk_dint->intatr & TA_HLNG) != 0) );

		inthdr = pk_dint->inthdr;

		BEGIN_CRITICAL_SECTION;
		if ( (pk_dint->intatr & TA_HLNG) != 0 ) {
			hll_inthdr[dintno] = inthdr;
			inthdr = ( dintno == EIT_DEFAULT )? defaulthdr_startup:
				 ( dintno <  EIT_FIQ     )? exchdr_startup:
				 ( dintno <= EIT_GPIO(127) )? inthdr_startup:
				                            exchdr_startup;
		}
		define_inthdr(dintno, inthdr);
		END_CRITICAL_SECTION;
	} else {
		/* Clear interrupt handler */
		switch ( dintno ) {
		  case VECNO_DEFAULT:	inthdr = SaveMonHdr.default_hdr; break;
		  case VECNO_IDEBUG:	inthdr = SaveMonHdr.idebug_hdr;	 break;
		  case VECNO_DDEBUG:	inthdr = SaveMonHdr.ddebug_hdr;	 break;
		  case VECNO_MONITOR:	inthdr = SaveMonHdr.monitor_hdr; break;
		  case VECNO_ABORTSW:	inthdr = SaveMonHdr.abortsw_hdr; break;
		  case VECNO_GIO0:	inthdr = SaveMonHdr.gio_hdr[0];	 break;
		  case VECNO_GIO1:	inthdr = SaveMonHdr.gio_hdr[1];	 break;
		  case VECNO_GIO2:	inthdr = SaveMonHdr.gio_hdr[2];	 break;
		  case VECNO_GIO3:	inthdr = SaveMonHdr.gio_hdr[3];	 break;
		  case VECNO_GIO4:	inthdr = SaveMonHdr.gio_hdr[4];	 break;
		  case VECNO_GIO5:	inthdr = SaveMonHdr.gio_hdr[5];	 break;
		  case VECNO_GIO6:	inthdr = SaveMonHdr.gio_hdr[6];	 break;
		  case VECNO_GIO7:	inthdr = SaveMonHdr.gio_hdr[7];	 break;
		  default:		inthdr = NULL;
		}

		BEGIN_CRITICAL_SECTION;
		define_inthdr(dintno, inthdr);
		hll_inthdr[dintno] = NULL;
		END_CRITICAL_SECTION;
	}

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * Get task space
 */
SYSCALL ER _tk_get_tsp( ID tskid, T_TSKSPC *pk_tskspc )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		pk_tskspc->uatb = tcb->tskctxb.uatb;
		pk_tskspc->lsid = tcb->tskctxb.lsid;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Set task space
 */
SYSCALL ER _tk_set_tsp( ID tskid, CONST T_TSKSPC *pk_tskspc )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST) {
		ercd = E_NOEXS;
	} else {
		tcb->tskctxb.uatb = pk_tskspc->uatb;
		tcb->tskctxb.lsid = pk_tskspc->lsid;

		/* When it is the currently running task,
		   switch the current space */
		if ( tcb == ctxtsk ) {
			change_space(tcb->tskctxb.uatb, tcb->tskctxb.lsid);
		}
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Set task register contents
 */
LOCAL void set_reg( TCB *tcb, CONST T_REGS *regs, CONST T_EIT *eit, CONST T_CREGS *cregs )
{
	SStackFrame	*ssp;
	UW	cpsr;
	INT	i;

	ssp = tcb->tskctxb.ssp;
	cpsr = ssp->spsr_svc;

	if ( cregs != NULL ) {
		ssp = cregs->ssp;
	}

	if ( regs != NULL ) {
		for ( i = 0; i < 12; ++i ) {
			ssp->r[i] = regs->r[i];
		}
		ssp->ip = regs->r[12];
		if ( (cpsr & PSR_M(31)) == PSR_SVC ) {
			ssp->lr_svc = regs->lr;
		} else {
			ssp->lr_usr = regs->lr;
		}
	}

	if ( eit != NULL ) {
		ssp->pc       = eit->pc;
		ssp->spsr_svc = (eit->cpsr & 0xff000000) | (cpsr & 0x00ffffff);
		if ( tcb->tskctxb.svc_ssp == NULL ) {
			ssp->taskmode = eit->taskmode;
		} else {
			/* The value immediately before the T-Kernel system call (SVC) during
			   the call is made. */
			*(tcb->tskctxb.svc_ssp - 4) = eit->taskmode;
		}
	}

	if ( cregs != NULL ) {
		tcb->tskctxb.ssp  = cregs->ssp;
		tcb->tskctxb.uatb = cregs->uatb;
		tcb->tskctxb.lsid = cregs->lsid;

		ssp->usp = cregs->usp;
	}
}

/*
 * Set task register contents
 */
SYSCALL ER _tk_set_reg( ID tskid,
	CONST T_REGS *pk_regs, CONST T_EIT *pk_eit, CONST T_CREGS *pk_cregs )
{
	TCB		*tcb;
	ER		ercd = E_OK;

	CHECK_INTSK();
	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		set_reg(tcb, pk_regs, pk_eit, pk_cregs);
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Get task register contents
 */
LOCAL void get_reg( TCB *tcb, T_REGS *regs, T_EIT *eit, T_CREGS *cregs )
{
	SStackFrame	*ssp;
	UW		cpsr;
	INT		i;

	ssp = tcb->tskctxb.ssp;
	cpsr = ssp->spsr_svc;

	if ( regs != NULL ) {
		for ( i = 0; i < 12; ++i ) {
			regs->r[i] = ssp->r[i];
		}
		regs->r[12] = ssp->ip;
		if ( (cpsr & PSR_M(31)) == PSR_SVC ) {
			regs->lr = ssp->lr_svc;
		} else {
			regs->lr = ssp->lr_usr;
		}
	}

	if ( eit != NULL ) {
		eit->pc       = ssp->pc;
		eit->cpsr     = ssp->spsr_svc;
		if ( tcb->tskctxb.svc_ssp == NULL ) {
			eit->taskmode = ssp->taskmode;
		} else {
			/* The value immediately before the T-Kernel system call (SVC) during
			   the call is made. */
			eit->taskmode = *(tcb->tskctxb.svc_ssp - 4);
		}
	}

	if ( cregs != NULL ) {
		cregs->ssp   = tcb->tskctxb.ssp;
		cregs->uatb  = tcb->tskctxb.uatb;
		cregs->lsid  = tcb->tskctxb.lsid;

		cregs->usp = ssp->usp;
	}
}

/*
 * Get task register contents
 */
SYSCALL ER _tk_get_reg( ID tskid,
		T_REGS *pk_regs, T_EIT *pk_eit, T_CREGS *pk_cregs )
{
	TCB		*tcb;
	ER		ercd = E_OK;

	CHECK_INTSK();
	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		get_reg(tcb, pk_regs, pk_eit, pk_cregs);
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Set task coprocessor register
 */
SYSCALL ER _tk_set_cpr( ID tskid, INT copno, CONST T_COPREGS *pk_copregs )
{
	ATR	copatr = TA_COP0 << copno;
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_INTSK();
	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);
	CHECK_PAR((copatr & available_cop) != 0);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else if ( (tcb->tskatr & copatr) == 0 ) {
		ercd = E_PAR;
	} else {
		/* No coprocessor */
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Get task coprocessor register
 */
SYSCALL ER _tk_get_cpr( ID tskid, INT copno, T_COPREGS *pk_copregs )
{
	ATR	copatr = TA_COP0 << copno;
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_INTSK();
	CHECK_TSKID(tskid);
	CHECK_NONSELF(tskid);
	CHECK_PAR((copatr & available_cop) != 0);

	tcb = get_tcb(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else if ( (tcb->tskatr & copatr) == 0 ) {
		ercd = E_PAR;
	} else {
		/* No coprocessor */
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
 * Set task register
 */
SYSCALL ER _td_set_reg( ID tskid, CONST T_REGS *regs, CONST T_EIT *eit, CONST T_CREGS *cregs )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);

	tcb = get_tcb(tskid);
	if ( tcb == ctxtsk ) {
		return E_OBJ;
	}

	BEGIN_DISABLE_INTERRUPT;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		set_reg(tcb, regs, eit, cregs);
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

/*
 * Get task register
 */
SYSCALL ER _td_get_reg( ID tskid, T_REGS *regs, T_EIT *eit, T_CREGS *cregs )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID(tskid);

	tcb = get_tcb(tskid);
	if ( tcb == ctxtsk ) {
		return E_OBJ;
	}

	BEGIN_DISABLE_INTERRUPT;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		get_reg(tcb, regs, eit, cregs);
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}

#endif /* USE_DBGSPT */
