/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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
 *	exchdr.c (T2EX)
 *	T2EX: system exception handler (em1d)
 */

#include <typedef.h>
#include <machine.h>
#include <tk/sysdef.h>
#include <tk/syscall.h>
#include <tk/errno.h>
#include <tm/tmonitor.h>
#include <sys/misc.h>
#include "excmgr.h"

/*
 * Show task register values
 */
LOCAL void print_regs( ID tid )
{
	T_REGS	g;
	T_EIT	e;
	T_CREGS	c;
	ER	er;

	er = tk_get_reg(tid, &g, &e, &c);
	if ( er < E_OK ) {
		tm_printf("- register can not be read -\n");
		return;
	}

	PrintTaskRegister(tm_printf, &g, &e, &c);
}

/*
 * Raw memory fault handler
 *
 *	This example handler shows system down message, and enters T-Monitor.
 *	This function can be customized to implement the required exception 
 *	handling, like rebooting the whole device. 
 */
EXPORT void RawMemFaultHdr( MemFaultInfo* fault )
{
	/* Output message using T-Monitor */
	tm_printf("\n***** OS SYSTEM DOWN *****\n");
	tm_printf("INT:%d PC:%08x CPSR:%08x FSR:%08x FAR:%08x tid:%d\n",
		fault->vecno, fault->sp->lr, fault->sp->spsr,
		fault->excinfo, fault->excaddr, fault->tskid);
	tm_monitor();	/* Enter T-Monitor */
}

#if T2EX_MM_USE_TASKEXCEPTION

/*
 * Task memory fault handler
 *
 *	This example handler sends task exception of type 0 to the task that 
 *	caused the memory access violation. This function can be customized to 
 *	implement the required exception handling. 
 */
EXPORT void TaskMemFaultHdr( MemFaultInfo* fault )
{
	ER	ercd;

	/* Notify exception to the task */
	ercd = tk_ras_tex(fault->tskid, 0);
	if ( ercd < E_OK ) {
		/* Failed to notify the exception
		   (Most probably because the task exception handler is undefined) */
		goto system_down;
	}
	tk_frsm_tsk(fault->tskid);

	return;

system_down:
	/* Output message using T-Monitor */
	tm_printf("\n***** OS SYSTEM DOWN *****\n");
	tm_printf("INT:%d FSR:%08x FAR:%08x tid:%d\n",
		fault->vecno, fault->excinfo, fault->excaddr,
		fault->tskid);
	print_regs(fault->tskid);
	tm_monitor();	/* Enter T-Monitor */
}

#endif /* T2EX_MM_USE_TASKEXCEPTION */
