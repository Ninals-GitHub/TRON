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
 *	etc.c
 *
 *       KB/PD device manager
 *       miscellaneous functions
 */

#include "kbpd.h"

/* ===== event notification ===================================================== */

/*
 * event notification
 */
EXPORT ER kpNotifyEvent( void *evt, W size )
{
	ER	ercd;

        /* unnecessary to notify event if message buffer for event notification is unspecified */
	if ( kpMgrInfo.eventMbf == InvalidID ) return E_OK;

        /* event notification */
	ercd = tk_snd_mbf(kpMgrInfo.eventMbf, evt, size, TMO_FEVR);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("kpNotifyEvent, tk_snd_mbf err = %d\n", ercd));
		return ercd;
	}

	return E_OK;
}

/*
 * notify meta key status change event
 */
EXPORT ER kpNotifyMetaEvent( void )
{
	KeyEvt	keyEvt;
	ER	err;

        /* set meta key status change event */
	keyEvt.h.evttyp = DE_KEYMETA;
	keyEvt.keytop = 0;
	keyEvt.code = 0;
	keyEvt.stat = kpMgrInfo.kpState.stat;

        /* event notification */
	err = kpNotifyEvent(&keyEvt, sizeof(keyEvt));
	if ( err != E_OK ) {
		DEBUG_PRINT(("kpNotifyMetaEvent, err = %d\n", err));
		return err;
	}

	return E_OK;
}

/*
 * notify PD event
 */
EXPORT ER kpNotifyPdEvent( TDEvtTyp devEvt, UW nodsp )
{
	PdEvt	pdEvt;
	ER	err;

        /* change pointer invisible status */
	kpMgrInfo.kpState.stat.nodsp = nodsp;

        /* set PD event */
	pdEvt.h.evttyp = devEvt;
	pdEvt.stat = kpMgrInfo.kpState;

        /* event notification */
	err = kpNotifyEvent(&pdEvt, sizeof(pdEvt));
	if ( err != E_OK ) {
		DEBUG_PRINT(("kpNotifyPdEvent, err = %d\n", err));
		return err;
	}

	return E_OK;
}

/* ===== KB ========================================================== */

/*
 * conversion from key top code to key code
 */
EXPORT UH kpToKeycode( KeyTop keytop, KpMetaBut *meta )
{
#define	keyTab	kbdef->keyDef.keytab

	KbDef	*kbdef;
	W	sel;
	UH	code;

	kbdef = GetKbDef(meta->o.kbsel, keytop.u.kid);
	if ( kbdef == NULL ) {
                /* if keytable for kbsel does not exist, then
                   use keyboard definition 1 (kbsel=0) */
		kbdef = GetKbDef(0, keytop.u.kid);
		if ( kbdef == NULL ) return InvalidKeycode;
	}

	if ( keytop.u.kcode >= keyTab.keymax ) return InvalidKeycode;

	sel = (*(UW*)meta & 0xfc) >> 2;
	code = keyTab.kct[keyTab.keymax * keyTab.kctsel[sel] + keytop.u.kcode];

	return code;

#undef keyTab
}

/*
 * find out key type
 *     return InKeyKind if it is direct key input
 *       if it is shiftkey, then return ShiftKeyKind
 */
EXPORT W kpGetKeyKind( KeyTop keytop )
{
	UH			code;

        /* evaluate keycode obtained from the keymap corresponding to the current shift status. */
	code = kpToKeycode(keytop, (KpMetaBut*)&kpMgrInfo.kpState.stat);

	switch ( code ) {
          /* shift key */
	  case KC_SHT_R:	return SK_SHIFT_R;	/* shift right */
	  case KC_SHT_L:	return SK_SHIFT_L;	/* shift Left */
	  case KC_EXP:		return SK_EXPANSION;	/* expansion */
	  case KC_CMD:		return SK_COMMAND;	/* command */

          /* direct key input */
	  case KC_CC_U:		return IK_CC_U;		/* main CC */
	  case KC_CC_D:		return IK_CC_D;		/* main CC */
	  case KC_CC_R:		return IK_CC_R;		/* main CC */
	  case KC_CC_L:		return IK_CC_L;		/* main CC */
	}
	return NormalKey;
}

/*
 * switch temporary shift or simple lock modes
 *       returns TRUE if meta key status changes
 */
EXPORT BOOL kpChangeShiftLock( InnEvtType type, ShiftKeyKind skind )
{
	KpMetaBut	meta;
	KpMetaBut	*stat = (KpMetaBut*)&kpMgrInfo.kpState.stat;

	meta.o = kpMgrInfo.kpState.stat;

	if ( (type & IE_BUTUP) != 0 ) type = IE_BUTUP;

	switch ( type ) {
	  case IE_KEYUP:	/* key up */
	  case IE_BUTUP:	/* PD button up */
		meta.u.tmpShift = 0;
		break;

	  case IE_S_REL:	/* shift release */
		meta.u.tmpShift &= ~skind;
		meta.u.shiftLock &= ~skind;
		break;

	  case IE_S_SCLK:	/* shift single click */
		if ( kpMgrInfo.kb.keyMode.tslock ) {
                        /* temporary shift */
			if ( (meta.u.shiftLock & skind) != 0 ) {
				meta.u.shiftLock &= ~skind;
				break;
			}
			if ( (meta.u.tmpShift & skind) != 0 ) {
				meta.u.tmpShift &= ~skind;
				meta.u.shiftLock |= skind;
				break;
			}
			meta.u.tmpShift |= skind;
		} else {
			if ( (meta.u.shiftLock & skind) == 0 ) {
				meta.u.tmpShift ^= skind;
			} else {
				meta.u.shiftLock &= ~skind;
			}
		}
		break;

	  case IE_S_DCLK:	/* shift double click */
		meta.u.tmpShift &= ~skind;
		meta.u.shiftLock |= skind;
		break;

	  default:
		return FALSE;
	}

	if ( meta.u.tmpShift == stat->u.tmpShift
	  && meta.u.shiftLock == stat->u.shiftLock ) return FALSE;

        /* change meta key status */
	stat->u.tmpShift = meta.u.tmpShift;
	stat->u.shiftLock = meta.u.shiftLock;
	stat->u.shift = meta.u.tmpShift | meta.u.shiftLock | kpMgrInfo.spress;

	return TRUE;
}

/*
 * key top code after addition of offset
 */
EXPORT W kpKeyTopCode( KeyTop keytop, UW kbsel )
{
	KbDef	*kbdef;
	W	keytopcode, keytopofs;

	kbdef = GetKbDef(kbsel, keytop.u.kid);
	if ( kbdef == NULL ) {
                /* if keytable for kbsel does not exist, then
                   use keyboard definition 1 (kbsel=0) */
		kbdef = GetKbDef(0, keytop.u.kid);
	}
	keytopofs = ( kbdef == NULL )? 0: kbdef->keyDef.keytopofs;

	keytopcode = keytop.u.kcode + keytopofs;
	if ( keytopcode >= KEYMAX ) return InvalidKeytop;

	return keytopcode;
}

/*
 * change keymap
 */
EXPORT void kpSetOrResetKeyMap( KeyTop keytop, UW kbsel, UW press )
{
	W	i;
	UB	*idx;

	if ( (i = kpKeyTopCode(keytop, kbsel)) != InvalidKeytop ) {
		idx = &kpMgrInfo.kb.keyMap[i >> 3];
		if ( press )	*idx |=  0x80 >> (i & 7);
		else 		*idx &= ~0x80 >> (i & 7);
	}
}

/*
 * reset all keymaps
 */
EXPORT void kpAllResetKeyMap( void )
{
	memset(kpMgrInfo.kb.keyMap, 0, sizeof(KeyMap));
}

/* ===== PD ========================================================== */

/*
 * pre-processing of PD events from real I/O drivers
 *       Left-handed processing (buttons and coordinates are modified accordingly)
 *       One button operation
 */
EXPORT void kpPdPreProcess( PdInput *msg )
{
	if ( kpMgrInfo.pd.pdMode.attr.reverse ) {
                /* left-handed mode */
		if ( msg->stat.xyrev && !msg->stat.abs ) {
                        /* reverse coordinate */
			msg->xpos = - msg->xpos;
			msg->ypos = - msg->ypos;
		}
		if ( msg->stat.butrev ) {
                        /* swap buttons */
			SWAP(UW, msg->stat.main, msg->stat.sub);
		}
	}

	if ( kpMgrInfo.pd.pdMode.attr.qpress ) {
		if ( msg->stat.onebut && msg->stat.qpress ) {
                        /* one button quick press */
			msg->stat.main = 1;
		}
	}
}

/*
 * normalize pointer position (X coordinate)
 */
LOCAL H normalizePositionX( W xpos )
{
#define	xmax	(kpMgrInfo.pd.pdRange.xmax)

	if ( xpos < 0 ) return 0;
	if ( xpos >= xmax ) return xmax - 1;
	return xpos;

#undef xmax
}

/*
 * normalize pointer position (Y coordinate)
 */
LOCAL H normalizePositionY( W ypos )
{
#define	ymax	(kpMgrInfo.pd.pdRange.ymax)

	if ( ypos < 0 ) return 0;
	if ( ypos >= ymax ) return ymax - 1;
	return ypos;

#undef ymax
}

/*
 * move pointer
 *       call after locking manager information
 *       if there is a movement, TRUE is returned
 */
EXPORT BOOL kpMovePointer( W xpos, W ypos )
{
	xpos = normalizePositionX(xpos);
	ypos = normalizePositionY(ypos);

	if ( kpMgrInfo.kpState.xpos == xpos
	  && kpMgrInfo.kpState.ypos == ypos ) return FALSE;

        /* move pointer */
	kpMgrInfo.kpState.xpos = xpos;
	kpMgrInfo.kpState.ypos = ypos;

	return TRUE;
}

/*
 * move pointer and notify event
 */
EXPORT ER kpMovePointerAndNotify( H xpos, H ypos )
{
	BOOL	move;
	ER	err = E_OK;

        /* move PD */
	move = kpMovePointer(xpos, ypos);

	if ( move ) {
                /* notify PD movement event */
		err = kpNotifyPdEvent(DE_PDMOVE, 0);
	}

	return err;
}

/*
 * move pointer
 *       process internal event, and move pointer
 */
EXPORT TDEvtTyp kpPdMoveEvent( InnerEvent *evt )
{
	W		x, y;
	TDEvtTyp	devEvt = 0;

	x = evt->i.pd.x;
	y = evt->i.pd.y;

	if ( evt->i.pd.stat.o.abs ) {
                /* absolute coordinate device */
		PdRange *range = &kpMgrInfo.pd.pdRange;

		x = range->xmax * x / PDIN_XMAX;
		y = range->ymax * y / PDIN_YMAX;

#if	0	/* we no longer support relative movement with absolute coordinate device. */
		if ( !kpMgrInfo.pd.pdMode.attr.absolute
		  && !evt->i.pd.stat.o.norel ) {
                        /* relative coordinate mode */
			if ( evt->i.pd.stat.o.vst ) {
                                /* save the origin */
				kpMgrInfo.firstXpos = x;
				kpMgrInfo.firstYpos = y;
			}
                        /* displacement */
			x = x - kpMgrInfo.firstXpos;
			y = y - kpMgrInfo.firstYpos;
                        /* move */
			kpMgrInfo.firstXpos += x;	/* move the origin */
			kpMgrInfo.firstYpos += y;
			x += kpMgrInfo.kpState.xpos;	/* move PD position */
			y += kpMgrInfo.kpState.ypos;
		}
#endif
	} else {
                /* relative coordinate device */
		x += kpMgrInfo.kpState.xpos;
		y += kpMgrInfo.kpState.ypos;
	}

        /* move PD */
	if ( kpMovePointer(x, y) ) {
		devEvt = DE_PDMOVE;
	}

	return devEvt;
}

/*
 * change button status
 *           process internal event, and change button status.
 */
EXPORT TDEvtTyp kpPdButEvent( InnerEvent *evt )
{
	if ( (evt->type & IE_MBUTDOWN) != 0 ) {	/* PD main button down */
		kpMgrInfo.kpState.stat.main = 1;
	}
	if ( (evt->type & IE_MBUTUP) != 0 ) {	/* PD main button up */
		kpMgrInfo.kpState.stat.main = 0;
	}
	if ( (evt->type & IE_SBUTDOWN) != 0 ) {	/* PD subbutton down */
		kpMgrInfo.kpState.stat.sub = 1;
	}
	if ( (evt->type & IE_SBUTUP) != 0 ) {	/* PD subbutton up */
		kpMgrInfo.kpState.stat.sub = 0;
	}

	return DE_PDBUT;
}
