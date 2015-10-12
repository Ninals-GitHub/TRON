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
 *	pdsim.c
 *
 *       KB/PD device manager
 *       PD simulation
 */

#include "kbpd.h"

/*
 * start and stop PD simulation
 */
LOCAL void pdSimToggle( PdSimMode mode )
{
	if ( kpMgrInfo.kpState.stat.pdsim == PdSim_Off ) {
                /* start PD simulation */
		kpMgrInfo.kpState.stat.pdsim = mode;
	} else {
                /* end PD simulation */
		kpMgrInfo.kpState.stat.pdsim = PdSim_Off;
	}

        /* notify PD event */
	kpNotifyPdEvent(DE_PDSTATE, 0);
}

/*
 * main button press
 */
LOCAL void simButPress( void )
{
        /* main button press */
	kpMgrInfo.kpState.stat.main = 1;

        /* notify PD event */
	kpNotifyPdEvent(DE_PDBUT, 0);
}

/*
 * main button release
 */
LOCAL void simButRelease( void )
{
        /* main button release */
	kpMgrInfo.kpState.stat.main = 0;

        /* notify PD event */
	kpNotifyPdEvent(DE_PDBUT, 0);
}

/*
 * main button click presse
 */
LOCAL void simButClickPress( void )
{
        /* press */
	kpMgrInfo.kpState.stat.main = 1;
	kpNotifyPdEvent(DE_PDBUT, 0);

        /* release */
	kpMgrInfo.kpState.stat.main = 0;
	kpNotifyPdEvent(DE_PDBUT, 0);

        /* press */
	kpMgrInfo.kpState.stat.main = 1;
	kpNotifyPdEvent(DE_PDBUT, 0);
}

/*
 * PD displacement
 */
LOCAL W simMoveUnit( BOOL start )
{
	const W	N = 10;
	W	n;

	if ( start ) {
                /* initialize repeat count */
		kpMgrInfo.pdSimState.rep = 0;
	}

	n = kpMgrInfo.pdSimState.rep;
	if ( n < 20 ) kpMgrInfo.pdSimState.rep++;

	return ( kpMgrInfo.pd.pdSimSpeed < N )?
		(n / (N - kpMgrInfo.pd.pdSimSpeed + 1) + 1):
		(n * (kpMgrInfo.pd.pdSimSpeed - N + 1) + 1);
}

/*
 * move PD
 */
LOCAL void simMove( BOOL start )
{
	W	unit = simMoveUnit(start);
	BOOL	move;

        /* relative movement */
	move = kpMovePointer(
		kpMgrInfo.kpState.xpos + kpMgrInfo.pdSimState.x * unit,
		kpMgrInfo.kpState.ypos + kpMgrInfo.pdSimState.y * unit);

	if ( move ) {
                /* notify PD movement event */
		kpNotifyPdEvent(DE_PDMOVE, 0);
	}
}

/*
 * start PD movement
 */
LOCAL void simMoveStart( UH code, TMO *tmout )
{
        /* set movement direction */
	switch ( code ) {
	  case KC_CC_U:		/* main CC UP */
		kpMgrInfo.pdSimState.y = -1;
		break;
	  case KC_CC_D:		/* main CC DOWN */
		kpMgrInfo.pdSimState.y = +1;
		break;
	  case KC_CC_R:		/* main CC RIGHT */
		kpMgrInfo.pdSimState.x = +1;
		break;
	  case KC_CC_L:		/* main CC LEFT */
		kpMgrInfo.pdSimState.x = -1;
		break;
	  default:
		return;
	}

        /* period for location update */
	*tmout = simStartTime();

        /* move */
	simMove(TRUE);
}

/*
 * stop PD movement
 */
LOCAL void simMoveStop( UH code, TMO *tmout )
{
	switch ( code ) {
	  case KC_CC_U:		/* main CC UP */
	  case KC_CC_D:		/* main CC DOWN */
		kpMgrInfo.pdSimState.y = 0;
		if ( kpMgrInfo.pdSimState.x == 0 ) *tmout = TMO_FEVR;
		break;
	  case KC_CC_R:		/* main CC RIGHT */
	  case KC_CC_L:		/* main CC LEFT */
		kpMgrInfo.pdSimState.x = 0;
		if ( kpMgrInfo.pdSimState.y == 0 ) *tmout = TMO_FEVR;
		break;
	  default:
		kpMgrInfo.pdSimState.x = 0;
		kpMgrInfo.pdSimState.y = 0;
		*tmout = TMO_FEVR;
	}
}

/*
 * check whether PD simulation is in effect
 *       if PD simulation is on, return TRUE
 */
Inline BOOL isPdSimMode( void )
{
	return ( kpMgrInfo.kpState.stat.pdsim != 0 && !kpMgrInfo.pd.pdSimInh );
}

/*
 * check whether we can enter PD simulation mode
 *       if entering PD simulation mode is not permitted, return TRUE
 */
Inline BOOL isPdSimBlock( void )
{
	return ( kpMgrInfo.pd.pdSimSpeed == 0 || kpMgrInfo.pd.pdSimInh );
}

/*
 * check for key to enter PD simulation mode
 */
LOCAL PdSimMode isPdSimModeKey( InnerEvent *evt, UH code )
{
	static /*const*/ KpMetaBut meta = {{0}};	/* no shift status */

	if ( kpMgrInfo.kpState.stat.rsh == 0
	  || kpMgrInfo.kpState.stat.lsh == 0 ) return 0;

	if ( evt->i.key.keytop.u.tenkey != 0 ) {
		if ( isMainCC(code) ) return PdSim_TenKey;
		code = kpToKeycode(evt->i.key.keytop, &meta);
		if ( isMainCC(code) ) return PdSim_TenKey;
		return 0;
	}

	if ( isSubCC(code)
	  || code == KC_PG_U || code == KC_PG_D
	  || code == KC_PGUP || code == KC_PGDN
	  || code == KC_HOME || code == KC_END ) {
		return PdSim_MainBut;
	}

	if ( isMainCC(code) ) return PdSim_Std;

	return 0;
}

/*
 * key down
 *       if keys are related to PD simulation, do the processing, and return TRUE
 */
EXPORT BOOL kpExecPdSimKeyDown( InnerEvent *evt, UH code, TMO *tmout )
{
	PdSimMode	mode;

	if ( isPdSimBlock() ) return FALSE; /* PD simulation is prohibited */

	mode = isPdSimModeKey(evt, code);
	if ( mode != 0 ) {
                /* enter or exit PD simulation mode */
		pdSimToggle(mode);
		return TRUE;
	}

	if ( !isPdSimMode() ) {
                /* PD simulation mode is not on */
		return FALSE;
	}

	mode = kpMgrInfo.kpState.stat.pdsim;

	if ( mode == PdSim_TenKey ) {
		if ( evt->i.key.keytop.u.tenkey == 0
		  && !isSubCC(code) ) return FALSE;
	}

	switch ( code ) {
	  case KC_PGUP:		/* PageUp */
	  case KC_PG_U:		/* PageUp */
	  case KC_SC_R:		/* sub CC key RIGHT */
                /* main button off */
		simButRelease();
		break;

	  case KC_PGDN:		/* PageDown */
	  case KC_PG_D:		/* PageDown */
	  case KC_SC_L:		/* sub CC key LEFT */
                /* main button on */
	  case KC_END:		/* End */
	  case KC_SC_D:		/* sub CC key DOWN */
                /* main button press */
		simButPress();
		break;

	  case KC_HOME:		/* Home */
	  case KC_SC_U:		/* sub CC key UP */
                /* main button click presse */
		simButClickPress();
		break;

	  case KC_CC_U:		/* main CC UP */
	  case KC_CC_D:		/* main CC DOWN */
	  case KC_CC_R:		/* main CC RIGHT */
	  case KC_CC_L:		/* main CC LEFT */
		if ( mode == PdSim_MainBut ) return FALSE;
                /* start PD movement */
		simMoveStart(code, tmout);
		break;

	  default:
                /* keys unrelated to PD simulation */
		return FALSE;
	}

	return TRUE;
}

/*
 * key up
 *       if keys are related to PD simulation, do the processing, and return TRUE
 */
EXPORT BOOL kpExecPdSimKeyUp( InnerEvent *evt, UH code, TMO *tmout )
{
	PdSimMode	mode;

	if ( isPdSimBlock() ) return FALSE; /* PD simulation is prohibited */

	if ( isPdSimModeKey(evt, code) != 0 ) {
                /* ON / OFF PD simulation mode */
		return TRUE;
	}

	if ( !isPdSimMode() ) {
                /* PD simulation mode is not on */
		return FALSE;
	}

	mode = kpMgrInfo.kpState.stat.pdsim;

	if ( mode == PdSim_TenKey ) {
		if ( evt->i.key.keytop.u.tenkey == 0
		  && !isSubCC(code) ) return FALSE;
	}

	switch ( code ) {
	  case KC_PGUP:		/* PageUp */
	  case KC_PG_U:		/* PageUp */
	  case KC_SC_R:		/* sub CC key RIGHT */
	  case KC_PGDN:		/* PageDown */
	  case KC_PG_D:		/* PageDown */
	  case KC_SC_L:		/* sub CC key LEFT */
                /* keep the state (do nothing) */
		break;

	  case KC_END:		/* End */
	  case KC_SC_D:		/* sub CC key DOWN */
	  case KC_HOME:		/* Home */
	  case KC_SC_U:		/* sub CC key UP */
                /* main button release */
		simButRelease();
		break;

	  case KC_CC_U:		/* main CC UP */
	  case KC_CC_D:		/* main CC DOWN */
	  case KC_CC_R:		/* main CC RIGHT */
	  case KC_CC_L:		/* main CC LEFT */
		if ( mode == PdSim_MainBut ) return FALSE;
                /* stop PD movement */
		simMoveStop(code, tmout);
		break;

	  default:
                /* keys unrelated to PD simulation */
		return FALSE;
	}

	return TRUE;
}

/*
 * repeat
 */
EXPORT void kpExecPdSimRepeat( TMO *tmout )
{
	if ( isPdSimMode() ) {
                /* location update period */
		*tmout = simIntervalTime();

                /* move */
		simMove(FALSE);
	} else {
                /* halt PD simulation */
		simMoveStop(0, tmout);
	}
}
