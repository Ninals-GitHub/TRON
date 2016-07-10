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
 *	statmach.c
 *
 *       KB/PD device manager
 *       entry to the enableware function (state machine)
 */

#include "kbpd.h"

/*
 * send messages used only inside manager
 */
EXPORT ER kpSendPseudoMsg( T_MSG *msg )
{
#if 0
	//ER	ercd;
	//ercd = tk_snd_mbx(kpMgrInfo.dataMbx, msg);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("kpSendPseudoMsg err = %d\n", ercd));
		return ercd;
	}
#endif
	return E_OK;
}

/*
 * alarm handler
 */
LOCAL void alarmHandler( void *exinf )
{
	AlarmState	*alm = (AlarmState*)exinf;
	TimeoutMsg	*msg;
	SYSTIM		time;
	ER		ercd;
	int		i;

        /* look for message buffer free space */
	for ( i = 0; i < NumOfAlmMsgBuf; ++i ) {
		if ( alm->msg[i].stat.read != 0 ) break;
	}
	if ( i >= NumOfAlmMsgBuf ) {
                /* try notifying that the message buffer has become full. */
		for ( i = 0; i < NumOfAlmMsgBuf; ++i ) {
			alm->msg[i].stat.err = DEV_OVRRUN;
		}
		DEBUG_PRINT(("alarmHandler, over run error\n"));
		return;
	}

        /* set up message */
	msg = &alm->msg[i];
	msg->stat.read = 0;
	msg->stat.err = DEV_OK;

        /* set current time */
	ercd = tk_get_tim(&time);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("alarmHandler, tk_get_tim err = %d\n", ercd));
		msg->stat.err = DEV_SYSERR;
	}
	msg->time = time.lo;

        /* send timeout message */
	//kpSendPseudoMsg((T_MSG*)msg);
}

/*
 * set alarm
 */
EXPORT ER kpSetAlarm( AlarmState *alm, MSEC offsetTime )
{
	SYSTIM		time;
	ER		ercd;

        /* stop alarm (necessary) */
	ercd = tk_stp_alm(alm->almno);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("kpSetAlarm, tk_stp_alm err = %d\n", ercd));
		return ercd;
	}

        /* current time */
	ercd = tk_get_tim(&time);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("kpSetAlarm, tk_get_tim err = %d\n", ercd));
		return ercd;
	}
	alm->setTime = time.lo;

        /* start alarm handler */
	ercd = tk_sta_alm(alm->almno, offsetTime);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("kpSetAlarm, tk_sta_alm err = %d\n", ercd));
		return ercd;
	}

	return E_OK;
}

/*
 * check the validity of timeout messages
 *       if valid, return TRUE, and if invalid, return FALSE.
 */
EXPORT BOOL kpChkAlarm( AlarmState *alm, TimeoutMsg *msg )
{
	return ( alm->setTime <= msg->time )? TRUE: FALSE;
}

/*
 * initialize alarm management information
 */
LOCAL ER initAlmStat( AlarmState *alm, TimeoutKind kind )
{
	ER	ercd;
	int	i;
	T_CALM	calm;

        /* obtain alarm handler number */
	calm.exinf  = alm;
	calm.almatr = TA_HLNG;
	calm.almhdr = alarmHandler;
	ercd = tk_cre_alm(&calm);
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("initAlmStat, tk_cre_alm err = %d\n", ercd));
		return ercd;
	}
	alm->almno = (ID)ercd;

	for ( i = 0; i < NumOfAlmMsgBuf; ++i ) {
		alm->msg[i].stat.read = 1;
		alm->msg[i].stat.cmd = PIC_TIMEOUT;
		alm->msg[i].stat.kind = kind;
	}

	return E_OK;
}

/*
 * release alarm handler
 */
LOCAL ER finishAlarm( AlarmState *alm )
{
	ER	ercd;

	if ( alm->almno != InvalidHNO ) {

                /* cancel alarm handler */
		ercd  = tk_stp_alm(alm->almno);
		ercd |= tk_del_alm(alm->almno);
		if ( ercd != E_OK ) {
			DEBUG_PRINT(("finishAlarm, err = %d\n", ercd));
			return ercd;
		}
	}

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * initialize state machine
 */
EXPORT ER kpInitializeStateMachine( void )
{
	ER	err;
	int	i;

        /* key */
	QueInit(&StatMach.useq);
	QueInit(&StatMach.freq);
	for ( i = 0; i < MaxKey; ++i ) {
		err = initAlmStat(&StatMach.key[i].alm, KeyTmoutKind(i));
		if ( err != E_OK ) return err;
		QueInsert((QUEUE*)&StatMach.key[i], &StatMach.freq);
	}
	StatMach.spressMsg.cmd.read = 1;
	StatMach.spressMsg.cmd.cmd = PIC_SPRESS;
	StatMach.keyupMsg.cmd.read = 1;
	StatMach.keyupMsg.cmd.cmd = PIC_KEYUP;

        /* PD button */
	for ( i = 0; i < NumOfPdBut; ++i ) {
		StatMach.pdBut[i].state = BS_RELEASE;
		err = initAlmStat(&StatMach.pdBut[i].alm, PdButTmoutKind(i));
		if ( err != E_OK ) return err;
		StatMach.pdBut[i].button = PdButKind(i);
	}

	return E_OK;
}

/*
 * epilog processing of state machine
 */
EXPORT void kpFinishStateMachine( void )
{
	int	i;

        /* direct key input */
	for ( i = 0; i < MaxKey; ++i ) {
		finishAlarm(&StatMach.key[i].alm);
	}

        /* PD button */
	for ( i = 0; i < NumOfPdBut; ++i ) {
		finishAlarm(&StatMach.pdBut[i].alm);
	}
}

/*
 * release KeyState
 */
EXPORT void kpReleaseKey( KeyState *ks )
{
	QueRemove((QUEUE*)ks);
	QueInsert((QUEUE*)ks, &StatMach.freq);
}

/*
 * processing of INP_KEY
 */
LOCAL BOOL execINP_KEY( InnerEvent *evt, ReceiveData *msg )
{
	KeyTop		keytop;
	UW		kbsel;
	KeyState	*ks;

	if ( msg->head.cmd.err != DEV_OK ) {
                /* resetting key state due to error */
		kpAllResetKeyMap();

		if ( isQueEmpty(&StatMach.useq) ) return FALSE;

		kpExecKeyStateMachine((KeyState*)StatMach.useq.next, evt, msg);

		return !isQueEmpty(&StatMach.useq);
	}

	keytop = toKeyTop(&msg->kb);

        /* look for KeyState that matches keytop */
	ks = (KeyState*)QueSearch(&StatMach.useq, &StatMach.useq,
				keytop.w, offsetof(KeyState, keytop.w));

	kbsel = ( ks == (KeyState*)&StatMach.useq )?
		kpMgrInfo.kpState.stat.kbsel: ks->kbsel;

        /* change keymap */
	kpSetOrResetKeyMap(keytop, kbsel, msg->kb.stat.press);

	if ( ks == (KeyState*)&StatMach.useq ) {
                /* search empty KeyState */
		ks = (KeyState*)QueRemoveNext(&StatMach.freq);
		if ( ks == NULL ) return FALSE;

                /* initialize KeyState */
		ks->keytop = keytop;
		ks->kbsel = kbsel;
		ks->state = ( kpGetKeyKind(keytop) <= NormalKey )?
					KS_RELEASE: SS_RELEASE;
		memset(&ks->u, 0, sizeof(ks->u));
		QueInsert((QUEUE*)ks, &StatMach.useq);
	}

        /* execute the state machine */
	kpExecKeyStateMachine(ks, evt, msg);

	return FALSE;
}

/*
 * execute the state machine
 *       if repetition is necessary, return TRUE
 *       As long as TRUE is returned, evt needs to be processed, and calling must be repeated with the
 * the same set of parameters.
 *       (If multiple internal events, evt, are generated, this type of situation can arise.)
 */
EXPORT BOOL kpExecStateMachine( InnerEvent *evt, ReceiveData *msg )
{
	BOOL		cont = FALSE;
	W		kind;
	KeyState	*ks;

	evt->type = IE_NULL;

	switch ( msg->head.cmd.cmd ) {
	  case INP_PD:
		kpPdPreProcess(&msg->pd); /* preprocessing (left-handed mode, etc.) */
		kpExecPdButStateMachine(evt, msg, BK_ALL);
		break;

	  case INP_KEY:
		cont = execINP_KEY(evt, msg);
		break;

	  case PIC_TIMEOUT:
		switch ( kind = msg->tmout.stat.kind ) {
		  case TMO_MAIN:	/* main button */
			kpExecPdButStateMachine(evt, msg, BK_MAIN);
			break;
		  case TMO_SUB:		/* subbutton */
			kpExecPdButStateMachine(evt, msg, BK_SUB);
			break;
		  default:		/* key */
			kpExecKeyStateMachine(&StatMach.key[kind], evt, msg);
			break;
		}
		break;

	  case PIC_KEYUP:
		ks = (KeyState*)QueSearchH(&StatMach.useq, &StatMach.useq,
				KS_RELEASE, offsetof(KeyState, state));
		if ( ks != (KeyState*)&StatMach.useq ) {
			kpExecKeyStateMachine(ks, evt, msg);
		}
		break;

	  case PIC_SPRESS:
                /* this msg does not generate evt, and
                   we need to repeat as many times as necessary */
		for ( ks = (KeyState*)StatMach.useq.next;
					ks != (KeyState*)&StatMach.useq;
					ks = (KeyState*)ks->q.next ) {
			if ( ks->state == KS_CONTIME ) {
				kpExecKeyStateMachine(ks, evt, msg);
			}
		}
		break;

	  default:
		DEBUG_PRINT(("unknown cmd(%d)\n", msg->head.cmd.cmd));
		return FALSE;
	}

	return cont;
}

/*
 * request forced up of keys and buttons
 * (used for entering suspeneded state)
 */
EXPORT ER kpKeyAndButtonForceUp( void )
{
	//static KeyInput	keymsg;
	//static PdInput	pdmsg;
	//ER		err, error = E_OK;
	ER		error = E_OK;

#if 0
        /* key up request */
	keymsg.stat.read = 0;
	keymsg.stat.cmd  = INP_KEY;
	keymsg.stat.err  = DEV_RESET;
	//err = kpSendPseudoMsg((T_MSG*)&keymsg);
	if ( err < E_OK ) error = err;

        /* button up request */
	pdmsg.stat.read = 0;
	pdmsg.stat.cmd  = INP_PD;
	pdmsg.stat.err  = DEV_RESET;
	//err = kpSendPseudoMsg((T_MSG*)&pdmsg);
	if ( err < E_OK ) error = err;
#endif

DO_DEBUG(
	if ( error < E_OK )
		DEBUG_PRINT(("kpKeyAndButtonForceUp err = %d\n", error));
)
	return error;
}
