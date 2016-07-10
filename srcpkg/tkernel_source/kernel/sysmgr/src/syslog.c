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
 *	syslog.c (T-Kernel/SM)
 *	System Log Task
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/util.h>
#include <tm/tmonitor.h>
#include <sys/debug.h>
#include "syslog.h"

#if USE_SYSLOG_CONSIO
#include <sys/consio.h>
#endif

EXPORT	W	logtask_alive = 0;	/* Is log task running? */

#if USE_SYSLOG_CONSIO
LOCAL	W	log_msg_maxmsz;		/* Maximum length of log message */
LOCAL	ID	log_mbfid;		/* Message buffer ID for log */
#endif

/* Set Object Name in .exinf for DEBUG */
#define OBJNAME_SYSLOG	"SLog"		/* Object Name for syslog */

/*
 * Low level output using monitor
 */
LOCAL void sys_write( const char *s, int len )
{
	while ( len-- > 0 ) {
		tm_putchar(*s++);
	}
}

/*
 * Wait until log is completed.
 */
LOCAL ER svc_syslog_wait( void )
{
#if USE_SYSLOG_CONSIO
	T_RMBF	rmbf;
	W	i;
	ER	ercd;

	if ( logtask_alive ) {
		for ( i = 0; i < 50; i++ ) {
			tk_dly_tsk(100);
			ercd = tk_ref_mbf(log_mbfid, &rmbf);
			if ( ercd != E_OK || rmbf.wtsk > 0 ) {
				break;
			}
		}
	}
#endif

	return E_OK;
}

/*
 * Extension SVC handler for log
 */
EXPORT ER __syslog_send( const char *string, int len )
{
	ER	ercd;

	if ( string == NULL ) {
		return svc_syslog_wait();
	}

	if ( len <= 0 ) {
		return E_OK;
	}

#if USE_SYSLOG_CONSIO
	if ( logtask_alive ) {
		/* When log task is running, send message to log task */
		if ( len > log_msg_maxmsz ) {
			len = log_msg_maxmsz;
		}
		ercd = tk_snd_mbf(log_mbfid, (void*)string, len, TMO_POL);
		if ( ercd != E_OK ) {
			logtask_alive = 0;
		}
	}
#endif

	if ( !logtask_alive ) {
		/* When it is not running, use low level character
		   output routine directly. */
		sys_write(string, len);
		sys_write("\n", 1);
	}

	return E_OK;
}

#if USE_SYSLOG_CONSIO
/*
 * Log task
 */
LOCAL void log_task( INT logtask_port )
{
	static B	logtask_buf[MBF_LOG_MAXMSZ+1];
	INT		msgsz;
	ER		ercd;

	logtask_alive = 1;
	log_msg_maxmsz = MBF_LOG_MAXMSZ;

	for ( ;; ) {
		ercd = tk_rcv_mbf(log_mbfid, logtask_buf, TMO_FEVR);
		if ( ercd < E_OK ) {
			break;
		}
		msgsz = ercd;
		logtask_buf[msgsz++] = '\n';

		ercd = console_out(logtask_port, logtask_buf, (UW)msgsz);
		if ( ercd < E_OK ) {
			sys_write(logtask_buf, msgsz);
		}
	}

	logtask_alive = 0;
	tk_exd_tsk();
}
#endif

/*
 * syslog initialization
 */
EXPORT ER initialize_syslog( void )
{
#if USE_SYSLOG_CONSIO
	T_CMBF	cmbf;
	T_CTSK	ctsk;
	ID	tskid;
	ER	ercd;

	/* Generate message buffer */
	SetOBJNAME(cmbf.exinf, OBJNAME_SYSLOG);
	cmbf.mbfatr = TA_TFIFO | TA_NODISWAI;
	cmbf.bufsz  = MBF_LOG_BUFSZ;
	cmbf.maxmsz = MBF_LOG_MAXMSZ;
	ercd = tk_cre_mbf(&cmbf);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}
	log_mbfid = ercd;

	/* Temporarily lower the local task priority
	   so that the system log task executes initialization sequence. */

	tk_chg_pri(TSK_SELF, 10);

	/* Start log task */
	SetOBJNAME(ctsk.exinf, OBJNAME_SYSLOG);
	ctsk.tskatr  = TA_HLNG | TA_RNG0;
	ctsk.task    = log_task;
	ctsk.itskpri = 6;
	ctsk.stksz   = 512;
	ctsk.dsname[0] = 's';
	ctsk.dsname[1] = 'y';
	ctsk.dsname[2] = 's';
	ctsk.dsname[3] = 'l';
	ctsk.dsname[4] = NULL;
	ercd = tk_cre_tsk(&ctsk);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}
	tskid = ercd;
	ercd = tk_sta_tsk(tskid, CONSOLE_PORT);
	if ( ercd < E_OK ) {
		goto err_ret2;
	}

	/* Return local task priority */
	tk_chg_pri(TSK_SELF, TPRI_INI);

	return E_OK;

err_ret2:
	tk_del_tsk(tskid);
err_ret1:
	DEBUG_PRINT(("initialize_syslog ercd = %d\n", ercd));
	return ercd;
#else
	return E_OK;
#endif
}

/*
 * syslog finalization sequence
 */
EXPORT ER finish_syslog( void )
{
#if USE_SYSLOG_CONSIO
	ER	ercd;

	/* Delete message buffer
	   This also stops log task */
	ercd = tk_del_mbf(log_mbfid);

#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("finish_syslog ercd = %d\n", ercd));
	}
#endif
	return ercd;
#else
	return E_OK;
#endif
}
