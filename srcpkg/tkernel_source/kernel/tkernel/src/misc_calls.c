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
 *	misc_calls.c (T-Kernel/OS)
 *	Other System Calls
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "check.h"
#include <sys/rominfo.h>

IMPORT const T_RVER kernel_version;

/*
 * Refer system state
 */
SYSCALL ER _tk_ref_sys( T_RSYS *pk_rsys )
{
	if ( in_indp() ) {
		pk_rsys->sysstat = TSS_INDP;
	} else {
		if ( in_qtsk() ) {
			pk_rsys->sysstat = TSS_QTSK;
		} else {
			pk_rsys->sysstat = TSS_TSK;
		}
		if ( in_loc() ) {
			pk_rsys->sysstat |= TSS_DINT;
		}
		if ( in_ddsp() ) {
			pk_rsys->sysstat |= TSS_DDSP;
		}
	}
	pk_rsys->runtskid = ( ctxtsk != NULL )? ctxtsk->tskid: 0;
	pk_rsys->schedtskid = ( schedtsk != NULL )? schedtsk->tskid: 0;

	return E_OK;
}

/*
 * Refer version information
 *	If there is no kernel version information,
 *	set 0 in each information. (Do NOT cause errors.)
 */
#define RVER_PRNO_SIZE	(4)		/* T_RVER.prno[4] */
SYSCALL ER _tk_ref_ver( T_RVER *pk_rver )
{
	INT	info[RVER_PRNO_SIZE];	/* fit max number of T_RVER member */
	INT	n;			/* item number in SYSCONF */

	memcpy(pk_rver, &kernel_version, sizeof(T_RVER));

	if ( (n = _tk_get_cfn(SCTAG_MAKER, info, 1)) >= 1 ) {
		pk_rver->maker = (UH)info[0];	/* OS manufacturer */
	}
	if ( (n = _tk_get_cfn(SCTAG_PRODUCTID, info, 1)) >= 1) {
		pk_rver->prid = (UH)info[0];	/* OS identification number */
	}
	if ( (n = _tk_get_cfn(SCTAG_SPECVER, info, 1)) >= 1) {
		pk_rver->spver = (UH)info[0];	/* Specification version */
	}
	if ( (n = _tk_get_cfn(SCTAG_PRODUCTVER, info, 1)) >= 1) {
		pk_rver->prver = (UH)info[0];	/* OS product version */
	}
	if ( (n = _tk_get_cfn(SCTAG_PRODUCTNO, info, RVER_PRNO_SIZE)) >= 1) {
		if (n > RVER_PRNO_SIZE) {
			n = RVER_PRNO_SIZE;
		}
		while (--n >= 0) {
			pk_rver->prno[n] = (UH)info[n];
		}			/* Product number */
	}

	return E_OK;
}

/*
 * Number of times for disabling power-saving mode switch
 *	If it is 0, the mode switch is enabled.
 */
EXPORT UINT	lowpow_discnt = 0;

#define LOWPOW_LIMIT	0x7fff		/* Maximum number for disabling */

/*
 * Set Power-saving mode
 */
SYSCALL ER _tk_set_pow( UINT pwmode )
{
IMPORT	void	off_pow( void );	/* T-Kernel/SM */
	ER	ercd = E_OK;

	CHECK_INTSK();

	BEGIN_CRITICAL_SECTION;
	switch ( pwmode ) {
	  case TPW_DOSUSPEND:
		off_pow();
		break;

	  case TPW_DISLOWPOW:
		if ( lowpow_discnt >= LOWPOW_LIMIT ) {
			ercd = E_QOVR;
		} else {
			lowpow_discnt++;
		}
		break;
	  case TPW_ENALOWPOW:
		if ( lowpow_discnt <= 0 ) {
			ercd = E_OBJ;
		} else {
			lowpow_discnt--;
		}
		break;

	  default:
		ercd = E_PAR;
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
 * Hook routine address
 */
EXPORT FP hook_enterfn;
EXPORT FP hook_leavefn;
EXPORT FP hook_execfn;
EXPORT FP hook_stopfn;
EXPORT FP hook_ienterfn;
EXPORT FP hook_ileavefn;

#if TA_GP
EXPORT void *hook_svc_gp;
EXPORT void *hook_dsp_gp;
EXPORT void *hook_int_gp;
#endif

/*
 * Hook enable/disable setting
 */
IMPORT void hook_svc( void );
IMPORT void unhook_svc( void );
IMPORT void hook_dsp( void );
IMPORT void unhook_dsp( void );
IMPORT void hook_int( void );
IMPORT void unhook_int( void );

/*
 * Set/Cancel system call/extended SVC hook routine
 */
SYSCALL ER _td_hok_svc P1( CONST TD_HSVC *hsvc )
{
	BEGIN_DISABLE_INTERRUPT;
	if ( hsvc == NULL ) { /* Cancel system call hook routine */
		/* Cancel */
		unhook_svc();
	} else {
		/* Set */
		hook_enterfn = hsvc->enter;
		hook_leavefn = hsvc->leave;
#if TA_GP
		hook_svc_gp = gp;
#endif
		hook_svc();
	}
	END_DISABLE_INTERRUPT;

	return E_OK;
}

/*
 * Set/Cancel dispatcher hook routine */
SYSCALL ER _td_hok_dsp P1( CONST TD_HDSP *hdsp )
{
	BEGIN_DISABLE_INTERRUPT;
	if ( hdsp == NULL ) { /* Cancel dispatcher hook routine */
		/* Cancel */
		unhook_dsp();
	} else {
		/* Set */
		hook_execfn = hdsp->exec;
		hook_stopfn = hdsp->stop;
#if TA_GP
		hook_dsp_gp = gp;
#endif
		hook_dsp();
	}
	END_DISABLE_INTERRUPT;

	return E_OK;
}

/*
 * Set/Cancel EIT handler hook routine
 */
SYSCALL ER _td_hok_int P1( CONST TD_HINT *hint )
{
	BEGIN_DISABLE_INTERRUPT;
	if ( hint == NULL ) { /* Cancel interrupt handler hook routine */
		/* Cancel */
		unhook_int();
	} else {
		/* Set */
		hook_ienterfn = hint->enter;
		hook_ileavefn = hint->leave;
#if TA_GP
		hook_int_gp = gp;
#endif
		hook_int();
	}
	END_DISABLE_INTERRUPT;

	return E_OK;
}

/*
 * Refer system state
 */
SYSCALL ER _td_ref_sys( TD_RSYS *pk_rsys )
{
	if ( in_indp() ) {
		pk_rsys->sysstat = TSS_INDP;
	} else {
		if ( in_qtsk() ) {
			pk_rsys->sysstat = TSS_QTSK;
		} else {
			pk_rsys->sysstat = TSS_TSK;
		}
		if ( in_loc() ) {
			pk_rsys->sysstat |= TSS_DINT;
		}
		if ( in_ddsp() ) {
			pk_rsys->sysstat |= TSS_DDSP;
		}
	}
	pk_rsys->runtskid = ( ctxtsk != NULL )? ctxtsk->tskid: 0;
	pk_rsys->schedtskid = ( schedtsk != NULL )? schedtsk->tskid: 0;

	return E_OK;
}

#endif /* USE_DBGSPT */
