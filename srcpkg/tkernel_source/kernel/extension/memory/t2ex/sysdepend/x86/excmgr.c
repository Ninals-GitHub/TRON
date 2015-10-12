/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/08.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	excmgr.c (x86)
 *	T2EX: system exception manager (x86)
 */

#include <typedef.h>
#include <machine.h>
#include <tk/sysdef.h>
#include "sysmgr.h"
#include "segmgr.h"
#include "excmgr.h"
#include "memdef.h"
#include <sys/sysinfo.h>
#include <sys/rominfo.h>

/*
 * Get/Set fault status registers (DFSR/IFSR)
 */
Inline UW getDFSR( void )
{
	UW	dfsr = 0;;
	//Asm("mrc p15, 0, %0, cr5, c0, 0": "=r"(dfsr));
	return dfsr;
}
Inline void setDFSR( UW dfsr )
{
	//Asm("mcr p15, 0, %0, cr5, c0, 0":: "r"(dfsr));
}
Inline UW getIFSR( void )
{
	UW	ifsr = 0;
	//Asm("mrc p15, 0, %0, cr5, c0, 1": "=r"(ifsr));
	return ifsr;
}
Inline void setIFSR( UW ifsr )
{
	//Asm("mcr p15, 0, %0, cr5, c0, 1":: "r"(ifsr));
}

/*
 * Get/Set fault address registers (DFAR/IFAR)
 */
Inline UW getDFAR( void )
{
	UW	dfar = 0;
	//Asm("mrc p15, 0, %0, cr6, c0, 0": "=r"(dfar));
	return dfar;
}
Inline void setDFAR( UW dfar )
{
	//Asm("mcr p15, 0, %0, cr6, c0, 0":: "r"(dfar));
}
Inline UW getIFAR( void )
{
	UW	ifar = 0;
	//Asm("mrc p15, 0, %0, cr6, c0, 2": "=r"(ifar));
	return ifar;
}
Inline void setIFAR( UW ifar )
{
	//Asm("mcr p15, 0, %0, cr6, c0, 2":: "r"(ifar));
}

/*
 * Returns true if the exception is caused in task-independent portion
 */
Inline BOOL TaskIndependent( void )
{
	return ( SCInfo.taskindp > 1 )? TRUE: FALSE;
}

/* ------------------------------------------------------------------------ */

EXPORT	FP		DefaultHandlerEntry;	/* Default handler set by T-Monitor */

#if T2EX_MM_USE_TASKEXCEPTION

/* Exception message buffer */
LOCAL	MemFaultInfo	*MemFault_buf;
LOCAL	UINT		MaxMemFault, MemFault_top, MemFault_cnt;

LOCAL	ID		SysExcTskID;		/* System exception processing task */

/*
 * Sends an exception message
 *	Called with interrupt disabled.
 */
LOCAL ER sendMemFault( MemFaultInfo *msg )
{
	UW	pos;

	if ( MemFault_cnt >= MaxMemFault ) return E_LIMIT;

	pos = (MemFault_top + MemFault_cnt) % MaxMemFault;
	MemFault_buf[pos] = *msg;

	if ( MemFault_cnt++ == 0 ) tk_wup_tsk(SysExcTskID);

	return E_OK;
}

/*
 * Receives an exception message
 */
LOCAL void recvMemFault( MemFaultInfo *msg )
{
	UINT	imask;

	for ( ;; ) {
		DI(imask);
		if ( MemFault_cnt > 0 ) {
			*msg = MemFault_buf[MemFault_top];
			if ( ++MemFault_top >= MaxMemFault ) MemFault_top = 0;
			MemFault_cnt--;
			break;
		}
		EI(imask);

		tk_slp_tsk(TMO_FEVR);
	}
	EI(imask);
}

/*
 * System exception processing task
 */
LOCAL void SysExcTsk( void )
{
	MemFaultInfo	excmsg;
	
	for ( ;; ) {
		/* Receive a system exception message */
		recvMemFault(&excmsg);

		/* Notify the system exception to the task that caused an exception */
		TaskMemFaultHdr(&excmsg);
	}
}

#endif /* T2EX_MM_USE_TASKEXCEPTION */

/*
 * System exception handler
 */
EXPORT void SysExcHdr( W vecno, ExcStack *sp )
{
	ER	ercd;
	MemFaultInfo	excmsg;

	memset(&excmsg, 0, sizeof(excmsg));

	excmsg.tskid   = tk_get_tid();
	excmsg.vecno   = vecno;
	excmsg.sp      = sp;
	if ( vecno == EIT_IABORT ) {
		excmsg.excinfo = getIFSR();
		excmsg.excaddr = (void*)getIFAR();
	} else {
		excmsg.excinfo = getDFSR();
		excmsg.excaddr = (void*)getDFAR();
	}

#if T2EX_MM_USE_TASKEXCEPTION
	if ( excmsg.tskid <= 0 ) goto rawhdr;

	/* If the interrupt is enabled at the point of interrupt, 
	   allow overlapping interrupts to happen */
	if ( (excmsg.sp->spsr & (PSR_A|PSR_I|PSR_F)) == 0 ) EI(0);

	/* If the interrupt has occurred while the interrupt is disabled,
	   while in non-user mode, treat is as a system fault */
	if ( (excmsg.sp->spsr & (PSR_A|PSR_I|PSR_F)) != 0
	     || (excmsg.sp->spsr & PSR_M(31)) != PSR_USR ) goto rawhdr;

	/* Suspend the target task */
	ercd = tk_sus_tsk(excmsg.tskid);
	if ( ercd < E_OK ) goto rawhdr;

	/* Notify the interrupt to the system exception processing task */
	ercd = sendMemFault(&excmsg);
	if ( ercd < E_OK ) goto rawhdr;

	return;
#endif /* T2EX_MM_USE_TASKEXCEPTION */

rawhdr:
	RawMemFaultHdr(&excmsg);
}

/* ------------------------------------------------------------------------ */

/*
 * Page fault handler
 *	EIT_IABORT	prefetch abort
 *	EIT_DABORT	data abort
 */
EXPORT W PageFaultHdr( UW vecno, ExcStack *sp )
{
	UW	excinfo, excaddr, dfsr;

	if ( vecno == EIT_IABORT ) {
		excinfo = getIFSR() & ~FSR_WnR;
		excaddr = getIFAR();
	} else {
		dfsr = getDFSR();
		excinfo = ( (dfsr & FSR_TypeMaskAll) == FSR_ICacheM )?
					getIFSR() & ~FSR_WnR: dfsr;
		excaddr = getDFAR();
	}
#ifdef DEBUG
	TM_DEBUG_PRINT(("PageFaultHdr vecno=%d "
			 "excinfo=0x%08x excaddr=0x%08x\n",
			 vecno, excinfo, excaddr));
#endif

	disint(); /* Disable interrupt */
	if ( vecno == EIT_IABORT ) {
		setIFSR(excinfo);
		setIFAR(excaddr);
	} else {
		if ( (dfsr & FSR_TypeMaskAll) == FSR_ICacheM )
			setIFSR(excinfo);
		setDFSR(dfsr);
		setDFAR(excaddr);
	}

	SysExcHdr(vecno, sp);
	return 0;
}

/* ------------------------------------------------------------------------ */

/*
 * Registers/Unregisters system exception handler
 *	regist = TRUE	register
 *	regist = FALSE	unregister
 */
EXPORT void RegistSysExcHdr( BOOL regist )
{
	T_DINT	dint;

	dint.intatr = TA_HLNG;
	dint.inthdr = SysExcHdr;

	tk_def_int(EIT_DEFAULT, ( regist )? &dint: NULL);
}

/*
 * Register page fault handlers
 */
LOCAL ER RegistPageFaultHdr( void )
{
IMPORT	void	asmIAbortHdr( void );
IMPORT	void	asmDAbortHdr( void );
	T_DINT	dint;
	ER	er;

	/* Save default interrupt handler set by T-Monitor */
	//DefaultHandlerEntry = SCArea->intvec[EIT_DEFAULT];

	/* Define prefetch/data abort handlers */
	dint.intatr = TA_ASM;
	dint.inthdr = asmIAbortHdr;
	er = tk_def_int(EIT_IABORT, &dint);
	if ( er < E_OK ) goto err_ret;
	dint.inthdr = asmDAbortHdr;
	er = tk_def_int(EIT_DABORT, &dint);
	if ( er < E_OK ) goto err_ret;

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("RegistPageFaultHdr ercd = %d\n", er));
#endif
	return er;
}

/*
 * Initialize system exception manager
 *	Must be called before the logical space is set (by segment manager)
 */
EXPORT ER init_excmgr( void )
{
#if T2EX_MM_USE_TASKEXCEPTION
	struct boot_info *info = getBootInfo();
	unsigned long laddr;
	ER	ercd;

	/* Maximum number of simultaneous page faults allowed */
	ercd = _tk_get_cfn(SCTAG_MAXPAGEFAULT, (INT*)&MaxMemFault, 1);
	if ( ercd <= 0 || MaxMemFault <= 0 ) 
		MaxMemFault = 64;

	/* Allocate exception message buffer */
	MemFault_buf =(MemFaultInfo*)((info->lowmem_top + 0x3) & ~0x3);
	
	info->lowmem_top = MemFault_buf + MaxMemFault;
#endif

	return E_OK;
}

/*
 * Start system exception manager
 */
EXPORT ER start_excmgr( void )
{
	ER	ercd;
#if T2EX_MM_USE_TASKEXCEPTION
	T_CTSK	ctsk;

	/* Start system exception processing task */
	SetOBJNAME(ctsk.exinf, "SExc");
	ctsk.tskatr  = TA_HLNG|TA_RNG0;
	ctsk.task    = SysExcTsk;
	ctsk.itskpri = 1;
	ctsk.stksz   = 2048;
	ctsk.dsname[0] = 'e';
	ctsk.dsname[1] = 'x';
	ctsk.dsname[2] = 'c';
	ctsk.dsname[3] = 'm';
	ctsk.dsname[4] = '\0';

	ercd = tk_cre_tsk(&ctsk);
	if ( ercd < E_OK ) goto err_ret;

	ercd = tk_sta_tsk(SysExcTskID = ercd, 0);
	if ( ercd < E_OK ) goto err_ret;
#endif

	/* Register page fault handlers */
	ercd = RegistPageFaultHdr();
	if ( ercd < E_OK ) goto err_ret;

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("start_excmgr ercd = %d\n", ercd));
#endif
	return ercd;
}

/*
 * Finish system exception manager
 */
EXPORT ER finish_excmgr( void )
{
	RegistSysExcHdr(FALSE);
	return E_OK;
}
