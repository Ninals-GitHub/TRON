/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */

/*
 *	monent.c
 *
 *       Entry to monitor
 */

#include "../cmdsvc.h"
#include <sys/sysinfo.h>

EXPORT W	bootFlag;	/* boot flag */

/*
 * monitor entry processing
 *       vec     exception vector number
 */
EXPORT void entMonitor( UW vec )
{
	UW	v;
	W	bpflg;
	UB	*cmd;
	UW	save_taskmode;

        /* update task mode */
	save_taskmode = SCInfo.taskmode;
	SCInfo.taskmode <<= 16;

        /* monitor entry processing (flushing cache, etc.) */
	enterMonitor(0);

        /* initialize address check data */
	initChkAddr();

        /* release breakpoint temporarily */
	bpflg = resetBreak(vec);

	bootFlag = 0;

	switch ( vec ) {
	  case EIT_DEFAULT:	/* reset */
		if ( bootSelect() == BS_AUTO ) {
                        /* automatic boot */
			if ( bootDisk(NULL) >= E_OK ) break; /* execute boot */
		}
                /* invoke monitor */
		dispTitle();			/* boot message */
		procCommand(NULL, 0);		/* command processing */
		break;

	  case SWI_MONITOR:	/* service call */
                /* Execute SVC: Parameters given are, r12(fn), r0, r1, r2, r3 */
		v = procSVC(getRegister(12), getRegister(0),
			getRegister(1), getRegister(2), getRegister(3));

                /* At boot time, r0 holds the boot parameter,
                   so don't change r0 */
		if ( bootFlag == 0 ) setRegister(0, v);	/* result is set to 0 */
		break;

	  case EIT_IDEBUG:	/* debug abort instruction */
	  case EIT_DDEBUG:	/* debug abort data */
		if ( procBreak(bpflg, &cmd) > 0 ) {
			procCommand(cmd, 0);
		}
		break;

	  default:	/* unsupported instruction exception, interrupt, or traps */
		if ( procEIT(vec) == 0 ) {
			stopTrace();		/* stop tracing */
			procCommand(NULL, 0);	/* command processing */
		}
	}

        /* set breakpoint */
	setupBreak();

        /* monitor exit processing (flushing cache, etc.) */
	leaveMonitor(getCP15(1, 0));

        /* restore task mode */
	SCInfo.taskmode = save_taskmode;

	return; /* returning leads to user program execution */
}
