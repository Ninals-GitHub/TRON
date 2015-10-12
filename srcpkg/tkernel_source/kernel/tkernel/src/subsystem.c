/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	subsystem.c (T-Kernel/OS)
 *	Subsystem Manager/Task Exception
 */

#include <tk/kernel.h>
#include <tk/task.h>
#include "check.h"
//#include "bitop.h"
#include <tstdlib/bitop.h>
#include <sys/rominfo.h>

EXPORT ID	max_ssyid;	/* Maximum subsystem ID */
EXPORT PRI	max_ssypri;	/* Maximum subsystem priority */
EXPORT ID	max_resid;	/* Maximum resource ID */

typedef INT  (*SVC)( void *pk_para, FN fncd );	/* Extended SVC handler */
typedef void (*BrkFN)( ID tskid );		/* Break function */
typedef void (*StuFN)( ID resid, INT info );	/* Startup function */
typedef void (*CluFN)( ID resid, INT info );	/* Cleanup function */
typedef ER   (*EvtFN)( INT evttyp, ID resid, INT info );	/* Event function */

/*
 * Definition of subsystem control block
 */
typedef struct subsystem_control_block	SSYCB;
struct subsystem_control_block {
	SSYCB	*link;		/* Same subsystem priority link */
	ATR	ssyatr;		/* Subsystem attribute */
	PRI	ssypri;		/* Subsystem priority */
	SVC	svchdr;		/* Extended SVC handler */
	BrkFN	breakfn;	/* Break function */
	StuFN	startupfn;	/* Startup function */
	CluFN	cleanupfn;	/* Cleanup function */
	EvtFN	eventfn;	/* Event function */
	INT	resblksz;	/* Resource control block size */
	void	*resblk;	/* Resource control block */
#if TA_GP
	void	*gp;		/* Global pointer */
#endif
};

LOCAL SSYCB	*ssycb_table;	/* Subsystem control block */

#define get_ssycb(id)	( &ssycb_table[INDEX_SSY(id)] )

/*
 * Control table of subsystem priority
 */
LOCAL SSYCB	**ssypri_table;

#define ssypri_index(ssypri)	( (ssypri) - MIN_SSYPRI )

/*
 * Bitmap of use resource ID
 *	If the bit corresponding to the resource ID is 1, it is in use.
 */
LOCAL UB *resid_bitmap;

/*
 * Convert bitmap index and resource ID
 */
#define index_to_resid(index)	( (index) + MIN_RESID )
#define resid_to_index(resid)	( (resid) - MIN_RESID )

/*
 * Adjust alignment of resource control block
 */
#define ROUND_RESBLKSZ(resblksz)	( ((resblksz) + 3) & ~0x00000003U )

/*
 * Undefined extended SVC function
 */
IMPORT INT no_support();

/*
 * Initialization of subsystem control block
 */
EXPORT ER subsystem_initialize( void )
{
	INT	i;

	/* Get system information */
	i = _tk_get_cfn(SCTAG_TMAXSSYID, &max_ssyid, 1);
	if ( i < 1 || NUM_SSYID < 1 ) {
		return E_SYS;
	}
	i = _tk_get_cfn(SCTAG_TMAXSSYPRI, &max_ssypri, 1);
	if ( i < 1 || NUM_SSYPRI < 1 ) {
		return E_SYS;
	}

	/* Create subsystem control block */
	ssycb_table = Imalloc((UINT)NUM_SSYID * sizeof(SSYCB));
	if ( ssycb_table == NULL ) {
		goto err_ret1;
	}

	/* Create subsystem priority control table */
	ssypri_table = Imalloc((UINT)NUM_SSYPRI * sizeof(SSYCB*));
	if ( ssypri_table == NULL ) {
		goto err_ret2;
	}

	for ( i = 0; i < NUM_SSYID; i++ ) {
		ssycb_table[i].svchdr    = no_support;
		ssycb_table[i].breakfn   = NULL;
		ssycb_table[i].startupfn = NULL;
		ssycb_table[i].cleanupfn = NULL;
		ssycb_table[i].eventfn   = NULL;
	}

	for ( i = 0; i < NUM_SSYPRI; i++ ) {
		ssypri_table[i] = NULL;
	}

	return E_OK;

err_ret2:
	Ifree(ssycb_table);
err_ret1:
	return E_NOMEM;
}


/*
 * Call break function
 */
IMPORT void call_brkhdr( TCB *tcb );	/* Call break function */


/*
 * Definition of subsystem
 */
SYSCALL ER _tk_def_ssy P2( ID ssid, CONST T_DSSY *pk_dssy )
{
	SSYCB	*ssycb;
	void	*resblk;
	SSYCB	**prev;
	INT	i;
	ER	ercd = E_OK;

	CHECK_SSYID(ssid);
#if CHK_PAR
	if ( pk_dssy != NULL ) {
		CHECK_RSATR(pk_dssy->ssyatr, TA_NULL|TA_GP);
		CHECK_SSYPRI(pk_dssy->ssypri);
		CHECK_PAR(pk_dssy->resblksz >= 0);
	}
#endif

	ssycb = get_ssycb(ssid);

	resblk = NULL;
	if ( pk_dssy != NULL ) {
		if ( pk_dssy->resblksz > 0 ) {
			/* Create resource control block */
			i = ROUND_RESBLKSZ(pk_dssy->resblksz);
			resblk = Icalloc((UINT)NUM_RESID, (UINT)i);
			if ( resblk == NULL ) {
				return E_NOMEM;
			}
		}
	}

	BEGIN_CRITICAL_SECTION;
	if ( pk_dssy != NULL ) {
		/* Register */
		if ( ssycb->svchdr != no_support ) {
			ercd = E_OBJ;  /* Registered */
			goto error_exit;
		}
		ssycb->ssyatr    = pk_dssy->ssyatr;
		ssycb->ssypri    = pk_dssy->ssypri;
		ssycb->svchdr    = (SVC)pk_dssy->svchdr;
		ssycb->breakfn   = (BrkFN)pk_dssy->breakfn;
		ssycb->startupfn = (StuFN)pk_dssy->startupfn;
		ssycb->cleanupfn = (CluFN)pk_dssy->cleanupfn;
		ssycb->eventfn   = (EvtFN)pk_dssy->eventfn;
		ssycb->resblksz  = pk_dssy->resblksz;
		ssycb->resblk    = resblk;
#if TA_GP
		if ( (pk_dssy->ssyatr & TA_GP) != 0 ) {
			gp = pk_dssy->gp;
		}
		ssycb->gp = gp;
#endif

		/* Register to subsystem priority control table */
		i = ssypri_index(ssycb->ssypri);
		ssycb->link = ssypri_table[i];
		ssypri_table[i] = ssycb;
	} else {
		/* Delete */
		if ( ssycb->svchdr == no_support ) {
			ercd = E_NOEXS;  /* Not registered */
			goto error_exit;
		}

		/* Delete from subsystem priority group */
		prev = &ssypri_table[ssypri_index(ssycb->ssypri)];
		while ( *prev != NULL ) {
			if ( *prev == ssycb ) {
				*prev = ssycb->link;
				break;
			}
			prev = &(*prev)->link;
		}

		resblk = ssycb->resblk;
		ssycb->svchdr    = no_support;
		ssycb->breakfn   = NULL;
		ssycb->startupfn = NULL;
		ssycb->cleanupfn = NULL;
		ssycb->eventfn   = NULL;
	}

    error_exit:
	END_CRITICAL_SECTION;

	if ( ercd < E_OK || pk_dssy == NULL ) {
		if ( resblk != NULL ) {
			Ifree(resblk);
		}
	}

	return ercd;
}

/*
 * Call startup function
 */
SYSCALL ER _tk_sta_ssy( ID ssid, ID resid, INT info )
{
	SSYCB	*ssycb;
	StuFN	startupfn;
	UH	save_texflg;
	INT	i;
	ER	ercd = E_OK;

	CHECK_SSYID_ALL(ssid);
	CHECK_RESID(resid);
	CHECK_DISPATCH();

	//DISABLE_INTERRUPT;
	BEGIN_DISABLE_INTERRUPT;
	/* Suspend task exception while startup function is running */
	save_texflg = ctxtsk->texflg;
	ctxtsk->texflg |= SSFN_RUNNING;
	ctxtsk->sysmode++;
	//ENABLE_INTERRUPT;
	END_DISABLE_INTERRUPT;

	if ( ssid > 0 ) {
		/* Call only specified subsystem */
		ssycb = get_ssycb(ssid);
		if ( ssycb->svchdr == no_support ) {
			ercd = E_NOEXS;
		} else {
			startupfn = ssycb->startupfn;
			if ( startupfn != NULL ) {
				CallUserHandlerP2(resid, info,
							startupfn, ssycb);
			}
		}
	} else {
		/* Call all subsystems in the order of descending priorities */
		for ( i = 0; i < NUM_SSYPRI; ++i ) {
			ssycb = ssypri_table[i];
			while ( ssycb != NULL ) {
				startupfn = ssycb->startupfn;
				if ( startupfn != NULL ) {
					CallUserHandlerP2(resid, info,
							startupfn, ssycb);
				}
				ssycb = ssycb->link;
			}
		}
	}

	//DISABLE_INTERRUPT;
	BEGIN_DISABLE_INTERRUPT;
	ctxtsk->sysmode--;
	ctxtsk->texflg = save_texflg;
	//ENABLE_INTERRUPT;
	END_DISABLE_INTERRUPT;

	/* Processing if an exception occurs while suspending a
	   task exception */
	if ( ctxtsk->exectex != 0 ) {
		call_brkhdr(ctxtsk);	/* Execute break function */
	}

	return ercd;
}

/*
 * Clear resource control block
 */
LOCAL void clean_resblk( SSYCB *ssycb, ID idx )
{
	INT	sz;

	if ( ssycb->resblk != NULL ) {
		sz = ROUND_RESBLKSZ(ssycb->resblksz);
		memset((B*)ssycb->resblk + sz * idx, 0, (size_t)sz);
	}
}

/*
 * Call cleanup function
 */
SYSCALL ER _tk_cln_ssy( ID ssid, ID resid, INT info )
{
	SSYCB	*ssycb;
	CluFN	cleanupfn;
	UH	save_texflg;
	INT	i, idx;
	ER	ercd = E_OK;

	CHECK_SSYID_ALL(ssid);
	CHECK_RESID(resid);
	CHECK_DISPATCH();

	//DISABLE_INTERRUPT;
	BEGIN_DISABLE_INTERRUPT;
	/* Suspend task exception while startup function is running */
	save_texflg = ctxtsk->texflg;
	ctxtsk->texflg |= SSFN_RUNNING;
	ctxtsk->sysmode++;
	//ENABLE_INTERRUPT;
	END_DISABLE_INTERRUPT;

	idx = resid_to_index(resid);

	if ( ssid > 0 ) {
		/* Call only specified subsystem */
		ssycb = get_ssycb(ssid);
		if ( ssycb->svchdr == no_support ) {
			ercd = E_NOEXS;
		} else {
			cleanupfn = ssycb->cleanupfn;
			if ( cleanupfn != NULL ) {
				CallUserHandlerP2(resid, info,
							cleanupfn, ssycb);
			}
			clean_resblk(ssycb, idx);
		}
	} else {
		/*  Call all subsystems in the order of ascending priorities */
		for ( i = NUM_SSYPRI-1; i >= 0; --i ) {
			ssycb = ssypri_table[i];
			while ( ssycb != NULL ) {
				cleanupfn = ssycb->cleanupfn;
				if ( cleanupfn != NULL ) {
					CallUserHandlerP2(resid, info,
							cleanupfn, ssycb);
				}
				clean_resblk(ssycb, idx);
				ssycb = ssycb->link;
			}
		}
	}

	//DISABLE_INTERRUPT;
	BEGIN_DISABLE_INTERRUPT;
	ctxtsk->sysmode--;
	ctxtsk->texflg = save_texflg;
	//ENABLE_INTERRUPT;
	END_DISABLE_INTERRUPT;

	/* Processing if an exception occurs while suspending a
	   task exception */
	if ( ctxtsk->exectex != 0 ) {
		call_brkhdr(ctxtsk);	/* Execute break function */
	}

	return ercd;
}

/*
 * Call event function
 */
SYSCALL ER _tk_evt_ssy( ID ssid, INT evttyp, ID resid, INT info )
{
	SSYCB	*ssycb;
	EvtFN	eventfn;
	UH	save_texflg;
	INT	i, e, d;
	ER	ercd = E_OK;
	ER	er = E_OK;

	CHECK_SSYID_ALL(ssid);
	CHECK_RESID_ANY(resid);
	CHECK_DISPATCH();

	//DISABLE_INTERRUPT;
	BEGIN_DISABLE_INTERRUPT;
	/* Suspend task exception while startup function is running */
	save_texflg = ctxtsk->texflg;
	ctxtsk->texflg |= SSFN_RUNNING;
	ctxtsk->sysmode++;
	//ENABLE_INTERRUPT;
	END_DISABLE_INTERRUPT;

	if ( ssid > 0 ) {
		/* Call only specified subsystem */
		ssycb = get_ssycb(ssid);
		if ( ssycb->svchdr == no_support ) {
			ercd = E_NOEXS;
		} else {
			eventfn = ssycb->eventfn;
			if ( eventfn != NULL ) {
				ercd = CallUserHandlerP3(evttyp, resid, info,
							eventfn, ssycb);
			}
		}
	} else {
		if ( (evttyp % 2) == 0 ) {
			/* Even number:  Call all in the order of
			   ascending priorities */
			i = NUM_SSYPRI-1;
			e = -1;
			d = -1;
		} else {
			/* Odd number: Call all in the order of
			   descending priorities */
			i = 0;
			e = NUM_SSYPRI;
			d = +1;
		}
		for ( ; i != e; i += d ) {
			ssycb = ssypri_table[i];
			while ( ssycb != NULL ) {
				eventfn = ssycb->eventfn;
				if ( eventfn != NULL ) {
					er = CallUserHandlerP3(evttyp,resid,info,
							eventfn, ssycb);
					if ( er < 0 ) {	/* Set Latest Error	*/
						ercd = er;	/* Overwrite!	*/
					}
				}
				ssycb = ssycb->link;
			}
		}
	}

	//DISABLE_INTERRUPT;
	BEGIN_DISABLE_INTERRUPT;
	ctxtsk->sysmode--;
	ctxtsk->texflg = save_texflg;
	//ENABLE_INTERRUPT;
	END_DISABLE_INTERRUPT;

	/* Processing if an exception occurs while suspending a
	   task exception */
	if ( ctxtsk->exectex != 0 ) {
		call_brkhdr(ctxtsk);	/* Execute break function */
	}

	return ercd;
}

/*
 * Refer subsystem definition information
 */
SYSCALL ER _tk_ref_ssy( ID ssid, T_RSSY *pk_rssy )
{
	SSYCB	*ssycb;
	ER	ercd = E_OK;

	CHECK_SSYID(ssid);

	ssycb = get_ssycb(ssid);

	BEGIN_CRITICAL_SECTION;
	if ( ssycb->svchdr == no_support ) {
		ercd = E_NOEXS;
	} else {
		pk_rssy->ssypri   = ssycb->ssypri;
		pk_rssy->resblksz = ssycb->resblksz;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

#if USE_DBGSPT

/*
 * Refer subsystem usage state
 */
SYSCALL INT _td_lst_ssy( ID list[], INT nent )
{
	SSYCB	*ssycb, *end;
	INT	n = 0;

	BEGIN_DISABLE_INTERRUPT;
	end = ssycb_table + NUM_SSYID;
	for ( ssycb = ssycb_table; ssycb < end; ssycb++ ) {
		if ( ssycb->svchdr == no_support ) {
			continue;
		}

		if ( n++ < nent ) {
			*list++ = ID_SSY(ssycb - ssycb_table);
		}
	}
	END_DISABLE_INTERRUPT;

	return n;
}

/*
 * Refer subsystem definition information
 */
SYSCALL ER _td_ref_ssy( ID ssid, TD_RSSY *pk_rssy )
{
	SSYCB	*ssycb;
	ER	ercd = E_OK;

	CHECK_SSYID(ssid);

	ssycb = get_ssycb(ssid);

	BEGIN_DISABLE_INTERRUPT;
	if ( ssycb->svchdr == no_support ) {
		ercd = E_NOEXS;
	} else {
		pk_rssy->ssypri   = ssycb->ssypri;
		pk_rssy->resblksz = ssycb->resblksz;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_DBGSPT */

/*
 * Branch routine to extended SVC handler
 */
EXPORT ER svc_ientry P2GP( void *pk_para, FN fncd )
{
	ID	ssid;
	SSYCB	*ssycb;
	ID	save_execssid;
	UINT	save_waitmask;
	UINT	save_exectex;
	ER	ercd;

	/* Lower 8 bits are subsystem ID */
	ssid = fncd & 0xff;
	if ( ssid < 1 || ssid > MAX_SSYID ) {
		return E_RSFN;
	}

	ssycb = get_ssycb(ssid);

	if ( in_indp() ) {
		/* Execute at task-independent part */
		ercd = CallUserHandlerP2_GP(pk_para, fncd,
						ssycb->svchdr, ssycb);
	} else {
		/* Lock to run with break function exclusively */
		LockSVC(&ctxtsk->svclock);

		if ( (ctxtsk->waitmask & TTX_SVC) != 0 ) {
			UnlockSVC();
			return E_DISWAI; /* Disable extended SVC call */
		}

		//DISABLE_INTERRUPT;
		BEGIN_DISABLE_INTERRUPT;
		save_execssid = ctxtsk->execssid;
		save_waitmask = ctxtsk->waitmask;
		save_exectex  = ctxtsk->exectex;
		ctxtsk->execssid = ssid;
		ctxtsk->waitmask = 0;
		/* If the break function was already called,
		   clear the task exception to avoid to run the
		   extended SVC break function at called extended SVC */
		if ( save_execssid < 0 ) {	/* if MSB == 1 break handler was already executed */
			ctxtsk->exectex = 0;
		}
		ctxtsk->sysmode++;
		//ENABLE_INTERRUPT;
		END_DISABLE_INTERRUPT;

		UnlockSVC();

		/* Call extended SVC handler */
		ercd = CallUserHandlerP2_GP(pk_para, fncd,
						ssycb->svchdr, ssycb);

		/* Lock in order to run with break function exclusively */
		LockSVC(&ctxtsk->svclock);

		//DISABLE_INTERRUPT;
		BEGIN_DISABLE_INTERRUPT;
		ctxtsk->sysmode--;
		ctxtsk->execssid = save_execssid;
		ctxtsk->waitmask = save_waitmask;
		ctxtsk->exectex |= save_exectex;
		//ENABLE_INTERRUPT;
		END_DISABLE_INTERRUPT;

		UnlockSVC();

		if ( ctxtsk->exectex != 0 ) {
			call_brkhdr(ctxtsk);	/* Execute break function */
		}
	}

	return ercd;
}

/* ------------------------------------------------------------------------ */
/*
 *	Task exception function
 */

/*
 * Definition of task exception handler
 */
SYSCALL ER _tk_def_tex( ID tskid, CONST T_DTEX *pk_dtex )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);
#if CHK_PAR
	if ( pk_dtex != NULL ) {
		CHECK_RSATR(pk_dtex->texatr, TA_NULL);
	}
#endif

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( (tcb->tskatr & TA_RNG3) == TA_RNG0 ) {
		ercd = E_OBJ;
		goto error_exit;
	}

	/* Initialize task exception information */
	tcb->pendtex = 0;
	tcb->exectex = 0;
	tcb->texmask = 0;
	tcb->texhdr  = ( pk_dtex != NULL )? pk_dtex->texhdr: NULL;

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Enable/Disable task exception
 */
LOCAL ER set_tex_mask( ID tskid, UINT texmsk, BOOL enable )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST || tcb->texhdr == NULL ) {
		ercd = E_NOEXS;
		goto error_exit;
	}

	if ( enable ) {
		tcb->texmask |= texmsk;		/* Enable */
	} else {
		tcb->texmask &= ~texmsk;	/* Disable */
	}

	/* Narrow down to only the enabled task exception that is suspended */
	tcb->pendtex &= tcb->texmask;

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Disable task exception
 */
SYSCALL ER _tk_dis_tex( ID tskid, UINT texptn )
{
	return set_tex_mask(tskid, texptn, FALSE);
}

/*
 * Enable task exception
 */
SYSCALL ER _tk_ena_tex( ID tskid, UINT texptn )
{
	return set_tex_mask(tskid, texptn, TRUE);
}


/*
 * Call break function
 *	Call break function for extended SVC which 'tcb' task is executing.
 */
EXPORT void call_brkhdr( TCB *tcb )
{
	SSYCB	*ssycb;
	BrkFN	breakfn = NULL;
	UH	save_texflg = 0;
	INT	priority;
#if TA_GP
	void	*ssy_gp = NULL;
#endif

	BEGIN_CRITICAL_SECTION;
	/* If subsystem function (Startup function, Cleanup function,
	   Event function, Break function) is running, suspend the
	   task exception. */
	if ( (tcb->texflg & SSFN_RUNNING) != 0 ) {
		goto not_call;
	}

	/* When the task exception occurred with extended SVC running,
	   call break function only once */
	/* if tcb->execssid:MSB == 1 break handler was already executed */
	if ( !(tcb->exectex != 0 && tcb->execssid > 0) ) {
		goto not_call;
	}

	/* Extended SVC (subsystem) in execution */
	ssycb = get_ssycb(tcb->execssid);

	tcb->execssid |= BREAK_RAN;	/* Executed mark: Disable multiple call */
					/* BREAK_RAN is MSB bit --> tcb->execssid < 0 */

	breakfn = ssycb->breakfn;
	if ( breakfn == NULL ) {
		goto not_call;  /* Break function is not set */
	}
#if TA_GP
	ssy_gp = ssycb->gp;
#endif

	if ( tcb->priority < ctxtsk->priority ) {
		/* Raise its own task priority to the same level of the task
		   running extended SVC */
		change_task_priority(ctxtsk, tcb->priority);
	}

	/* Suspend the task exception during break function is running */
	save_texflg = ctxtsk->texflg;
	ctxtsk->texflg |= SSFN_RUNNING;
	ctxtsk->sysmode++;

    not_call:
	END_CRITICAL_SECTION;
	if ( breakfn == NULL ) {
		return;  /* Do not call break function */
	}

	/* Call break function */
#if TA_GP
	CallUserHandler(tcb->tskid, 0, 0, breakfn, ssy_gp);
#else
	(*breakfn)(tcb->tskid);
#endif

	BEGIN_CRITICAL_SECTION;
	ctxtsk->sysmode--;
	ctxtsk->texflg = save_texflg;

	/* Set the task priority back to the original level */
#ifdef NUM_MTXID
	priority = chg_pri_mutex(ctxtsk, ctxtsk->bpriority);
#else
	priority = ctxtsk->bpriority;
#endif
	if ( ctxtsk->priority != priority ) {
		change_task_priority(ctxtsk, priority);
	}
	END_CRITICAL_SECTION;
}


/*
 * Raise task exception
 */
SYSCALL ER _tk_ras_tex( ID tskid, INT texcd )
{
	TCB	*tcb;
	BOOL	rastex = FALSE;
	ER	ercd = E_OK;

	CHECK_INTSK();
	CHECK_TSKID_SELF(tskid);
	CHECK_PAR(texcd >= 0 && texcd <= 31);
	CHECK_DISPATCH();

	tcb = get_tcb_self(tskid);

	/* Lock in order to run with extended SVC handler exclusively */
	LockSVC(&tcb->svclock);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST || tcb->texhdr == NULL ) {
		ercd = E_NOEXS;
		goto error_exit;
	}
	if ( tcb->state == TS_DORMANT ) {
		ercd = E_OBJ;
		goto error_exit;
	}

	/* Ignore exceptions if '0' exception code handler is in execution */
	if ( (tcb->texflg & TEX0_RUNNING) != 0 ) {
		goto error_exit;
	}

	tcb->pendtex |= (UINT)(1 << texcd) & tcb->texmask;
	if ( tcb->pendtex == 0 ) {
		goto error_exit; /* No exception occurred */
	}

	/* The exception handler can be nested only when the exception
	   code is 0.
	   In other cases, if the exception handler is running, suspend it. */
	if ( (tcb->pendtex & 0x00000001U) != 0 || (tcb->texflg & TEX1_RUNNING) == 0 ) {
		/* Task exception occurred */
		tcb->exectex |= tcb->pendtex;
		rastex = TRUE;

		/* Request task exception handler execution */
		request_tex(tcb);
	}

    error_exit:
	END_CRITICAL_SECTION;

	if ( rastex ) {
		/* Execute break function */
		call_brkhdr(tcb);
	}

	UnlockSVC();

	if ( rastex ) {
		/* Execute self-break function
		   if task exception occurred while at break function is in executeion */
		if ( ctxtsk->exectex != 0 ) {
			call_brkhdr(ctxtsk);	/* Execute break function */
		}
	}

	return ercd;
}

/*
 * End task exception handler
 */
SYSCALL INT _tk_end_tex( BOOL enatex )
{
	INT	texcd = 0;
	UINT	texptn;

	CHECK_DISPATCH();
	if ( (ctxtsk->texflg & (TEX0_RUNNING|TEX1_RUNNING)) != TEX1_RUNNING ) {
		return E_CTX;
	}

	BEGIN_CRITICAL_SECTION;
	texptn = ctxtsk->pendtex & ~0x00000001U;  /* Except exception code 0 */
	if ( texptn != 0 ) {
		while ( (texptn & 1) == 0 ) {
			texcd++;
			texptn >>= 1;
		}
	}

	/* If enatex = TRUE or a task exception didn't occur,
	   enable the task exception after executing the task
	   exception handler  */
	if ( enatex || texcd == 0 ) {
		ctxtsk->texflg &= ~TEX1_RUNNING;
		ctxtsk->exectex |= ctxtsk->pendtex;

		if ( texcd > 0 ) {
			/* Since there is a task exception suspension,
			   request the task execution */
			request_tex(ctxtsk);
		}
	} else {
		/* Continue task exception handler execution */
		ctxtsk->pendtex &= ~(UINT)(1 << texcd);
	}
	END_CRITICAL_SECTION;

	return texcd;
}

/*
 * Refer task exception state
 */
SYSCALL ER _tk_ref_tex( ID tskid, T_RTEX *pk_rtex )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_CRITICAL_SECTION;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		pk_rtex->pendtex = tcb->pendtex;
		pk_rtex->texmask = tcb->texmask;
	}
	END_CRITICAL_SECTION;

	return ercd;
}

#if USE_DBGSPT
/*
 * Refer task exception state
 */
SYSCALL ER _td_ref_tex( ID tskid, TD_RTEX *pk_rtex )
{
	TCB	*tcb;
	ER	ercd = E_OK;

	CHECK_TSKID_SELF(tskid);

	tcb = get_tcb_self(tskid);

	BEGIN_DISABLE_INTERRUPT;
	if ( tcb->state == TS_NONEXIST ) {
		ercd = E_NOEXS;
	} else {
		pk_rtex->pendtex = tcb->pendtex;
		pk_rtex->texmask = tcb->texmask;
	}
	END_DISABLE_INTERRUPT;

	return ercd;
}
#endif /* USE_DBGSPT */

/* ------------------------------------------------------------------------ */
/*
 *	Resource group management
 */

/*
 * Initialization of resource group management
 */
EXPORT ER resource_group_initialize( void )
{
	W	n;

	/* Get system information */
	n = _tk_get_cfn(SCTAG_TMAXRESID, &max_resid, 1);
	if ( n < 1 || NUM_RESID < 1 ) {
		return E_SYS;
	}

	/* Create bitmap of resource ID */
	resid_bitmap = Icalloc(1, (UINT)((NUM_RESID + 7) / 8));
	if ( resid_bitmap == NULL ) {
		return E_NOMEM;
	}

	/* System resource always exist */
	tstdlib_bitset(resid_bitmap, resid_to_index(SYS_RESID));

	return E_OK;
}

/*
 * Create resource group
 */
SYSCALL ID _tk_cre_res( void )
{
	INT	n;
	ER	ercd;

	BEGIN_CRITICAL_SECTION;
	n = tstdlib_bitsearch0(resid_bitmap, 0, NUM_RESID);
	if ( n < 0 ) {
		ercd = E_LIMIT;
	} else {
		tstdlib_bitset(resid_bitmap, n);
		ercd = index_to_resid(n);
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Delete resource group
 */
SYSCALL ER _tk_del_res( ID resid )
{
	INT	idx;
	ER	ercd = E_OK;

	CHECK_RESID(resid);
	if ( resid == SYS_RESID ) {
		return E_ID;
	}

	BEGIN_CRITICAL_SECTION;
	idx = resid_to_index(resid);
	if ( !tstdlib_bittest(resid_bitmap, idx) ) {
		ercd = E_NOEXS;
	} else {
		tstdlib_bitclr(resid_bitmap, idx);
	}
	END_CRITICAL_SECTION;

	return ercd;
}

/*
 * Get resource control block
 */
SYSCALL ER _tk_get_res( ID resid, ID ssid, void **p_resblk )
{
	INT	idx;
	SSYCB	*ssycb;
	ER	ercd = E_OK;

	CHECK_RESID(resid);
	CHECK_SSYID(ssid);

	idx = resid_to_index(resid);
	ssycb = get_ssycb(ssid);

	BEGIN_CRITICAL_SECTION;
	if ( !tstdlib_bittest(resid_bitmap, idx) || ssycb->svchdr == no_support ) {
		ercd = E_NOEXS;
		goto error_exit;
	}

	/* Resource control block address */
	*p_resblk = (B*)ssycb->resblk + ROUND_RESBLKSZ(ssycb->resblksz) * idx;

    error_exit:
	END_CRITICAL_SECTION;

	return ercd;
}
