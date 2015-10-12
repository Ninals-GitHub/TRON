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
 *	accept.c
 *
 *       KB/PD device manager
 *       request handler task
 */

#include "kbpd.h"

/*
 * set KB/PD status (DN_KPSTAT)
 */
LOCAL ER setKpState( KPStat *state )
{
	ER	err;

	/*
         * only PD position can be changed.
         * (other information can not be changed.)
	 */
	err = kpMovePointerAndNotify(state->xpos, state->ypos);

	return err;
}

/*
 * set meta key / button status (DN_KPMETABUT)
 */
LOCAL ER setKpMetaBut( MetaBut meta[2] )
{
const	UW	KBMET =	ES_ALPH|ES_KANA
			|ES_LSHFT|ES_RSHFT|ES_EXT|ES_CMD
			|ES_LLSHFT|ES_LRSHFT|ES_LEXT|ES_LCMD
			|ES_TLSHFT|ES_TRSHFT|ES_TEXT|ES_TCMD
			|ES_HAN|ES_KBSEL;
const	UW	PDBUT =	ES_BUT|ES_BUT2;
const	UW	PDMOV =	ES_NODSP;
const	UW	PDSIM =	ES_PDSIM;
	UW	stat, chg;
	ER	err, error = E_OK;
	union {
		MetaBut	stat;
		UW	uw;
	} u;

	u.stat = kpMgrInfo.kpState.stat;
	stat = (u.uw & *(UW*)&meta[0]) | *(UW*)&meta[1];
	chg = stat ^ u.uw;

	if ( (chg & KBMET) != 0 ) {
                /* change meta key status */
		u.stat = kpMgrInfo.kpState.stat;
		u.uw ^= chg & KBMET;
		kpMgrInfo.kpState.stat = u.stat;

		if ( (chg & (ES_ALPH|ES_KANA)) != 0 ) {
                        /* change keyboard LED */
			err = kpChangeKbInputMode(kpMgrInfo.kpState.stat.mode);
			if ( err < E_OK ) error = err;
		}

		err = kpNotifyMetaEvent();
		if ( err < E_OK ) error = err;
	}

	if ( (chg & (PDBUT|PDMOV)) != 0 ) {
                /* change PD status */
		u.stat = kpMgrInfo.kpState.stat;
		u.uw ^= chg & (PDBUT|PDMOV);
		kpMgrInfo.kpState.stat = u.stat;

		err = kpNotifyPdEvent(
			( (chg & PDBUT) != 0 )? DE_PDBUT: DE_PDMOVE,
			kpMgrInfo.kpState.stat.nodsp);
		if ( err < E_OK ) error = err;
	}

	if ( (chg & PDSIM) != 0 ) {
                /* change PD simulation status */
		u.stat = kpMgrInfo.kpState.stat;
		u.uw ^= chg & PDSIM;
		kpMgrInfo.kpState.stat = u.stat;

		err = kpNotifyPdEvent(DE_PDSTATE,
			kpMgrInfo.kpState.stat.nodsp);
		if ( err < E_OK ) error = err;
	}

	return error;
}

/*
 * select keyboard definition
 *      if datano is invalid, return FALSE.
 */
Inline BOOL SelKbDef( UW *kbsel, UW *kid, W datano )
{
	if ( datano <= DN_KEYDEF_S && datano >= DN_KEYDEF_E ) {
		*kbsel = 0;
		*kid = DN_KEYDEF_S - datano;
		return TRUE;
	}
	if ( datano <= DN_KEYDEF2_S && datano >= DN_KEYDEF2_E ) {
		*kbsel = 1;
		*kid = DN_KEYDEF2_S - datano;
		return TRUE;
	}
	return FALSE;
}

/*
 * keyboard definition
 *       keytopofs < 0   change keytable only
 *       keytabsz <= 0   change keytop code offset only
 */
LOCAL ER defineKeyboard( UW kbsel, UW kid,
				W keytopofs, KeyTab *keytab, W keytabsz )
{
	KbDef	*kbdef = GetKbDef(kbsel, kid);
	ER	err;

	if ( keytabsz <= 0 ) {
                /* change keytop code offset only */
		if ( kbdef == NULL ) { err = E_NOEXS; goto err_ret; }
	} else {
                /* change keytable, too */
		if ( keytabsz < offsetof(KeyTab, kct)
		  || !(keytab->keymax > 0 && keytab->keymax <= KEYMAX)
		  || !(keytab->kctmax > 0 && keytab->kctmax <= KCTSEL)
		  || !(keytabsz >= keytab->keymax * keytab->kctmax * sizeof(UH)
						+ offsetof(KeyTab, kct)) )
			{ err = E_PAR; goto err_ret; }

                /* During new registration, if keytop code offset is unspecified,
                   set keytop code offset to 0 */
		if ( kbdef == NULL && keytopofs < 0 ) keytopofs = 0;
	}

	if ( keytabsz > 0 ) {
                /* chnage keytable */
		kbdef = Vrealloc(kbdef,
				offsetof(KbDef, keyDef.keytab) + keytabsz);
		if ( kbdef == NULL ) { err = E_NOMEM; goto err_ret; }
		kbdef->size = offsetof(KeyDef, keytab) + keytabsz;
		SetKbDef(kbsel, kid, kbdef);

		memcpy(&kbdef->keyDef.keytab, keytab, keytabsz);
	}

	if ( keytopofs >= 0 ) {
                /* change keytop code offset */
		kbdef->keyDef.keytopofs = keytopofs;
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("defineKeyboard err = %d\n", err));
	return err;
}

/*
 * configure keyboard definition (DN_KEYDEF)
 */
LOCAL ER setKeyDef( KeyDef *keydef, UW kbsel, UW kid, W datacnt )
{
	ER		err;

	if ( datacnt >= offsetof(KeyDef, keytab.kctmax)
	  && keydef->keytab.keymax == 0 ) {
                /* deletion */
		KbDef	*kbdef = GetKbDef(kbsel, kid);
		if ( kbdef != NULL ) {
			SetKbDef(kbsel, kid, NULL);
			Vfree(kbdef);
		}
	} else {
                /* set */
		if ( (datacnt -= offsetof(KeyDef, keytab)) < 0 )
					return E_PAR;
		err = defineKeyboard(kbsel, kid,
				keydef->keytopofs, &keydef->keytab, datacnt);
		if ( err < E_OK ) return err;
	}

	return E_OK;
}

/*
 * configure keytable (DN_KEYTAB)
 */
LOCAL ER setKeyTab( KeyTab *table, W datacnt )
{
	return defineKeyboard(0, kpMgrInfo.kb.keyID, -1, table, datacnt);
}

/*
 * set keyboard ID (DN_KEYID)
 */
LOCAL ER setKeyID( UW kid )
{
        /* parameter check */
	if ( kid < 0 || kid > MAX_KID ) return E_PAR;

	kpMgrInfo.kb.keyID = kid;
	kpMgrInfo.kb.defKeyID = 2; /* fixed */

	return E_OK;
}

/*
 * set key mode (DN_KEYMODE)
 */
LOCAL ER setKeyMode( KeyMode *mode )
{
#define	settime(time)					\
	if ( mode->time >= 0 ) {			\
		kpMgrInfo.kb.keyMode.time		\
			= min(mode->time, KB_MAXTIME);	\
	}

        /* change key mode */
	settime(ontime);
	settime(offtime);
	settime(invtime);
	settime(contime);
	settime(sclktime);
	settime(dclktime);
	kpMgrInfo.kb.keyMode.tslock = mode->tslock;

	return E_OK;

#undef settime
}

/*
 * set PD mode (DN_PDMODE)
 */
LOCAL ER setPdMode( PdMode *mode )
{
#define	settime(time)					\
	if ( mode->time >= 0 ) {			\
		kpMgrInfo.pd.pdMode.time		\
			= min(mode->time, PD_MAXTIME);	\
	}

	union {
		PdAttr	attr;
		UW	uw;
	} u;

        /* change PD mode */
	settime(ontime);
	settime(offtime);
	settime(invtime);
	settime(timeout);
	kpMgrInfo.pd.pdMode.attr = mode->attr;

        /* change PD scan frequency */
	kpChangePdScanRate(mode->attr.rate);

        /* change sensitivity */
	u.attr = mode->attr;
	kpChangePdSense(u.uw & (PD_ACMSK|PD_ABS|PD_SNMSK));

	return E_OK;

#undef settime
}

/*
 * set PD range (DN_PDRANGE)
 */
LOCAL ER setPdRange( PdRange *range )
{
	BOOL	move;
	ER	err = E_OK;

        /* parameter check */
	if ( range->xmax < 0 || range->ymax < 0 )
				return E_PAR;

        /* change PD range */
	kpMgrInfo.pd.pdRange = *range;

        /* adjust pointer position */
	move = kpMovePointer(kpMgrInfo.kpState.xpos, kpMgrInfo.kpState.ypos);

	if ( move ) {
                /* notify PD movement event */
		err = kpNotifyPdEvent(DE_PDMOVE, 0);
	}

	return err;
}

/*
 * set PD simulation speed (DN_PDSIM)
 */
LOCAL ER setPdSimSpeed( W simSpeed )
{
        /* parameter check */
	if ( !(simSpeed >= 0 && simSpeed <= 15) )
				return E_PAR;

        /* change PD simulation speed */
	kpMgrInfo.pd.pdSimSpeed = simSpeed;

	if ( simSpeed == 0 ) {
                /* halt PD simulation */
		kpMgrInfo.kpState.stat.pdsim = 0;
	}

	return E_OK;
}

/*
 * configure PD simulation temporary halt(DN_PDSIMINH)
 */
LOCAL ER setPdSimInh( BOOL inhibit )
{
        /* change PD simulation temporary halt configuration */
	kpMgrInfo.pd.pdSimInh = inhibit;

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * read data
 */
LOCAL INT readData( ID devid, INT datano, INT datacnt, void *buf, SDI sdi )
{
#define setAddrAndSize(var)	addr = &(var); size = sizeof(var)

	void	*addr;
	W	size;
	KbDef	*kbdef;
	UW	kbsel, kid;

        /* parameter check */
	if ( datacnt < 0 ) {
		DEBUG_PRINT(("readData, datacnt(%d) err\n", datacnt));
		return E_PAR;
	}

	switch ( datano ) {
	  case DN_KPEVENT:	/* message buffer ID for event notification    (RW) */
		setAddrAndSize(kpMgrInfo.eventMbf);
		break;
	  case DN_KPSTAT:	/* KB/PD status                             (RW) */
		setAddrAndSize(kpMgrInfo.kpState);
		break;
	  case DN_KEYMAP:	/* keymap                                (R)  */
		setAddrAndSize(kpMgrInfo.kb.keyMap);
		break;
	  case DN_KEYTAB:	/* keytable                               (RW) */
		kbdef = GetKbDef(0, kpMgrInfo.kb.keyID);
		if ( kbdef == NULL ) return E_NOEXS;
		addr = &kbdef->keyDef.keytab;
		size = kbdef->size - offsetof(KeyDef, keytab);
		break;
	  case DN_KEYMODE:	/* change key mode 			(RW) */
		setAddrAndSize(kpMgrInfo.kb.keyMode);
		break;
	  case DN_PDMODE:	/* PD mode 				(RW) */
		setAddrAndSize(kpMgrInfo.pd.pdMode);
		break;
	  case DN_PDRANGE:	/* PD range            (RW) */
		setAddrAndSize(kpMgrInfo.pd.pdRange);
		break;
	  case DN_PDSIM:	/* PD simulation speed                (RW) */
		setAddrAndSize(kpMgrInfo.pd.pdSimSpeed);
		break;
	  case DN_PDSIMINH:	/* halt PD simulation temporarily (RW) */
		setAddrAndSize(kpMgrInfo.pd.pdSimInh);
		break;
	  case DN_KPINPUT:	/* input mailbox ID                  (R)  */
		setAddrAndSize(kpMgrInfo.dataMbx);
		break;
	  case DN_KEYID:	/* keyboard ID                     (RW) */
		setAddrAndSize(kpMgrInfo.kb.keyID);
		break;

	  default:
		if ( SelKbDef(&kbsel, &kid, datano) ) {
			/* DN_KEYDEF keyboard definition                    (RW) */
			kbdef = GetKbDef(kbsel, kid);
			if ( kbdef == NULL ) return E_NOEXS;
			addr = &kbdef->keyDef;
			size = kbdef->size;
		} else {
			/* data number error */
			DEBUG_PRINT(("readData, datano(%d) err\n", datano));
			return E_PAR;
		}
	}

	if ( datacnt > 0 ) {
		/* read data */
		if ( datacnt < size ) size = datacnt;
		memcpy(buf, addr, size);
	}
	return size;

#undef	setAddrAndSize
}

EXPORT INT kpReadFn( ID devid, INT datano, INT datacnt, void *buf, SDI sdi )
{
	INT	er;

	Lock(&kpMgrInfo.lock);
	er = readData(devid, datano, datacnt, buf, sdi);
	Unlock(&kpMgrInfo.lock);

	return er;
}

/*
 * data write
 */
LOCAL INT writeData( ID devid, INT datano, INT datacnt, void *buf, SDI sdi )
{
	W	size;
	KbDef	*kbdef;
	UW	kbsel, kid;
	ER	err = E_OK;

        /* parameter check */
	if ( datacnt < 0 ) {
		DEBUG_PRINT(("writeData, datacnt(%d) err\n", datacnt));
		return E_PAR;
	}

	kbsel = kid = 0;
	switch ( datano ) {
	  case DN_KPEVENT:	/* message buffer ID for event notification    (RW) */
		size = sizeof(kpMgrInfo.eventMbf);
		break;
	  case DN_KPSTAT:	/* KB/PD status                             (RW) */
		size = sizeof(kpMgrInfo.kpState);
		break;
	  case DN_KEYTAB:	/* keytable                               (RW) */
		kbdef = GetKbDef(0, kpMgrInfo.kb.keyID);
		size = ( kbdef == NULL )?
				0: kbdef->size - offsetof(KeyDef, keytab);
		break;
	  case DN_KEYMODE:	/* change key mode			(RW) */
		size = sizeof(kpMgrInfo.kb.keyMode);
		break;
	  case DN_PDMODE:	/* PD mode			(RW) */
		size = sizeof(kpMgrInfo.pd.pdMode);
		break;
	  case DN_PDRANGE:	/* PD range            (RW) */
		size = sizeof(kpMgrInfo.pd.pdRange);
		break;
	  case DN_PDSIM:	/* PD simulation speed                (RW) */
		size = sizeof(kpMgrInfo.pd.pdSimSpeed);
		break;
	  case DN_PDSIMINH:	/* halt PD simulation temporarily (RW) */
		size = sizeof(kpMgrInfo.pd.pdSimInh);
		break;
	  case DN_KEYID:	/* keyboard ID                     (RW) */
		size = sizeof(kpMgrInfo.kb.keyID);
		break;
	  case DN_KPMETABUT:	/* meta key / button status                   (W)  */
		size = sizeof(MetaBut) * 2;
		break;

	  case DN_KEYMAP:	/* keymap                                (R)  */
	  case DN_KPINPUT:	/* input mailbox ID                  (R)  */
                /* read-only attribute data */
		DEBUG_PRINT(("writeData, read only\n"));
		return E_PAR;

	  default:
		if ( SelKbDef(&kbsel, &kid, datano) ) {
			/* DN_KEYDEF keyboard definition                    (RW) */
			kbdef = GetKbDef(kbsel, kid);
			size = ( kbdef == NULL )? 0: kbdef->size;
		} else {
			/* data number error */
			DEBUG_PRINT(("writeData, datano(%d) err\n", datano));
			return E_PAR;
		}
	}

	if ( datacnt > 0 ) {
		if ( datano != DN_KEYTAB
		  && !(datano <= DN_KEYDEF_S && datano >= DN_KEYDEF_E)
		  && !(datano <= DN_KEYDEF2_S && datano >= DN_KEYDEF2_E) ) {
			/* except for keytable, partial write is prohibited */
			if ( datacnt < size ) return E_PAR;
		}

		/* data write */
		switch ( datano ) {
		  case DN_KPEVENT:	/* message buffer ID for event notification */
			kpMgrInfo.eventMbf = *(ID*)buf;
			break;
		  case DN_KPSTAT:	/* KB/PD status                          */
			err = setKpState((KPStat*)buf);
			break;
		  case DN_KEYTAB:	/* keytable */
			err = setKeyTab((KeyTab*)buf, datacnt);
			break;
		  case DN_KEYMODE:	/* key mode */
			err = setKeyMode((KeyMode*)buf);
			break;
		  case DN_PDMODE:	/* PD mode */
			err = setPdMode((PdMode*)buf);
			break;
		  case DN_PDRANGE:	/* PD range */
			err = setPdRange((PdRange*)buf);
			break;
		  case DN_PDSIM:	/* PD simulation speed                */
			err = setPdSimSpeed(*(W*)buf);
			break;
		  case DN_PDSIMINH:	/* halt PD simulation temporarily */
			err = setPdSimInh(*(BOOL*)buf);
			break;
		  case DN_KEYID:	/* keyboard ID */
			err = setKeyID(*(UW*)buf);
			break;
		  case DN_KPMETABUT:	/* meta key / button status  */
			err = setKpMetaBut((MetaBut*)buf);
			break;
		  default:		/* keyboard definition */
			err = setKeyDef((KeyDef*)buf,
						kbsel, kid, datacnt);
		}
	}
	if ( err != E_OK ) {
		DEBUG_PRINT(("writeData, write err = %d\n", err));
		return err;
	}
	return size;
}

EXPORT INT kpWriteFn( ID devid, INT datano, INT datacnt, void *buf, SDI sdi )
{
	INT	er;

	Lock(&kpMgrInfo.lock);
	er = writeData(devid, datano, datacnt, buf, sdi);
	Unlock(&kpMgrInfo.lock);

	return er;
}

/* ------------------------------------------------------------------------ */

/*
 * enter suspend state
 */
LOCAL ER suspend( void )
{
	ER	err, error = E_OK;

        /* I/O driver suspend request */
	err = kpSendDeviceCommand(SuspendKBPD);
	if ( err < E_OK ) error = err;

        /* key and button forced to up state */
	err = kpKeyAndButtonForceUp();
	if ( err < E_OK ) error = err;

	kpMgrInfo.suspended = TRUE;  /* suspended */

DO_DEBUG(
	if ( error < E_OK ) DEBUG_PRINT(("suspend err = %d\n", error));
)
	return error;
}

/*
 * resume
 */
LOCAL ER resume( void )
{
	ER	err, error = E_OK;

        /* ignored if the system is not suspended */
	if ( !kpMgrInfo.suspended ) return E_OK;

        /* I/O driver resume request */
	err = kpSendDeviceCommand(ResumeKBPD);
	if ( err < E_OK ) error = err;

        /* record current status in I/O driver */
	err = kpSetAllDeviceStatus();
	if ( err < E_OK ) error = err;

	kpMgrInfo.suspended = FALSE;  /* release suspend */

DO_DEBUG(
	if ( error < E_OK ) DEBUG_PRINT(("resume err = %d\n", error));
)
	return error;
}

/* ------------------------------------------------------------------------ */

/*
 * event request processing
 */
EXPORT INT kpEventFn( INT evttyp, void *evtinf, SDI sdi )
{
	ER	err;

	Lock(&kpMgrInfo.lock);

	switch ( evttyp ) {
	  case TDV_SUSPEND:
		err = suspend();
		break;

	  case TDV_RESUME:
		err = resume();
		break;

	  default:
                /* command error */
		err = E_PAR;
		DEBUG_PRINT(("evttyp(%d) err\n", evttyp));
	}

	Unlock(&kpMgrInfo.lock);
	return err;
}

