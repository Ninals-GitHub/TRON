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
 *	common.c	System disk driver
 *
 *	Various common processings
 */

#include "sdisk.h"

/*
 *	Event notification
 */
EXPORT	void	sdSendEvt(DrvTab *drv, W kind)
{
	W	i;
	DiskEvt	evt;
	ER	er;

	evt.h.evttyp = (kind == TDE_EJECT && drv->OpenCnt != 0) ?
			TDE_ILLEJECT : kind;
	evt.h.devid = drv->DevId;

	/* Set the subunit's open status to "info" */
	evt.info = 0;
	for (i = 0; i <= drv->nSUnit; i++) {
		if (drv->s.SUnit[i].OpenCnt > 0) evt.info |= 1 << i;
	}

	/* Send to the message buffer for event notification:ignore error */
	er = tk_snd_mbf(drv->MbfId, (void*)&evt, sizeof(evt), TMO_EVENT);
}

/*
 *	Set the device-support processing function
 */
EXPORT	void	sdSetUpAccFn(DrvTab *drv)
{
	switch(drv->Spec.accif) {
	case MEM_ACCIF:
		drv->Abort = memAbort;
		drv->ReadWrite = memReadWrite;
		drv->Format = memFormat;
		drv->Identify = memIdentify;
		drv->DiskInit = memDiskInit;
		drv->Misc = memMisc;
		drv->DiskFmt = DiskFmt_MEM;
		drv->IntHdr = NULL;
		break;
	case ATA_ACCIF:
		drv->Abort = ataAbort;
		drv->ReadWrite = ataReadWrite;
		drv->Format = ataFormat;
		drv->Identify = ataIdentify;
		drv->DiskInit = ataDiskInit;
		drv->Misc = ataMisc;
		drv->DiskFmt = DiskFmt_STD;
		drv->IntHdr = ataIntHdr;
		break;
	case MMC_ACCIF:
		drv->Abort = mmcAbort;
		drv->ReadWrite = mmcReadWrite;
		drv->Format = mmcFormat;
		drv->Identify = mmcIdentify;
		drv->DiskInit = mmcDiskInit;
		drv->Misc = mmcMisc;
		drv->DiskFmt = DiskFmt_STD;
		drv->IntHdr = NULL;
		break;
	}
}

/*
 *	Create a disk information (DiskInfo)
 */
EXPORT	void	sdMakeDiskInfo(DrvTab *drv, DiskInfo *inf)
{
	memset(inf, 0, sizeof(DiskInfo));
	inf->format = drv->DiskFmt;
	inf->protect = drv->Spec.readonly ? TRUE : FALSE;
	inf->removable = drv->Spec.eject;
	inf->blocksize = drv->SecSize;
	inf->blockcont = drv->s.SUnit[drv->CurSUnit].EndBlock -
			 drv->s.SUnit[drv->CurSUnit].StartBlock;
}

/*
 *	Device registration
 */
EXPORT	ER	sdRegistDevice(DrvTab *drv)
{
	ER		er, err;
	GDefDev		ddev;
	DiskInfo	diskinfo;

	/* Disk initialization (read in the partition information)
		Continue processing to register physical device also when error occurs */
	er = (*(drv->DiskInit))(drv);

	/* Set "DiskInfo" */
	sdMakeDiskInfo(drv, &diskinfo);

	/* Registration information */
	ddev = *GDI_ddev(drv->Gdi);
	ddev.drvatr = (drv->Spec.lockreq) ? TDA_LOCKREQ : 0;

	switch(drv->Spec.accif) {
	case MEM_ACCIF:	ddev.devatr = (drv->Spec.readonly) ?
					TDK_DISK_ROM : TDK_DISK_RAM;
			break;
	case ATA_ACCIF:	ddev.devatr = TDK_DISK_HD;
			break;
	case MMC_ACCIF:	ddev.devatr = TDK_DISK_FLA;
			break;
	default:	ddev.devatr = TDK_DISK_UNDEF;
	}

	ddev.devatr |= (drv->DrvNo & 0x3F) << 16;
	ddev.devatr |= (drv->Spec.eject) ? TD_REMOVABLE : 0;
	ddev.devatr |= (drv->Spec.readonly) ? TD_PROTECT : 0;

	ddev.blksz = drv->SecSize;
	ddev.nsub = drv->nSUnit;

	/* (Re)register the device */
	err = GRedefDevice(&ddev, drv->Gdi);
	if (err < E_OK) goto E_EXIT;
	return	er;
E_EXIT:
	return err;
}

/*
 *	Fetch the current time (msec)
 */
EXPORT	UW	sdGetMsec(void)
{
	SYSTIM	ctm;
	UW	ct;

	tk_get_otm(&ctm);		/* Current time : monotone increasing	*/

	ct = ctm.lo;
	return (ct == 0) ? 1 : ct;	/* Avoid 0		*/
}

/*
 *	Check the time-out (msec unit)
 */
EXPORT	BOOL	sdChkTmout(UW *tm, UW tmo, W delay)
{
	UW	ct;

	ct = sdGetMsec();			/* Current time	*/
	if (*tm == 0) *tm = ct;			/* Start setting */
	else if (ct - *tm >= tmo) return TRUE;	/* Time-out */

	tk_dly_tsk(delay);			/* Wait	*/
	return FALSE;
}
