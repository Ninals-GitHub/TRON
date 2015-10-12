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
 *	inkey.c
 *
 *       KB/PD device manager
 *       Enableware function for key input (state machine) processing
 */

#include "kbpd.h"

#define	keyMode		(kpMgrInfo.kb.keyMode)

/*
 * sending quasi-device message for shift press
 */
LOCAL void sendPseudoSPressMsg( void )
{
	CommonMsg	*msg = &kpMgrInfo.statMach.spressMsg;

	if ( msg->cmd.read == 0 ) {
                /* It was already send, and so unnecessary */
		return;
	}

	msg->cmd.read = 0;

        /* sending shift press message */
	//kpSendPseudoMsg((T_MSG*)msg);
}

/*
 * generate shift key event
 */
LOCAL void makeShiftKeyEvent( InnerEvent *evt, InnEvtType type,
						ShiftKeyKind skind )
{
	evt->type = type;
	evt->i.sft.kind = skind;

	if ( type == IE_S_PRESS ) {
                /* sending quasi-device message for shift press */
		//sendPseudoSPressMsg();
	}
}

#if 0	// unused function
/*
 * generate shift key error event
 */
LOCAL void makeShiftKeyErrorEvent( InnerEvent *evt )
{
	evt->type = IE_KEYERR;
	evt->i.key.keytop.w = InvalidKeytop;
}
#endif

/*
 * shift key : release status
 */
LOCAL KState ssRelease( KeyState *skey, InnerEvent *evt, ReceiveData *msg )
{
	W		keykind;
	SYSTIM		lnow;
	UW		now;
	ER		err;

	if ( msg == NULL ) return SS_RELEASE;

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press != 0 ) break;
		/* no break */
	  default:
		return SS_RELEASE;
	}

        /* current time */
	err = tk_get_tim(&lnow);
	if ( err != E_OK ) {
		DEBUG_PRINT(("ssRelease, tk_get_tim err = %d\n", err));
		return SS_ERROR;
	}
	now = lnow.lo;

        /* repeated press during the time interval for double click, key type is not changed. */
	if ( !(skey->u.s.pressCount == 1
		&& (now - skey->u.s.pressTime) <= keyMode.dclktime) ) {

                /* key type */
		keykind = kpGetKeyKind(skey->keytop);

		if ( keykind <= NormalKey ) {
                        /* enter the ordinary state machine for normal keys */
			memset(&skey->u.i, 0, sizeof(skey->u.i));
			return KS_RELEASE;
		}

		skey->u.s.skind = keykind;
	}

	return SS_ONTIME;
}

/*
 * shift key : ON wait during effective time
 */
LOCAL KState ssOnTime( KeyState *skey, InnerEvent *evt, ReceiveData *msg )
{
	SYSTIM		lnow;
	UW		now;
	ER		err;

	if ( msg == NULL ) {
		if ( keyMode.ontime == 0 ) goto spress;

                /* timeout setting for ON effective time */
		err = kpSetAlarm(&skey->alm, keyMode.ontime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("ssOnTime, SetAlarm err = %d\n", err));
			return SS_ERROR;
		}
		return SS_ONTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press == 0 ) return SS_RELEASE;
		return SS_ONTIME;

	  case PIC_TIMEOUT:
		if ( !kpChkAlarm(&skey->alm, &msg->tmout) ) return SS_ONTIME;
		break;

	  default:
		return SS_ONTIME;
	}

spress:
        /* current time */
	err = tk_get_tim(&lnow);
	if ( err != E_OK ) {
		DEBUG_PRINT(("ssOnTime, tk_get_tim err = %d\n", err));
		return SS_ERROR;
	}
	now = lnow.lo;

	if ( skey->u.s.pressCount == 1
	  && (now - skey->u.s.pressTime) <= keyMode.dclktime ) {

		skey->u.s.pressCount = 2;
	} else {
		skey->u.s.pressCount = 1;
		skey->u.s.pressTime = now;
	}

        /* shift press event generated */
	makeShiftKeyEvent(evt, IE_S_PRESS, skey->u.s.skind);

	return SS_PRESS;
}

/*
 * shift key: press status
 */
LOCAL KState ssPress( KeyState *skey, InnerEvent *evt, ReceiveData *msg )
{
	if ( msg == NULL ) return SS_PRESS;

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press != 0 ) return SS_PRESS;
		break;

	  default:
		return SS_PRESS;
	}

	return SS_OFFTIME;
}

/*
 * shift key: OFF wait during effective time
 */
LOCAL KState ssOffTime( KeyState *skey, InnerEvent *evt, ReceiveData *msg )
{
	InnEvtType	innEvtType;
	SYSTIM		lnow;
	UW		now;
	ER		err;

	if ( msg == NULL ) {
		if ( keyMode.offtime == 0 ) goto srelease;

                /* time out setting for OFF effective time */
		err = kpSetAlarm(&skey->alm, keyMode.offtime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("ssOffTime, SetAlarm err = %d\n", err));
			return SS_ERROR;
		}
		return SS_OFFTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press != 0 ) return SS_PRESS;
		return SS_OFFTIME;

	  case PIC_TIMEOUT:
		if ( !kpChkAlarm(&skey->alm, &msg->tmout) ) return SS_OFFTIME;
		break;

	  default:
		return SS_OFFTIME;
	}

srelease:
        /* current time */
	err = tk_get_tim(&lnow);
	if ( err != E_OK ) {
		DEBUG_PRINT(("ssOffTime, tk_get_tim err = %d\n", err));
		return SS_ERROR;
	}
	now = lnow.lo;

	switch ( skey->u.s.pressCount ) {
	  case 1:
		if ( (now - skey->u.s.pressTime) < keyMode.sclktime ) {
                        /* single click */
			innEvtType = IE_S_SCLK;
		} else {
			innEvtType = IE_S_REL;
		}
		break;

	  case 2:
		if ( (now - skey->u.s.pressTime) < keyMode.dclktime ) {
                        /* double click */
			innEvtType = IE_S_DCLK;
		} else {
			innEvtType = IE_S_REL;
		}
		skey->u.s.pressCount = 0;
		break;

	  default:
		innEvtType = IE_S_REL;
		skey->u.s.pressCount = 0;
	}

        /* event generated */
	makeShiftKeyEvent(evt, innEvtType, skey->u.s.skind);

	return SS_RELEASE;
}

/*
 * shift key: status reset
 */
LOCAL KState ssReset( KeyState *skey, InnerEvent *evt )
{
        /* generate shift release event */
	makeShiftKeyEvent(evt, IE_S_REL, skey->u.s.skind);

	kpReleaseKey(skey);

	return SS_RELEASE;
}

/*
 * cancel shift click
 *       clear the press count of the shift key specified by skind,
 *       and cancel the click or double click which was being monitored.
 */
LOCAL void kpCancelShiftClick( ShiftKeyKind skind )
{
	KeyState	*skey;

	for ( skey = (KeyState*)StatMach.useq.next;
				skey != (KeyState*)&StatMach.useq;
				skey = (KeyState*)skey->q.next ) {

		if ( skey->state >= SS_RELEASE
		  && (skey->u.s.skind & skind) != 0 ) {
                        /* cancel shift click */
			skey->u.s.pressCount = 0;

			if ( (skind &= ~skey->u.s.skind) == 0 ) break;
		}
	}
}

/* ------------------------------------------------------------------------ */

/*
 * send quasi-device message for key up notification
 */
LOCAL void sendPseudoKeyUpMsg( void )
{
	if ( StatMach.keyupMsg.cmd.read != 0 ) {
                /* send */
		StatMach.keyupMsg.cmd.read = 0;
		kpSendPseudoMsg((T_MSG*)&StatMach.keyupMsg);
	}
}

/*
 * generate key down event
 */
LOCAL void makeKeyDownEvent( KeyState *inkey, InnerEvent *evt )
{
	KpMetaBut	meta;

        /* meta key status when key down occurrs */
	meta.o = kpMgrInfo.kpState.stat;
	meta.u.shift |= inkey->u.i.meta.u.shift;

	evt->type = IE_KEYDOWN;
	evt->i.key.keytop = inkey->keytop;
	evt->i.key.meta = inkey->u.i.meta = meta;

	inkey->u.i.keydown = TRUE;
}

/*
 * generate key up event
 */
LOCAL void makeKeyUpEvent( KeyState *inkey, InnerEvent *evt )
{
	evt->type = IE_KEYUP;
	evt->i.key.keytop = inkey->keytop;
	evt->i.key.meta = inkey->u.i.meta;

	inkey->u.i.keydown = FALSE;
}

/*
 * generate key error event
 */
LOCAL void makeKeyErrorEvent( KeyState *inkey, InnerEvent *evt )
{
	evt->type = IE_KEYERR;
	evt->i.key.keytop = inkey->keytop;
	evt->i.key.meta = inkey->u.i.meta;

	inkey->u.i.keydown = FALSE;
}

/*
 * direct input key: release status
 */
LOCAL KState ksRelease( KeyState *inkey, InnerEvent *evt, ReceiveData *msg )
{
	W	keykind = kpGetKeyKind(inkey->keytop);

	if ( msg == NULL ) {
		if ( keykind == NormalKey ) {
			StatMach.keyPress--;

                        /* quasi-event for key up notification */
			sendPseudoKeyUpMsg();
		}
		kpReleaseKey(inkey);
		return KS_RELEASE;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press == 0 ) {
			kpReleaseKey(inkey);
			return KS_RELEASE;
		}
		break;

	  case PIC_KEYUP:  /* previous key was released */
		break;

	  default:
		return KS_RELEASE;
	}

        /* If ON effective or ineffective period is not zero, simultaneous pressing of multiple keys is
         * not allowed. However, main CC keys are exceptions.
	 */
	if ( StatMach.keyPress > 0 && keykind == NormalKey
	  && (keyMode.ontime > 0 || keyMode.invtime > 0) ) return KS_RELEASE;

	if ( keykind == NormalKey ) StatMach.keyPress++;
	return KS_ONTIME;
}

/*
 * direct input key: ON wait during effective time
 */
LOCAL KState ksOnTime( KeyState *inkey, InnerEvent *evt, ReceiveData *msg )
{
	ER	err;

	if ( msg == NULL ) {
		if ( keyMode.ontime == 0 ) return KS_CONTIME;

                /* timeout setting for ON effective time */
		err = kpSetAlarm(&inkey->alm, keyMode.ontime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("ksOnTime, SetAlarm err = %d\n", err));
			return KS_ERROR;
		}
		return KS_ONTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press == 0 ) return KS_RELEASE;
		break;

	  case PIC_TIMEOUT:
		if ( kpChkAlarm(&inkey->alm, &msg->tmout) ) return KS_CONTIME;
		break;

	  default:
		return KS_ONTIME;
	}

	return KS_ONTIME;
}

/*
 * direct input key: wait during simultaneous press period
 */
LOCAL KState ksConTime( KeyState *inkey, InnerEvent *evt, ReceiveData *msg )
{
	KState		next = KS_PRESS;
	ER		err;

	if ( msg == NULL ) {
                /* record shift key status */
		inkey->u.i.meta.u.shift = kpMgrInfo.spress;

                /* Shift key used for modifying direct input key has its shift click
                   cancelled */
		kpCancelShiftClick(kpMgrInfo.spress);

		if ( keyMode.contime == 0 ) goto keydown;

                /* timeout setting for simultaneous press period */
		err = kpSetAlarm(&inkey->alm, keyMode.contime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("ksConTime, SetAlarm err = %d\n", err));
			return KS_ERROR;
		}
		return KS_CONTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press != 0 ) return KS_CONTIME;
		next = KS_OFFTIME;
		break;

	  case PIC_SPRESS:
                /* record shift key status, and cancel shift click */
		inkey->u.i.meta.u.shift |= kpMgrInfo.spress;
		kpCancelShiftClick(kpMgrInfo.spress);
		return KS_CONTIME;

	  case PIC_TIMEOUT:
		if ( !kpChkAlarm(&inkey->alm, &msg->tmout) ) return KS_CONTIME;
		break;

	  default:
		return KS_CONTIME;
	}

keydown:
        /* generate key down event */
	makeKeyDownEvent(inkey, evt);

	return next;
}

/*
 * direct input key: press status
 */
LOCAL KState ksPress( KeyState *inkey, InnerEvent *evt, ReceiveData *msg )
{
	if ( msg == NULL ) return KS_PRESS;

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press != 0 ) return KS_PRESS;
		break;

	  default:
		return KS_PRESS;
	}

	return KS_OFFTIME;
}

/*
 * direct input key: OFF wait during effective time
 */
LOCAL KState ksOffTime( KeyState *inkey, InnerEvent *evt, ReceiveData *msg )
{
	ER	err;

	if ( msg == NULL ) {
                /* time out setting for OFF effective time (offtime == 0 allowed) */
		err = kpSetAlarm(&inkey->alm, keyMode.offtime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("ksOffTime, SetAlarm err = %d\n", err));
			return KS_ERROR;
		}
		return KS_OFFTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		if ( msg->kb.stat.press != 0 ) return KS_PRESS;
		return KS_OFFTIME;

	  case PIC_TIMEOUT:
		if ( !kpChkAlarm(&inkey->alm, &msg->tmout) ) return KS_OFFTIME;
		break;

	  default:
		return KS_OFFTIME;
	}

        /* generate key up event */
	makeKeyUpEvent(inkey, evt);

	return KS_INVTIME;
}

/*
 * direct input key: wait during ineffective time
 */
LOCAL KState ksInvTime( KeyState *inkey, InnerEvent *evt, ReceiveData *msg )
{
	ER	err;

	if ( msg == NULL ) {
		inkey->u.i.invpress = FALSE;

                /* timeout setting for inffective period (invtime == 0 allowed) */
		err = kpSetAlarm(&inkey->alm, keyMode.invtime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("ksInvTime, SetAlarm err = %d\n", err));
			return KS_ERROR;
		}
		return KS_INVTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_KEY:
		inkey->u.i.invpress = ( msg->kb.stat.press != 0 );
		return KS_INVTIME;

	  case PIC_TIMEOUT:
		if ( !kpChkAlarm(&inkey->alm, &msg->tmout) ) return KS_INVTIME;
		break;

	  default:
		return KS_INVTIME;
	}

	if ( inkey->u.i.invpress ) {
                /* same key is pushed again during ineffective time period */
		return KS_ONTIME;
	}

	return KS_RELEASE;
}

/*
 * direct input key: status reset
 */
LOCAL KState ksReset( KeyState *inkey, InnerEvent *evt )
{
	if ( inkey->u.i.keydown ) {
                /* generate key errror event */
		makeKeyErrorEvent(inkey, evt);
	}

	if ( inkey->state > KS_RELEASE ) StatMach.keyPress--;
	kpReleaseKey(inkey);

	return KS_RELEASE;
}

/* ------------------------------------------------------------------------ */

/*
 * key state machine
 */
EXPORT void kpExecKeyStateMachine( KeyState *ks, InnerEvent *evt, ReceiveData *msg )
{
	KState	prev;

	if ( msg->head.cmd.err != DEV_OK ) {
		if ( ks->state < SS_RELEASE ) {
                        /* reset key status */
			ks->state = ksReset(ks, evt);
		} else {
                        /* reset shift status */
			ks->state = ssReset(ks, evt);
		}
		return;
	}

	do {
		switch ( prev = ks->state ) {
		  case KS_RELEASE:
			ks->state = ksRelease(ks, evt, msg);
			break;
		  case KS_ONTIME:
			ks->state = ksOnTime(ks, evt, msg);
			break;
		  case KS_CONTIME:
			ks->state = ksConTime(ks, evt, msg);
			break;
		  case KS_PRESS:
			ks->state = ksPress(ks, evt, msg);
			break;
		  case KS_OFFTIME:
			ks->state = ksOffTime(ks, evt, msg);
			break;
		  case KS_INVTIME:
			ks->state = ksInvTime(ks, evt, msg);
			break;

		  case SS_RELEASE:
			ks->state = ssRelease(ks, evt, msg);
			break;
		  case SS_ONTIME:
			ks->state = ssOnTime(ks, evt, msg);
			break;
		  case SS_PRESS:
			ks->state = ssPress(ks, evt, msg);
			break;
		  case SS_OFFTIME:
			ks->state = ssOffTime(ks, evt, msg);
			break;

		  default:
			DEBUG_PRINT(("unknown KState(%d)\n", ks->state));
			return;
		}
		if ( (prev < SS_RELEASE) == (ks->state < SS_RELEASE) ) {
			msg = NULL;
		}
	} while ( ks->state != prev );
}
