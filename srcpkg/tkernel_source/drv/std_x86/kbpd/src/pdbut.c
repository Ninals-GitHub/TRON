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
 *	pdbut.c
 *
 *       KB/PD device manager
 *       Enableware function for PD button (state machine) processing
 */

#include "kbpd.h"

#define	pdMode		(kpMgrInfo.pd.pdMode)
#define	pdInStat	(kpMgrInfo.statMach.pdInStat)

/*
 * generate PD move event
 */
LOCAL void makePdMoveEvent( InnerEvent *evt, PdInput *msg )
{
	evt->type |= IE_PDMOVE;
	evt->i.pd.stat.o = msg->stat;
	evt->i.pd.x = msg->xpos;
	evt->i.pd.y = msg->ypos;
}

/*
 * generate PD button down event
 */
LOCAL void makePdButDownEvent( PdButState *pdBut, InnerEvent *evt )
{
	evt->type |= ( pdBut->button == BK_MAIN )? IE_MBUTDOWN: IE_SBUTDOWN;
	evt->i.pd.stat = pdInStat;

	pdBut->butdown = TRUE;
}

/*
 * generate PD button up event
 */
LOCAL void makePdButUpEvent( PdButState *pdBut, InnerEvent *evt )
{
	evt->type |= ( pdBut->button == BK_MAIN )? IE_MBUTUP: IE_SBUTUP;
	evt->i.pd.stat = pdInStat;

	pdBut->butdown = FALSE;
}

/* ------------------------------------------------------------------------ */

/*
 * if button press, then TRUE
 */
LOCAL BOOL buttonPress( PdButState *pdBut )
{
	return ( (pdInStat.u.button & pdBut->button) != 0 )? TRUE: FALSE;
}

/*
 * release button forcefully
 */
LOCAL void forceButtonRelease( PdButState *pdBut )
{
	pdInStat.u.button &= ~pdBut->button;
}

/* ------------------------------------------------------------------------ */

/*
 * PD button : release status
 */
LOCAL BState bsRelease( PdButState *pdBut, InnerEvent *evt, ReceiveData *msg )
{
	if ( msg == NULL ) return BS_RELEASE;

	switch ( msg->head.cmd.cmd ) {
	  case INP_PD:
		if ( !buttonPress(pdBut) ) return BS_RELEASE;
		break;

	  default:
		return BS_RELEASE;
	}

	return BS_ONTIME;
}

/*
 * PD button: ON wait during effective time
 */
LOCAL BState bsOnTime( PdButState *pdBut, InnerEvent *evt, ReceiveData *msg )
{
	ER	err;

	if ( msg == NULL ) {
		if ( pdMode.ontime == 0 ) goto butdown;

                /* timeout setting for ON effective time */
		err = kpSetAlarm(&pdBut->alm, pdMode.ontime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("bsOnTime, SetAlarm err = %d\n", err));
			return BS_ERROR;
		}
		return BS_ONTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_PD:
		if ( !buttonPress(pdBut) ) return BS_RELEASE;
		return BS_ONTIME;

	  case PIC_TIMEOUT:
		if ( !kpChkAlarm(&pdBut->alm, &msg->tmout) ) return BS_ONTIME;
		break;

	  default:
		return BS_ONTIME;
	}

butdown:
        /* generate PD button down event */
	makePdButDownEvent(pdBut, evt);

	return BS_PRESS;
}

/*
 * PD button : press status
 */
LOCAL BState bsPress( PdButState *pdBut, InnerEvent *evt, ReceiveData *msg )
{
	ER	err;

	if ( msg == NULL ) {
reset:
                /* A PD for which timeout is ineffective, the timeout setting is unnecessary */
		if ( pdInStat.u.tmout == 0 ) return BS_PRESS;

                /* setting timeout */
		err = kpSetAlarm(&pdBut->alm, pdMode.timeout);
		if ( err != E_OK ) {
			DEBUG_PRINT(("bsPress, SetAlarm err = %d\n", err));
			return BS_ERROR;
		}
		return BS_PRESS;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_PD:
		if ( buttonPress(pdBut) ) goto reset;
		return BS_OFFTIME;

	  case PIC_TIMEOUT:
		if ( pdInStat.u.tmout == 0
		  || !kpChkAlarm(&pdBut->alm, &msg->tmout) ) return BS_PRESS;

                /* button release due to timeout */
		forceButtonRelease(pdBut);
		break;

	  default:
		return BS_PRESS;
	}

        /* generate PD button up event */
	makePdButUpEvent(pdBut, evt);

	return BS_INVTIME;
}

/*
 * PD button: OFF wait during effective time
 */
LOCAL BState bsOffTime( PdButState *pdBut, InnerEvent *evt, ReceiveData *msg )
{
	ER	err;

	if ( msg == NULL ) {
		if ( pdMode.offtime == 0 ) goto butup;

                /* time out setting for OFF effective time */
		err = kpSetAlarm(&pdBut->alm, pdMode.offtime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("bsOffTime, SetAlarm err = %d\n", err));
			return BS_ERROR;
		}
		return BS_OFFTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case INP_PD:
		if ( buttonPress(pdBut) ) return BS_PRESS;
		return BS_OFFTIME;

	  case PIC_TIMEOUT:
		if ( !kpChkAlarm(&pdBut->alm, &msg->tmout) ) return BS_OFFTIME;
		break;

	  default:
		return BS_OFFTIME;
	}

butup:
        /* generate PD button up event */
	makePdButUpEvent(pdBut, evt);

	return BS_INVTIME;
}

/*
 * PD button: wait during ineffective time period
 */
LOCAL BState bsInvTime( PdButState *pdBut, InnerEvent *evt, ReceiveData *msg )
{
	ER	err;

	if ( msg == NULL ) {
		if ( pdMode.invtime == 0 ) goto timeout;

                /* setting timeout for ineffective period */
		err = kpSetAlarm(&pdBut->alm, pdMode.invtime);
		if ( err != E_OK ) {
			DEBUG_PRINT(("bsInvTime, SetAlarm err = %d\n", err));
			return BS_ERROR;
		}
		return BS_INVTIME;
	}

	switch ( msg->head.cmd.cmd ) {
	  case PIC_TIMEOUT:
		if ( !kpChkAlarm(&pdBut->alm, &msg->tmout) ) return BS_INVTIME;
		break;

	  default:
		return BS_INVTIME;
	}

timeout:
	return ( buttonPress(pdBut) )? BS_ONTIME: BS_RELEASE;
}

/*
 * PD button: status reset
 */
LOCAL BState bsReset( PdButState *pdBut, InnerEvent *evt )
{
	if ( pdBut->butdown ) {
                /* generate button up event */
		makePdButUpEvent(pdBut, evt);
	}
	return BS_RELEASE;
}

/*
 * processing state for one PD button
 */
LOCAL void execPdButStateMachine( PdButState *pdBut,
					InnerEvent *evt, ReceiveData *msg )
{
	BState	prev;

	do {
		switch ( prev = pdBut->state ) {
		  case BS_RELEASE:
			pdBut->state = bsRelease(pdBut, evt, msg);
			break;
		  case BS_ONTIME:
			pdBut->state = bsOnTime(pdBut, evt, msg);
			break;
		  case BS_PRESS:
			pdBut->state = bsPress(pdBut, evt, msg);
			break;
		  case BS_OFFTIME:
			pdBut->state = bsOffTime(pdBut, evt, msg);
			break;
		  case BS_INVTIME:
			pdBut->state = bsInvTime(pdBut, evt, msg);
			break;

		  default:
			DEBUG_PRINT(("unknown BState(%d)\n", pdBut->state));
			pdBut->state = bsReset(pdBut, evt);
			return;
		}
		msg = NULL;
	} while ( pdBut->state != prev );
}

/*
 * state machine for a PD button
 */
EXPORT void kpExecPdButStateMachine( InnerEvent *evt, ReceiveData *msg, ButtonKind button )
{
	PdButState	*pdBut;
	W		i;

	if ( msg->head.cmd.err != DEV_OK ) {
                /* reset button status */
		for ( i = 0; i < NumOfPdBut; ++i ) {
			if ( (button & (1 << i)) == 0 ) continue;
			pdBut = &kpMgrInfo.statMach.pdBut[i];
			pdBut->state = bsReset(pdBut, evt);
		}
		return;
	}

	if ( msg->head.cmd.cmd == INP_PD ) {

                /* save PD attribute data */
		pdInStat.o = msg->pd.stat;

		if ( msg->pd.stat.inv == 0 ) {
                        /* enable PD movement */
			makePdMoveEvent(evt, &msg->pd);
		}
	}

        /* execute the state machine */
	for ( i = 0; i < NumOfPdBut; ++i ) {
		if ( (button & (1 << i)) == 0 ) continue;
		pdBut = &kpMgrInfo.statMach.pdBut[i];
		execPdButStateMachine(pdBut, evt, msg);
	}
}
