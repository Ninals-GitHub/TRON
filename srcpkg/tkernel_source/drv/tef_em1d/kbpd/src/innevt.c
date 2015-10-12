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
 *	innevt.c
 *
 *       KB/PD device manager
 *       internal event processing
 */

#include "kbpd.h"

/*
 * lock key down
 *      if a code is processed as lock key, then return TRUE
 */
LOCAL BOOL lockKeyDown( UH code )
{
	InputMode	mode;
	UW		meta;
	BOOL		chgLED;
	union {
		MetaBut	stat;
		UW	uw;
	} u;

	mode = kpMgrInfo.kpState.stat.mode;
	u.stat = kpMgrInfo.kpState.stat;
	meta = u.uw;

	switch ( code ) {
	  case KC_EIJI:	/* English <-> Japanese swap */
		mode ^= AlphaMode;
		break;
	  case KC_CAPN:	/* Hiragana <-> Katakana swap */
		mode ^= KataMode;
		break;
	  case KC_JPN0_Z: /* Japanese Hiragana & Zenkaku */
		meta &= ~ES_HAN;
	  case KC_JPN0:	/* Japanese Hiragana */
		mode = HiraMode;
		break;
	  case KC_JPN1_Z: /* Japanese Katakana & Zenkaku */
		meta &= ~ES_HAN;
	  case KC_JPN1:	/* Japanese Katanaka */
		mode = KataMode;
		break;
	  case KC_ENG0_H: /* English & Hankaku */
		meta |= ES_HAN;
	  case KC_ENG0:	/* English */
		mode = AlphaMode;
		break;
	  case KC_ENG1_H: /* English CAPS & Hanaku */
		meta |= ES_HAN;
	  case KC_ENG1:	/* English CAPS */
		mode = CapsMode;
		break;
	  case KC_ENGALT:	/* -> English <--> English CAPS */
		if (mode & AlphaMode) mode ^= KataMode;
		else	mode = AlphaMode;
		break;
	  case KC_JPNALT:	/* ->Hiragana<-->Katakana */
		if (mode & AlphaMode) mode = HiraMode;
		else	mode ^= KataMode;
		break;

	  case KC_KBSEL:	/* Kana <--> Roman Input */
		meta ^= ES_KBSEL;
		break;
	  case KC_HAN:		/* Zenkaku <--> Hankaku */
		meta ^= ES_HAN;
		break;
	  default:
                /* not a lock key */
		return FALSE;
	}

	if ( (chgLED = ( mode != kpMgrInfo.kpState.stat.mode ))
	  || meta != u.uw ) {

                /* change input mode */
		u.uw = meta;
		kpMgrInfo.kpState.stat = u.stat;
		kpMgrInfo.kpState.stat.mode = mode;

		if ( chgLED ) {
                        /* change keyboard LED */
			kpChangeKbInputMode(mode);
		}

                /* notify meta key status change event */
		kpNotifyMetaEvent();
	}

	return TRUE;
}

/*
 * key down
 */
LOCAL void ieKeyDown( InnerEvent *evt, TMO *tmout )
{
	KeyEvt		keyEvt;
	UH		code;

        /* convert to character code */
	code = kpToKeycode(evt->i.key.keytop, &evt->i.key.meta);

        /* lock key processing */
	if ( lockKeyDown(code) ) {
                /* process only lock key and finish */
		return;
	}

        /* PD simulation processing */
	if ( kpExecPdSimKeyDown(evt, code, tmout) ) {
                /* process PD simulation only and finish */
		return;
	}

        /* notify key down event */
	keyEvt.h.evttyp = DE_KEYDOWN;
	keyEvt.keytop = kpKeyTopCode(evt->i.key.keytop,
				evt->i.key.meta.o.kbsel);
	keyEvt.code = code;
	keyEvt.stat = evt->i.key.meta.o;
	kpNotifyEvent(&keyEvt, sizeof(keyEvt));
}

/*
 * key up
 */
LOCAL void ieKeyUp( InnerEvent *evt, TMO *tmout )
{
	KeyEvt		keyEvt;
	UH		code;

        /* convert to character code */
	code = kpToKeycode(evt->i.key.keytop, &evt->i.key.meta);

        /* lock key needs no processing */
	if ( isMetaKey(code) ) goto skip;

        /* PD simulation processing */
	if ( kpExecPdSimKeyUp(evt, code, tmout) ) {
                /* process PD simulation only */
		goto skip;
	}

        /* notify key up event */
	keyEvt.h.evttyp = DE_KEYUP;
	keyEvt.keytop = kpKeyTopCode(evt->i.key.keytop,
				evt->i.key.meta.o.kbsel);
	keyEvt.code = code;
	keyEvt.stat = evt->i.key.meta.o;
	kpNotifyEvent(&keyEvt, sizeof(keyEvt));

skip:
        /* switch temporary shift or simple lock modes */
	if ( kpChangeShiftLock(evt->type, NoShift) ) {

                /* notify meta key status change event */
		kpNotifyMetaEvent();
	}
}

/*
 * shift key press / relea
 */
LOCAL void ieShift( InnerEvent *evt )
{
	ShiftKeyKind	shift;
	KpMetaBut	*meta = (KpMetaBut*)&kpMgrInfo.kpState.stat;
	BOOL		chg = FALSE;

	if ( evt->type == IE_S_PRESS ) {
                /* set press status (press) */
		kpMgrInfo.spress |= evt->i.sft.kind;
	} else {
                /* set press status (release) */
		kpMgrInfo.spress &= ~evt->i.sft.kind;

                /* switch temporary shift or simple lock modes */
		chg = kpChangeShiftLock(evt->type, evt->i.sft.kind);
	}

        /* shift status */
	shift = meta->u.tmpShift | meta->u.shiftLock | kpMgrInfo.spress;

	if ( shift != meta->u.shift || chg ) {

                /* change shift status */
		meta->u.shift = shift;

                /* notify meta key status change event */
		kpNotifyMetaEvent();
	}
}

/*
 * key input error
 */
LOCAL void ieKeyErr( InnerEvent *evt, TMO *tmout )
{
	InnerEvent	shift;

	if ( evt->i.key.keytop.w == InvalidKeytop ) {
                /* shift release */
		shift.type = IE_S_REL;
		shift.i.sft.kind = SK_ALL;
		ieShift(&shift);
	} else {
                /* key up */
		ieKeyUp(evt, tmout);
	}
}

/*
 * PD button up / down, move
 */
LOCAL void iePd( InnerEvent *evt )
{
	InnEvtType	type = evt->type;
	TDEvtTyp	devEvt = 0;
	BOOL		qpress = FALSE;
	UW		nodsp;

	if ( (type & IE_PDMOVE) != 0 ) {
                /* move */
		devEvt = kpPdMoveEvent(evt);
	}
	if ( (type & IE_PDBUT) != 0 ) {
                /* button change */
		devEvt = kpPdButEvent(evt);

                /* is this a quick press? */
		qpress = ( (type & IE_MBUT) == IE_MBUTDOWN
				&& evt->i.pd.stat.o.qpress
				&& kpMgrInfo.pd.pdMode.attr.qpress );
	}

	if ( devEvt != 0 ) {
                /* notify PD event */
		kpNotifyPdEvent(devEvt, nodsp = evt->i.pd.stat.o.nodsp);

		if ( qpress ) {
                        /* quickpress */
			kpMgrInfo.kpState.stat.main = 0;
			kpNotifyPdEvent(DE_PDBUT, nodsp);

			kpMgrInfo.kpState.stat.main = 1;
			kpNotifyPdEvent(DE_PDBUT, nodsp);
		}

                /* switch temporary shift or simple lock modes */
		if ( kpChangeShiftLock(evt->type, NoShift) ) {
                        /* notify meta key status change event */
			kpNotifyMetaEvent();
		}
	}
}

/*
 * internal event processing
 */
EXPORT void kpInnerEventProcess( InnerEvent *evt, TMO *tmout )
{
	switch ( evt->type ) {
	  case IE_NULL:		/* no event */
		return;

	  case IE_KEYDOWN:	 /* key down */
		ieKeyDown(evt, tmout);
		break;
	  case IE_KEYUP:	/* key up */
		ieKeyUp(evt, tmout);
		break;

	  case IE_S_PRESS:	/* shift press */
	  case IE_S_REL:	/* shift release */
	  case IE_S_SCLK:	/* shift single click */
	  case IE_S_DCLK:	/* shift double click */
		ieShift(evt);
		break;

	  case IE_KEYERR:	/* key input error */
		ieKeyErr(evt, tmout);
		break;

	  default:		/* PD related */
		iePd(evt);
	}
}
