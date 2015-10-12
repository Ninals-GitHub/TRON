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
 *	accept.c	System disk driver
 *
 *	Device request processing
 */

#include "sdisk.h"

/*
 *	Check the device ID error (Also set the object subunit)
 */
LOCAL	DrvTab	*CheckDrvTab(DrvTab *drv, INT devid)
{
	for (; drv != NULL; drv = drv->Next) {
		if (devid >= drv->DevId &&
		    devid <= drv->DevId + drv->nSUnit) break;
	}
	if (drv != NULL) {
		/* Set the object subunit */
		drv->CurSUnit = devid - drv->DevId;
	}
	return drv;
}

/*
 *	Check data size
 */
EXPORT	INT	CheckSize(W datacnt, W size)
{
	if (datacnt >= size) return E_OK;
	return (datacnt == 0) ? size : E_PAR;
}

/*
 *	DN_DISKEVENT: message buffer ID for event notification (RW)
 */
LOCAL	INT	accDISKEVENT(DrvTab *drv, T_DEVREQ *req)
{
	if (req->cmd == TDC_READ)	*((ID*)req->buf) = drv->MbfId;
	else				drv->MbfId = *((ID*)req->buf);
	return sizeof(ID);
}

/*
 *	DN_DISKINFO: disk information (R)
 */
LOCAL	INT	accDISKINFO(DrvTab *drv, T_DEVREQ *req)
{
	sdMakeDiskInfo(drv, (DiskInfo*)req->buf);
	return sizeof(DiskInfo);
}

/*
 *	DN_DISKFORMAT: disk format (W)
 */
LOCAL	INT	accDISKFORMAT(DrvTab *drv, T_DEVREQ *req)
{
	ER	er, err;

	/* Check read-only : the request shall not come */
	if (drv->Spec.readonly) return E_RONLY;

	/* Format the disk : logical unit also shall be approved */
	er = (*(drv->Format))(drv, ((DiskFormat*)req->buf), req->size);

	/* In the case of physical unit, initialize the disk and re-register the device.
	   Re-register to deregister subunit also when format error occurs. */
	if (er != E_PAR && drv->CurSUnit == 0) {
		err = sdRegistDevice(drv);
		if (er == E_OK) er = err;
	}
	return (er >= E_OK) ? sizeof(DiskFormat) : er;
}

/*
 *	DN_DISKINIT: Initialize disk (W)
 */
LOCAL	INT	accDISKINIT(DrvTab *drv, T_DEVREQ *req)
{
	ER	er;

	/* Check data */
	if (*((DiskInit*)req->buf) != DISKINIT) return E_PAR;

	/* Logical unit also shall be OK'd since it is automatically executed after the format */
	if (drv->CurSUnit == 0) {
		/* Initialize disk & Re-register device */
		if ((er = sdRegistDevice(drv)) < E_OK) return er;
	}
	return sizeof(DiskInit);
}

/*
 *	DN_DISKMEMADR:	disk area top address (R)
 */
LOCAL	INT	accDISKMEMADR(DrvTab *drv, T_DEVREQ *req)
{
	/* Check the support */
	if (drv->Spec.accif != MEM_ACCIF) return E_NOSPT;

	*((void**)req->buf) = (void*)drv->IOB;
	return sizeof(void*);
}

/*
 *	DN_DISKPARTINFO: Disk partition information (R)
 */
LOCAL	INT	accDISKPARTINFO(DrvTab *drv, T_DEVREQ *req)
{
	/* Approve the logical unit only */
	if (drv->CurSUnit == 0) return E_PAR;

	((DiskPartInfo*)req->buf)->systemid =
				drv->s.SUnit[drv->CurSUnit].SystemId;
	((DiskPartInfo*)req->buf)->startblock =
				drv->s.SUnit[drv->CurSUnit].StartBlock;
	((DiskPartInfo*)req->buf)->endblock =
				drv->s.SUnit[drv->CurSUnit].EndBlock - 1;
	return sizeof(DiskPartInfo);
}

/*
 *	DN_DISKCHSINFO: Disk CHS information (R)
 */
LOCAL	INT	accDISKCHSINFO(DrvTab *drv, T_DEVREQ *req)
{
	((DiskCHSInfo*)req->buf)->cylinder = drv->nXCyl;
	((DiskCHSInfo*)req->buf)->head = drv->nXHead;
	((DiskCHSInfo*)req->buf)->sector = drv->nXSec;
	return sizeof(DiskCHSInfo);
}

/*
 *	DN_DISKIDINFO: Disk ID information (R)
 */
LOCAL	INT	accDISKIDINFO(DrvTab *drv, T_DEVREQ *req)
{
	/* Check the support */
	if (drv->Spec.accif != ATA_ACCIF &&
	    drv->Spec.accif != MMC_ACCIF) return E_NOSPT;

	memcpy(req->buf, drv->Ident, IDENT_DTSZ);
	return IDENT_DTSZ;
}

/*
 *	DN_DISKCMD: disk command (W)
 */
LOCAL	INT	accDISKCMD(DrvTab *drv, T_DEVREQ *req)
{
	/* Support "ATAPI" only */
	return E_NOSPT;
}

/*
 *	(-999999) Disk master boot record information (special processing) (R)
 */
#define	MBR_DTSZ	(512 + 4)

LOCAL	INT	accDISKMBR(DrvTab *drv, T_DEVREQ *req)
{
	ER	er;
	UW	*mp;
	BOOL	write;

	/* Check the support */
	if (!drv->SuptMBR || drv->CurSUnit != 0) return E_PAR;

	if (req->size != MBR_DTSZ) return E_PAR;

	/* Check data & Determine "R/W" */
	mp = (UW*)req->buf;
	if (*mp == (UW)CH4toW('M','B','R','R'))		write = FALSE;
	else if (*mp == (UW)CH4toW('M','B','R','W')
			&& !drv->Spec.readonly)		write = TRUE;
	else						return E_PAR;

	/* "R/W" of disk master boot record */
	er = (*(drv->ReadWrite))(drv, 0, 1, (void*)(++mp), write);

	if (er >= E_OK && write) {
		/* Disk initialization and device re-registration are not executed : ignore error */
		(*(drv->DiskInit))(drv);
	}
	return (er >= E_OK) ? MBR_DTSZ : er;
}

/*
 *	The read-in / writing request
 */
LOCAL	ER	ReadWriteReq(DrvTab *drv, T_DEVREQ *req)
{
#define	nDnT	10
static	const	struct {		/* Attribute data definition	*/
	W	dn;				/* Data number		*/
	UW	dsz:16;				/* Data size		*/
	UW	nomedia:1;			/* OK even without media */
	UW	readable:1;			/* Read OK		*/
	UW	writable:1;			/* Write OK		*/
	INT	(*acc)(DrvTab*, T_DEVREQ*);	/* Access functiuon	*/
} DnT[nDnT] = {
	{ DN_DISKEVENT, 		/* RW: "Mbf" ID for event notification	*/
		sizeof(ID),		1, 1, 1,	accDISKEVENT	},
	{ DN_DISKINFO,			/* R : Disk information		*/
		sizeof(DiskInfo),	0, 1, 0,	accDISKINFO	},
	{ DN_DISKFORMAT,		/* W : Disk format	*/
		sizeof(DiskFormat),	0, 0, 1,	accDISKFORMAT	},
	{ DN_DISKINIT,			/* W : Disk initialization	*/
		sizeof(DiskInit),	0, 0, 1,	accDISKINIT	},
	{ DN_DISKMEMADR,		/* R : Disk area top address */
		sizeof(void*),		0, 1, 0,	accDISKMEMADR	},
	{ DN_DISKPARTINFO,		/* R : Disk partition information	*/
		sizeof(DiskPartInfo),	0, 1, 0,	accDISKPARTINFO	},
	{ DN_DISKCHSINFO,		/* R : Disk CHS information	*/
		sizeof(DiskCHSInfo),	0, 1, 0,	accDISKCHSINFO	},
	{ DN_DISKIDINFO,		/* R : Disk identification information	*/
		IDENT_DTSZ,		1, 1, 0,	accDISKIDINFO	},
	{ (-999999),			/* R: Special processing (MBR)		*/
		MBR_DTSZ,		0, 1, 0,	accDISKMBR	},
	{ DN_DISKCMD,			/* W : Disk command    */
		sizeof(DiskCmd),	0, 0, 1,	accDISKCMD	}
};
	ER	er;
	W	cnt, dn, i, edn;

	/* Parameter check     */
	if ((cnt = req->size) < 0)
		{er = E_PAR; goto E_EXIT;}

	/* Set user space      */
	er = tk_set_tsp(TSK_SELF, &req->tskspc);
	if (er < E_OK) goto E_EXIT;

	if ((dn = req->start) >= 0) {		/* Specific data       */
		if (!drv->MediaOK) {
			er = ERR_NOMEDIA;	/* There is no media   */
		} else if (req->cmd == TDC_WRITE && drv->Spec.readonly) {
			er = E_RONLY;		/* Read-only (request shall not come) */
		} else {
			/* Absolutize & Check the start/end block */
			dn += drv->s.SUnit[drv->CurSUnit].StartBlock;
			edn = drv->s.SUnit[drv->CurSUnit].EndBlock;

			if (dn >= edn || (edn - dn) < cnt) {
				er = E_PAR;	/* Illegal*/
			} else if (cnt == 0) {
				er = edn - dn;	/* Return the remaining number of blocks */
			} else {		/* R/W processing */
				er = (*(drv->ReadWrite))(drv, dn, cnt,
					req->buf, req->cmd == TDC_WRITE);
			}
		}

	} else {				/* Attribute data	*/
		for (i = 0; i < nDnT && DnT[i].dn != dn; i++);

		if (i >= nDnT) {
			er = E_PAR;			/* Illegal number	*/
		} else if (DnT[i].acc == NULL) {
			er = E_NOSPT;			/* Unsupported		*/
		} else if (!drv->MediaOK && DnT[i].nomedia == 0) {
			er = ERR_NOMEDIA;		/* There is no media	*/
		} else if (((req->cmd == TDC_READ) ?
				DnT[i].readable : DnT[i].writable) == 0) {
			er = E_PAR;			/* Disable "R/W"	*/
		} else if ((er = CheckSize(cnt, DnT[i].dsz)) == E_OK) {
			er = (*(DnT[i].acc))(drv, req);	/* "R/W" processing	*/
		}
	}

	if (er < E_OK) goto E_EXIT;
	req->asize = (INT)er;
	return E_OK;

E_EXIT:
	return er;
}

/*
 *	Abort processing function
 */
EXPORT	ER	sdAbortFn(T_DEVREQ *req, GDI gdi)
{
	DrvTab	*drv;

	drv = CheckDrvTab(GDI_exinf(gdi), req->devid);
	if (! drv) return E_OK;		/* Ignore the request for invalid device */

	/* Ignore it while in "suspend" status */
	if (!drv->Suspended) {

		/* Abort it when the request from the specified task is in execution */
		if (drv->CurReq && drv->CurReq == req) {
			/* Abort request */
			(*(drv->Abort))(drv);

			/* In order to ensure no access to user space,
				wait until the completion of the request in processing */
			Lock(&drv->ProcLock);
			Unlock(&drv->ProcLock);
		}
	}
	return E_OK;
}

/*
 *	"Suspend" request
 */
LOCAL	ER	SuspendReq(DrvTab *drv)
{
	/* Ignore it while in "suspend" status */
	if (!drv->Suspended) {

		/* Wait until the completion of the request in processing instead of aborting :
			It takes time for format processing, etc., however,
			tentative waiting shall be executed without executing stop/resume. */

		Lock(&drv->ProcLock);

		drv->Suspended = TRUE;

		/* Turn OFF the power if possible */
		sdPowerOff(drv, CP_SUSPEND);

		/* Set the drive to the reset status */
		drv->Reset = TRUE;

		/* Exit the processing while locking the common processing */
	}
	return E_OK;
}

/*
 *	Resume request
 */
LOCAL	ER	ResumeReq(DrvTab *drv)
{
	/* Ignore it while it is not in "suspend" status */
	if (drv->Suspended) {
		/* Turn ON power : ignore error */
		sdPowerOn(drv, CP_RESUME);

		/* Release the suspend status/common processing lock, and resume processing */
		drv->Suspended = FALSE;
		Unlock(&drv->ProcLock);
	}
	return E_OK;
}

/*
 *	Event processing function
 */
EXPORT	INT	sdEventFn(INT evttyp, void *evtinf, GDI gdi)
{
	ER	er;
	DrvTab	*drv;

	drv = CheckDrvTab(GDI_exinf(gdi), GDI_devid(gdi));
	if (! drv) return E_NOEXS;

	switch (evttyp) {
	case TDV_SUSPEND:
		er = SuspendReq(drv);
		break;
	case TDV_RESUME:
		er = ResumeReq(drv);
		break;
	case TDV_CARDEVT:
		Lock(&drv->ProcLock);
		drv->CurSUnit = 0;
		er = sdCardEvent(evttyp, evtinf, gdi);
		Unlock(&drv->ProcLock);
		break;
	case TDV_USBEVT:
		er = E_NOSPT;
		break;
	default:	/* Undefined event */
		er = E_PAR;
	}
	return er;
}

/*
 *	Open processing function
 */
EXPORT	ER	sdOpenFn(ID devid, UINT omode, GDI gdi)
{
	ER	er;
	DrvTab	*drv;

	drv = CheckDrvTab(GDI_exinf(gdi), devid);
	if (! drv) return E_NOEXS;

	Lock(&drv->ProcLock);

	/* Open processing (checking of the absence or presence of media is also included) */
	if (drv->DriveOK && drv->OpenCnt == 0) {
		er = (*(drv->Misc))(drv, (W)DC_OPEN);
		//printf("drv->Misc failed\n");
		if (er < E_OK) goto E_EXIT;
	}

	er = ERR_NOMEDIA;

	/* Checking of the absence or presence of drive */
	if (!drv->DriveOK) goto E_EXIT;//{printf("drv->DriveOK failed\n");goto E_EXIT;}

	/* Checking of the absence or presence of media */
	if (!drv->MediaOK) {
		//printf("drv->MediaOK failed\n");
		/* In physical device, it shall be openable without media.  */
		if (drv->CurSUnit != 0) goto E_EXIT;
	}

	/* Check the existence of subunit :
	   to register the subunit for all partitions unconditionally in the case of HD.
*/
	if (drv->CurSUnit != 0 &&
	    drv->s.SUnit[drv->CurSUnit].StartBlock >=
	    drv->s.SUnit[drv->CurSUnit].EndBlock) goto E_EXIT;

	/* Open count of subunit */
	drv->s.SUnit[drv->CurSUnit].OpenCnt++;

	/* Overall open count: Turn ON the power at the first open */
	er = E_OK;
	if (drv->OpenCnt++ == 0) {
		if ((er = sdPowerOn(drv, CP_POWERON)) < E_OK) {
			drv->OpenCnt--;
			drv->s.SUnit[drv->CurSUnit].OpenCnt--;
		}
	}
E_EXIT:
	Unlock(&drv->ProcLock);
	return er;
}

/*
 *	Close processing function
 */
EXPORT	ER	sdCloseFn(ID devid, UINT option, GDI gdi)
{
	DrvTab	*drv;

	drv = CheckDrvTab(GDI_exinf(gdi), devid);
	if (! drv) return E_NOEXS;

	Lock(&drv->ProcLock);

	/* Open count of subunit */
	if (drv->s.SUnit[drv->CurSUnit].OpenCnt > 0) {
		drv->s.SUnit[drv->CurSUnit].OpenCnt--;
	}

	/* Turn OFF the power at the last close */
	if (--drv->OpenCnt <= 0) {
		drv->OpenCnt = 0;
		if (drv->DriveOK) {
			/* Close processing */
			(*(drv->Misc))(drv, (option & TD_EJECT) ?
				       DC_CLOSEALL : DC_CLOSE);
			/* Power-OFF*/
			sdPowerOff(drv, CP_POFF);
		}
	}

	Unlock(&drv->ProcLock);
	return E_OK;
}

/*
 *	Device request (read/write)
 */
EXPORT	void	sdAcceptRequest(DrvTab *drv)
{
	T_DEVREQ	*req;
	INT		er;

	/* Infinite loop of the request accept */
	for (;;) {
		/* Accept a processing request */
		er = GDI_Accept(&req, NORMAL_REQPTN, drv->ReqTmout, drv->Gdi);

		if (er < E_OK || (er & DRP_NORMREQ) == 0) {
			if(er >= E_OK || er == E_TMOUT) {
				/* Time-out processing */
				(*(drv->Misc))(drv, DC_TSKTMO);
			}
			continue;
		}

		drv = CheckDrvTab(drv, req->devid);
		if (! drv) er = E_NOEXS;
		else {
			Lock(&drv->ProcLock);
			if (!drv->DriveOK) er = ERR_NOMEDIA;
			else {
				drv->CurReq = req;	/* Set the abort target */
				er = ReadWriteReq(drv, req);
				drv->CurReq = NULL;	/* Release the abort target */
				drv->Aborted = FALSE;	/* Release the abort request */
			}
			Unlock(&drv->ProcLock);
		}

		/* Reply */
		req->error = er;
		GDI_Reply(req, drv->Gdi);
	}
}
