/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by T-Engine Forum at 2011/09/08.
 *
 *----------------------------------------------------------------------
 */

/*
 *	device.c (T-Kernel/SM)
 *	Device Management Function
 */

#include "sysmgr.h"
#include <sys/rominfo.h>
#include <sys/svc/ifdevmgr.h>

/* Lock for device management exclusive control */
EXPORT	FastMLock	DevMgrLock;
/* Semaphore for device management synchronous control */
EXPORT	ID		DevMgrSync;
/* Device initial setting information */
LOCAL	T_IDEV		DefaultIDev;

/* Set Object Name in .exinf for DEBUG */
#define OBJNAME_DMMBF	"DEvt"		/* Event notification mbf */
#define OBJNAME_DMSEM	"DMSy"		/* semaphore of synchronous control */
#define OBJNAME_DMLOCK	"DMLk"		/* Multi-lock for Dev.Mgr. */

/*
 * Initialization of device initial setting information
 */
LOCAL ER initIDev( void )
{
	T_CMBF	cmbf;
	INT	val[2];
	ER	ercd;

	/* Get system information */
	ercd = _tk_get_cfn(SCTAG_TDEVTMBFSZ, val, 2);
	if ( ercd < 2 ) {
		val[0] = -1;
	}

	if ( val[0] >= 0 ) {
		/* Generate message buffer for event notification */
		SetOBJNAME(cmbf.exinf, OBJNAME_DMMBF);
		cmbf.mbfatr = TA_TFIFO;
		cmbf.bufsz  = val[0];
		cmbf.maxmsz = val[1];
		ercd = tk_cre_mbf(&cmbf);
		if ( ercd < E_OK ) {
			DefaultIDev.evtmbfid = 0;
			goto err_ret;
		}
		DefaultIDev.evtmbfid = ercd;
	} else {
		/* Do not use message buffer for event notification */
		DefaultIDev.evtmbfid = 0;
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("initIDev ercd = %d\n", ercd));
	return ercd;
}

/*
 * Unregister device initial setting information
 */
LOCAL ER delIDev( void )
{
	ER	ercd = E_OK;

	/* Delete message buffer for event notification */
	if ( DefaultIDev.evtmbfid > 0 ) {
		ercd = tk_del_mbf(DefaultIDev.evtmbfid);
		DefaultIDev.evtmbfid = 0;
	}

#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("delIDev ercd = %d\n", ercd));
	}
#endif
	return ercd;
}

/* ------------------------------------------------------------------------ */
/*
 *	Device registration management
 */

EXPORT	DevCB		*DevCBtbl;	/* Device registration information
					   table */
EXPORT	QUEUE		UsedDevCB;	/* In-use queue */
LOCAL	QUEUE		FreeDevCB;	/* Unused queue */
LOCAL	INT		MaxRegDev;	/* Maximum number of device
					   registrations */

#define MAX_UNIT	255		/* Maximum number of subunits */

/*
 * Verify validity of device ID
 */
Inline ER check_devid( ID devid )
{
	devid >>= 8;
	if ( devid < 1 || devid > MaxRegDev ) {
		return E_ID;
	}
	return E_OK;
}

/*
 * Initialization of device registration information table
 */
LOCAL ER initDevCB( void )
{
	DevCB	*devcb;
	INT	num;
	ER	ercd;

	/* Get system information */
	ercd = _tk_get_cfn(SCTAG_TMAXREGDEV, &MaxRegDev, 1);
	if ( ercd < 1 ) {
		ercd = E_SYS;
		goto err_ret;
	}
	num = MaxRegDev;

	/* Generate device registration information table */
	DevCBtbl = Imalloc((UINT)num * sizeof(DevCB));
	if ( DevCBtbl == NULL ) {
		ercd = E_NOMEM;
		goto err_ret;
	}

	QueInit(&UsedDevCB);
	QueInit(&FreeDevCB);

	devcb = DevCBtbl;
	while ( num-- > 0 ) {
		QueInsert(&devcb->q, &FreeDevCB);
		devcb->devnm[0] = '\0';
		devcb++;
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("initDevCB ercd = %d\n", ercd));
	return ercd;
}

/*
 * Search registration device
 */
EXPORT DevCB* searchDevCB( CONST UB *devnm )
{
	QUEUE	*q;
	DevCB	*devcb;
vd_printf("devnm:%s\n", devnm);
	for ( q = UsedDevCB.next; q != &UsedDevCB; q = q->next ) {
		devcb = (DevCB*)q;
vd_printf("%s ", devcb->devnm);
		if ( devcb->devnm[0] == devnm[0] && strcmp((char*)devcb->devnm, (char*)devnm) == 0 ) {
			return devcb; /* Found */
		}
	}

	return NULL;
}

/*
 * Get DevCB for new registration
 */
LOCAL DevCB* newDevCB( CONST UB *devnm )
{
	DevCB	*devcb;

	devcb = (DevCB*)QueRemoveNext(&FreeDevCB);
	if ( devcb == NULL ) {
		return NULL; /* No space */
	}

	strncpy((char*)devcb->devnm, (char*)devnm, L_DEVNM+1);
	QueInit(&devcb->openq);
	QueInit(&devcb->syncq);

	QueInsert(&devcb->q, &UsedDevCB);

	return devcb;
}

/*
 * Free DevCB
 */
LOCAL void delDevCB( DevCB *devcb )
{
	QueRemove(&devcb->q);
	QueInsert(&devcb->q, &FreeDevCB);
	devcb->devnm[0] = '\0';
}

/*
 * Device registration
 */
LOCAL ID _tk_def_dev( CONST UB *devnm, CONST T_DDEV *ddev, T_IDEV *idev, void *caller_gp )
{
	DevCB	*devcb;
	INT	len, evttyp;
	ER	ercd;

	LockREG();

	len = ChkSpaceBstrR(devnm, 0);
	if ( len < E_OK ) {
		ercd = len;
		goto err_ret1;
	}
	if ( len <= 0 || len > L_DEVNM ) {
		ercd = E_PAR;
		goto err_ret1;
	}

	if ( ddev != NULL ) {
		ercd = ChkSpaceR(ddev, sizeof(T_DDEV));
		if ( ercd < E_OK ) {
			goto err_ret1;
		}
		if ( ddev->nsub < 0 || ddev->nsub > MAX_UNIT ) {
			ercd = E_PAR;
			goto err_ret1;
		}

		/* Make sure that the length of the logical device name
		   does not exceed the character limit */
		if ( ddev->nsub > 0   ) {
			++len;
		}
		if ( ddev->nsub > 10  ) {
			++len;
		}
		if ( ddev->nsub > 100 ) {
			++len;
		}
		if ( len > L_DEVNM ) {
			ercd = E_PAR;
			goto err_ret1;
		}
	}

	if ( idev != NULL ) {
		ercd = ChkSpaceRW(idev, sizeof(T_IDEV));
		if ( ercd < E_OK ) {
			goto err_ret1;
		}
	}

	LockDM();

	/* Search whether 'devnm' device is registered */
	devcb = searchDevCB(devnm);
	if ( devcb == NULL ) {
		if ( ddev == NULL ) {
			ercd = E_NOEXS;
			goto err_ret2;
		}

		/* Get 'devcb' for new registration because it is not
		   registered */
		devcb = newDevCB(devnm);
		if ( devcb == NULL ) {
			ercd = E_LIMIT;
			goto err_ret2;
		}
	}

	if ( ddev != NULL ) {
		/* Set/update device registration information */
		devcb->ddev = *ddev;
#if TA_GP
		if ( (ddev->drvatr & TA_GP) == 0 ) {
			/* Apply caller 'gp' if TA_GP is not specified */
			devcb->ddev.gp = caller_gp;
		}
#endif

		if ( idev != NULL ) {
			/* Device initial setting information */
			*idev = DefaultIDev;
		}
		evttyp = TSEVT_DEVICE_REGIST;
	} else {
		if ( !isQueEmpty(&devcb->openq) ) {
			/* In use (open) */
			ercd = E_BUSY;
			goto err_ret2;
		}

		/* Device unregistration */
		delDevCB(devcb);
		evttyp = TSEVT_DEVICE_DELETE;
	}

	UnlockDM();
	UnlockREG();

	/* Device registration/unregistration notification */
	tk_evt_ssy(0, evttyp, 0, DID(devcb));

	return DID(devcb);

err_ret2:
	UnlockDM();
err_ret1:
	UnlockREG();
	DEBUG_PRINT(("_tk_def_dev ercd = %d\n", ercd));
	return ercd;
}

/*
 * Check device initial information
 */
LOCAL ER _tk_ref_idv( T_IDEV *idev )
{
	ER	ercd;

	ercd = ChkSpaceRW(idev, sizeof(T_IDEV));
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	LockDM();
	*idev = DefaultIDev;
	UnlockDM();

	return E_OK;

err_ret:
	DEBUG_PRINT(("_tk_ref_idv ercd = %d\n", ercd));
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Get logical device name
 *	Get the logical device name from
 *	the physical device name (pdevnm) and the subunit number (unitno).
 */
LOCAL void logdevnm( UB *ldevnm, UB *pdevnm, INT unitno )
{
	UB	unostr[12], *cp;

	strcpy((char*)ldevnm, (char*)pdevnm);
	if ( unitno > 0 ) {
		cp = &unostr[11];
		*cp = '\0';
		while (*ldevnm != '\0') {
			++ldevnm;
		}
		--unitno;
		do {
			*(--cp) = (UB)('0' + (unitno % 10));
			unitno /= 10;
		} while (unitno);
		strcat((char*)ldevnm, (char*)cp);
	}
}

/*
 * Get physical device name
 *	Get the subunit number (return value)
 *	from the logical device name (ldevnm) and the physical
 *	device name (pdevnm).
 */
EXPORT INT phydevnm( UB *pdevnm, CONST UB *ldevnm )
{
	UB	c;
	INT	unitno;

	while ( (c = *ldevnm) != '\0' ) {
		if ( c >= '0' && c <= '9' ) {
			break;
		}
		*pdevnm++ = c;
		ldevnm++;
	}
	*pdevnm = '\0';

	unitno = 0;
	if (c != '\0') {
		while ( (c = *ldevnm) != '\0' ) {
			unitno = unitno * 10 + (c - '0');
			ldevnm++;
		}
		++unitno;
	}

	return unitno;
}

/*
 * Get device name
 */
LOCAL ID _tk_get_dev( ID devid, UB *devnm )
{
	DevCB	*devcb;
	ER	ercd;

	ercd = ChkSpaceRW(devnm, (L_DEVNM + 1) * sizeof(UB));
	if ( ercd < E_OK ) {
		goto err_ret1;
	}
	ercd = check_devid(devid);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}

	LockDM();

	devcb = DEVCB(devid);
	if ( (devcb->devnm[0] == '\0')||(UNITNO(devid) > devcb->ddev.nsub) ) {
		ercd = E_NOEXS;
		goto err_ret2;
	}

	logdevnm(devnm, devcb->devnm, UNITNO(devid));

	UnlockDM();

	return DID(devcb);

err_ret2:
	UnlockDM();
err_ret1:
	DEBUG_PRINT(("_tk_get_dev ercd = %d\n", ercd));
	return ercd;
}

/*
 * Get device information
 */
LOCAL ID _tk_ref_dev( CONST UB *devnm, T_RDEV *rdev )
{
	UB	pdevnm[L_DEVNM + 1];
	DevCB	*devcb;
	INT	unitno;
	ER	ercd;

	ercd = ChkSpaceBstrR(devnm, 0);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}
	if ( rdev != NULL ) {
		ercd = ChkSpaceRW(rdev, sizeof(T_RDEV));
		if ( ercd < E_OK ) {
			goto err_ret1;
		}
	}

	unitno = phydevnm(pdevnm, devnm);

	LockDM();

	devcb = searchDevCB(pdevnm);
	if ( devcb == NULL || unitno > devcb->ddev.nsub ) {
		ercd = E_NOEXS;
		goto err_ret2;
	}

	if ( rdev != NULL ) {
		rdev->devatr = devcb->ddev.devatr;
		rdev->blksz  = devcb->ddev.blksz;
		rdev->nsub   = devcb->ddev.nsub;
		rdev->subno  = unitno;
	}

	UnlockDM();

	return DEVID(devcb, unitno);

err_ret2:
	UnlockDM();
err_ret1:
	DEBUG_PRINT(("_tk_ref_dev ercd = %d\n", ercd));
	return ercd;
}

/*
 * Get device information
 */
LOCAL ID _tk_oref_dev( ID dd, T_RDEV *rdev )
{
	OpnCB	*opncb;
	DevCB	*devcb;
	INT	unitno;
	ER	ercd;

	if ( rdev != NULL ) {
		ercd = ChkSpaceRW(rdev, sizeof(T_RDEV));
		if ( ercd < E_OK ) {
			goto err_ret1;
		}
	}

	LockDM();

	ercd = check_devdesc(dd, 0, &opncb);
	if ( ercd < E_OK ) {
		goto err_ret2;
	}

	devcb  = opncb->devcb;
	unitno = opncb->unitno;

	if ( rdev != NULL ) {
		rdev->devatr = devcb->ddev.devatr;
		rdev->blksz  = devcb->ddev.blksz;
		rdev->nsub   = devcb->ddev.nsub;
		rdev->subno  = unitno;
	}

	UnlockDM();

	return DEVID(devcb, unitno);

err_ret2:
	UnlockDM();
err_ret1:
	DEBUG_PRINT(("_tk_oref_dev ercd = %d\n", ercd));
	return ercd;
}

/*
 * Get registration device list
 */
LOCAL INT _tk_lst_dev( T_LDEV *ldev, INT start, INT ndev )
{
	DevCB	*devcb;
	QUEUE	*q;
	INT	n, end;
	ER	ercd;

	if ( start < 0 || ndev < 0 ) {
		ercd = E_PAR;
		goto err_ret;
	}
	ercd = ChkSpaceRW(ldev, ndev * (INT)sizeof(T_LDEV));
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	LockDM();

	end = start + ndev;
	n = 0;
	for ( q = UsedDevCB.next; q != &UsedDevCB; q = q->next ) {
		if ( n >= start && n < end ) {
			devcb = (DevCB*)q;
			ldev->devatr = devcb->ddev.devatr;
			ldev->blksz  = devcb->ddev.blksz;
			ldev->nsub   = devcb->ddev.nsub;
			strncpy((char*)ldev->devnm, (char*)devcb->devnm, L_DEVNM);
			ldev++;
		}
		n++;
	}

	UnlockDM();

	if ( start >= n ) {
		ercd = E_NOEXS;
		goto err_ret;
	}

	return n - start;

err_ret:
	DEBUG_PRINT(("_tk_lst_dev ercd = %d\n", ercd));
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Send driver request event
 */
LOCAL INT _tk_evt_dev( ID devid, INT evttyp, void *evtinf )
{
	DevCB	*devcb;
	EVTFN	eventfn;
	void	*exinf;
#if TA_GP
	void	*gp;
#endif
	ER	ercd;

	ercd = check_devid(devid);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}
	if ( evttyp < 0 ) {
		ercd = E_PAR;
		goto err_ret1;
	}

	LockDM();

	devcb = DEVCB(devid);
	if ( (devcb->devnm[0] == '\0')||(UNITNO(devid) > devcb->ddev.nsub) ) {
		ercd = E_NOEXS;
		goto err_ret2;
	}

	eventfn = (EVTFN)devcb->ddev.eventfn;
	exinf = devcb->ddev.exinf;
#if TA_GP
	gp = devcb->ddev.gp;
#endif

	UnlockDM();

	/* Device driver call */
#if TA_GP
	ercd = CallDeviceDriver(evttyp, evtinf, exinf, 0, (FP)eventfn, gp);
#else
	ercd = (*eventfn)(evttyp, evtinf, exinf);
#endif

	return ercd;

err_ret2:
	UnlockDM();
err_ret1:
	DEBUG_PRINT(("_tk_evt_dev ercd = %d\n", ercd));
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Extension SVC entry
 */
LOCAL INT devmgr_svcentry( void *pk_para, FN fncd, void *caller_gp )
{
	ER	ercd;

	/* Test call protection level */
	ercd = ChkCallPLevel();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	switch ( fncd ) {
	  case DEVICE_TK_OPN_DEV_FN:
		{ DEVICE_TK_OPN_DEV_PARA *p = pk_para;
		return _tk_opn_dev(p->devnm, p->omode); }
	  case DEVICE_TK_CLS_DEV_FN:
		{ DEVICE_TK_CLS_DEV_PARA *p = pk_para;
		return _tk_cls_dev(p->dd, p->option); }
	  case DEVICE_TK_REA_DEV_FN:
		{ DEVICE_TK_REA_DEV_PARA *p = pk_para;
		return _tk_rea_dev(p->dd, p->start, p->buf, p->size,
							p->tmout); }
	  case DEVICE_TK_SREA_DEV_FN:
		{ DEVICE_TK_SREA_DEV_PARA *p = pk_para;
		return _tk_srea_dev(p->dd, p->start, p->buf, p->size,
							p->asize); }
	  case DEVICE_TK_WRI_DEV_FN:
		{ DEVICE_TK_WRI_DEV_PARA *p = pk_para;
		return _tk_wri_dev(p->dd, p->start, p->buf, p->size,
							p->tmout); }
	  case DEVICE_TK_SWRI_DEV_FN:
		{ DEVICE_TK_SWRI_DEV_PARA *p = pk_para;
		return _tk_swri_dev(p->dd, p->start, p->buf, p->size,
							p->asize); }
	  case DEVICE_TK_WAI_DEV_FN:
		{ DEVICE_TK_WAI_DEV_PARA *p = pk_para;
		return _tk_wai_dev(p->dd, p->reqid, p->asize, p->ioer,
							p->tmout); }
	  case DEVICE_TK_SUS_DEV_FN:
		{ DEVICE_TK_SUS_DEV_PARA *p = pk_para;
		return _tk_sus_dev(p->mode); }
	  case DEVICE_TK_GET_DEV_FN:
		{ DEVICE_TK_GET_DEV_PARA *p = pk_para;
		return _tk_get_dev(p->devid, p->devnm); }
	  case DEVICE_TK_REF_DEV_FN:
		{ DEVICE_TK_REF_DEV_PARA *p = pk_para;
		return _tk_ref_dev(p->devnm, p->rdev); }
	  case DEVICE_TK_OREF_DEV_FN:
		{ DEVICE_TK_OREF_DEV_PARA *p = pk_para;
		return _tk_oref_dev(p->dd, p->rdev); }
	  case DEVICE_TK_LST_DEV_FN:
		{ DEVICE_TK_LST_DEV_PARA *p = pk_para;
		return _tk_lst_dev(p->ldev, p->start, p->ndev); }
	  case DEVICE_TK_EVT_DEV_FN:
		{ DEVICE_TK_EVT_DEV_PARA *p = pk_para;
		return _tk_evt_dev(p->devid, p->evttyp, p->evtinf); }
	  case DEVICE_TK_DEF_DEV_FN:
		{ DEVICE_TK_DEF_DEV_PARA *p = pk_para;
		return _tk_def_dev(p->devnm, p->ddev, p->idev, caller_gp); }
	  case DEVICE_TK_REF_IDV_FN:
		{ DEVICE_TK_REF_IDV_PARA *p = pk_para;
		return _tk_ref_idv(p->idev); }

	  /* T-Kernel 2.0 */
	  case DEVICE_TK_REA_DEV_DU_FN:
		{ DEVICE_TK_REA_DEV_DU_PARA *p = pk_para;
		return _tk_rea_dev_du(p->dd, p->start_d, p->buf, p->size,
				      p->tmout_u); }
	  case DEVICE_TK_SREA_DEV_D_FN:
		{ DEVICE_TK_SREA_DEV_D_PARA *p = pk_para;
		return _tk_srea_dev_d(p->dd, p->start_d, p->buf, p->size,
				      p->asize); }
	  case DEVICE_TK_WRI_DEV_DU_FN:
		{ DEVICE_TK_WRI_DEV_DU_PARA *p = pk_para;
		return _tk_wri_dev_du(p->dd, p->start_d, p->buf, p->size,
				      p->tmout_u); }
	  case DEVICE_TK_SWRI_DEV_D_FN:
		{ DEVICE_TK_SWRI_DEV_D_PARA *p = pk_para;
		return _tk_swri_dev_d(p->dd, p->start_d, p->buf, p->size,
				      p->asize); }
	  case DEVICE_TK_WAI_DEV_U_FN:
		{ DEVICE_TK_WAI_DEV_U_PARA *p = pk_para;
		return _tk_wai_dev_u(p->dd, p->reqid, p->asize, p->ioer,
				     p->tmout_u); }

	  default:
		ercd = E_RSFN;
	}
err_ret:
	DEBUG_PRINT(("devmgr_svcentry ercd = %d\n", ercd));
	return ercd;
}

/*
 * Initialization of system management
 */
EXPORT ER initialize_devmgr( void )
{
	T_DSSY	dssy;
	T_CSEM	csem;
	ER	ercd;

	/* Generate lock for device management exclusive control */
	ercd = CreateMLock(&DevMgrLock, OBJNAME_DMLOCK);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Generate semaphore for device management synchronous control */
	SetOBJNAME(csem.exinf, OBJNAME_DMSEM);
	csem.sematr  = TA_TFIFO | TA_FIRST;
	csem.isemcnt = 0;
	csem.maxsem  = 1;
	ercd = tk_cre_sem(&csem);
	if ( ercd < E_OK ) {
		goto err_ret;
	}
	DevMgrSync = ercd;

	/* Generate device registration information table */
	ercd = initDevCB();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Initialization of device input/output-related */
	ercd = initDevIO();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Initialization of device initial setting information */
	ercd = initIDev();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Subsystem registration */
	dssy.ssyatr    = TA_NULL;
	dssy.ssypri    = DEVICE_PRI;
	dssy.svchdr    = (FP)&devmgr_svcentry;
	dssy.breakfn   = (FP)&devmgr_break;
	dssy.startupfn = (FP)&devmgr_startup;
	dssy.cleanupfn = (FP)&devmgr_cleanup;
	dssy.eventfn   = NULL;
	dssy.resblksz  = sizeof(ResCB);
	ercd = tk_def_ssy(DEVICE_SVC, &dssy);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("initialize_devmgr ercd = %d\n", ercd));
	finish_devmgr();
	return ercd;
}

/*
 * Finalization sequence of system management
 */
EXPORT ER finish_devmgr( void )
{
	ER	ercd;

	/* Unregister subsystem */
	ercd = tk_def_ssy(DEVICE_SVC, NULL);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("1. finish_devmgr -> tk_def_ssy ercd = %d\n", ercd));
	}
#endif

	/* Unregister device initial setting information */
	ercd = delIDev();
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("2. finish_devmgr -> delIDev ercd = %d\n", ercd));
	}
#endif

	/* Finalization sequence of device input/output-related */
	ercd = finishDevIO();
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("3. finish_devmgr -> finishDevIO ercd = %d\n", ercd));
	}
#endif

	/* Delete device registration information table */
	if ( DevCBtbl != NULL ) {
		Ifree(DevCBtbl);
		DevCBtbl = NULL;
	}

	/* Delete semaphore for device management synchronous control */
	if ( DevMgrSync > 0 ) {
		tk_del_sem(DevMgrSync);
		DevMgrSync = 0;
	}

	/* Delete lock for device management exclusive control */
	DeleteMLock(&DevMgrLock);

	return ercd;
}
