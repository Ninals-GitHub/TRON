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
 *	receive.c
 *
 *       KB/PD device manager
 * receiving task for data from I/O driver
 */

#include "kbpd.h"

/*
 * automatic setting of default keyboard ID
 */
LOCAL void kpSetAutoKeyID( UW kid )
{
	if ( kpMgrInfo.kb.defKeyID == 0 ) {
		kpMgrInfo.kb.defKeyID = 1; /* automatic */
		kpMgrInfo.kb.keyID = kid;
	}
}

/*
 * release the automatic configuraton of default keyboard ID
 */
LOCAL void kpClrAutoKeyID( UW kid )
{
	if ( kpMgrInfo.kb.defKeyID == 1
	  && kpMgrInfo.kb.keyID == kid ) {
		kpMgrInfo.kb.defKeyID = 0; /* unconfigured */
		kpMgrInfo.kb.keyID = KID_unknown;
	}
}

/*
 * register / delete the flags for commands
 *       When it is registered, its ID is returned. Otherwise InvalidID is returned.
 */
LOCAL ID registCmdFlag( FlgInput *msg )
{
	ID	schID, setID;
	int	i;

	if ( msg->stat.reg == 0 ) {
                /* unregister */
		schID = msg->flgid;
		setID = InvalidID;
		if ( msg->stat.kb != 0 ) kpClrAutoKeyID(msg->stat.kbid);
	} else {
                /* registration */
		schID = InvalidID;
		setID = msg->flgid;
		if ( msg->stat.kb != 0 ) kpSetAutoKeyID(msg->stat.kbid);
	}

	for ( i = 0; i < MaxCmd; ++i ) {
		if ( kpMgrInfo.cmdFlg[i] == schID ) {
			kpMgrInfo.cmdFlg[i] = setID;
			return setID;
		}
	}

	return InvalidID;
}

/* ------------------------------------------------------------------------ */

/*
 * notify PD data 2 event
 */
LOCAL ER kpNotifyPdInput2( PdInput2 *msg )
{
static	PdEvt2	evt = { {DE_PDEXT} };
	ER	err;

        /* ignore error data */
	if ( msg->stat.err != DEV_OK ) return E_OK;

        /* if wheel is not in effect, ignore */
	if ( !kpMgrInfo.pd.pdMode.attr.wheel ) return E_OK;

	evt.wheel = msg->wheel;

        /* notify PD extended event */
	err = kpNotifyEvent(&evt, sizeof(evt));
	if ( err < E_OK ) goto err_ret;

	return E_OK;

err_ret:
	DEBUG_PRINT(("kpNotifyPdInput2 err = %d\n", err));
	return err;
}

/* ------------------------------------------------------------------------ */

/*
 * entry to data receiving task
 */
EXPORT void kpDataReceiveTask( void )
{
	ReceiveData	*msg;
	InnerEvent	evt;
	BOOL		cont;
	TMO		tmout = TMO_FEVR;
	ID		flg;
	ER		ercd;

	Lock(&kpMgrInfo.lock);

        /* notify the meta key initial state to the event manager */
	kpNotifyMetaEvent();

	for ( ;; ) {	/* it never quits itself */

                /* receiving data */
		Unlock(&kpMgrInfo.lock);
		ercd = tk_rcv_mbx(kpMgrInfo.dataMbx, (T_MSG**)&msg, tmout);
		Lock(&kpMgrInfo.lock);
		if ( ercd != E_OK && ercd != E_TMOUT ) {
			DEBUG_PRINT(("tk_rcv_mbx err = %d\n", ercd));
			continue;
		}

		if ( ercd == E_TMOUT ) {
                        /* PD simulation / repeat processing */
			kpExecPdSimRepeat(&tmout);
			continue;
		}

		switch ( msg->head.cmd.cmd ) {
		  case INP_FLG:
                        /* register / delete the flags for commands */
			flg = registCmdFlag(&msg->flg);
			msg->head.cmd.read = 1;

			if ( flg != InvalidID ) {
                                /* configure device initial state */
				kpSendInitialDeviceCommand(flg);
			}
			break;

		  case INP_PD2:
                        /* PD data 2 */
			kpNotifyPdInput2(&msg->pd2);
			msg->head.cmd.read = 1;
			break;

		  default:
			do {
                                /* enableware processing (execute the state transition) */
				cont = kpExecStateMachine(&evt, msg);

                                /* internal event processing */
				kpInnerEventProcess(&evt, &tmout);
			} while ( cont );
			msg->head.cmd.read = 1;
		}
	}

	tk_exd_tsk();	/* should never reach here */
}

/*
 * start the receiving task for data from I/O driver
 */
EXPORT ER kpStartDataReceiveTask( PRI pri )
{
	T_CTSK	ctsk;
	ER	ercd;
	ER	err;

        /* set the initial state for the state transition */
	err = kpInitializeStateMachine();
	if ( err != E_OK ) {
		DEBUG_PRINT(("initialize state machine err = %d\n", err));
		return err;
	}

        /* start task */
	SetOBJNAME(ctsk.exinf, "kbpd");
	ctsk.tskatr  = TA_HLNG|TA_RNG0;
	ctsk.task    = kpDataReceiveTask;
	ctsk.itskpri = pri;
	ctsk.stksz   = DefaultStkSize;
	ercd = tk_cre_tsk(&ctsk);
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("create dataReceiveTask. ercd = %d\n", ercd));
		return ercd;
	}
	kpMgrInfo.dataReceiveTask = (ID)ercd;

	ercd = tk_sta_tsk(kpMgrInfo.dataReceiveTask, 0);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("start dataReceiveTask. ercd = %d\n", ercd));
		return ercd;
	}

	return E_OK;
}

/*
 * stop the receiving task for data from I/O driver
 */
EXPORT void kpStopDataReceiveTask( void )
{
	ID	id;

        /* stop the receiving task for data from I/O driver */
	if ( (id = kpMgrInfo.dataReceiveTask) != InvalidID ) {
		tk_ter_tsk(id);
		tk_del_tsk(id);
	}

        /* epilog processing of the state transition */
	kpFinishStateMachine();
}
