/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/10/11
 *
 *----------------------------------------------------------------------
 */


/*
 *	main.c		System disk driver
 *
 *	driver main
 */

#include "sdisk.h"
#include "tm/tmonitor.h"
#include "device/devconf.h"

#include <sys/sysinfo.h>
#include <tk/task.h>

/* Get "DEVCONF" entry */
IMPORT W GetDevConf( UB *name, W *val );

#define	SYSDISK_PRI	45

EXPORT	DrvTab	*TopDrvT[4 + 1] =		/* Top-drive table	*/
	{NULL, NULL, NULL, NULL, (DrvTab*)(-1)};

/*
 *	Object disk information
 */
/* MMC/SD card definition */
LOCAL	SDInfo	mmc_drv[] = {
	{1,	MMC_CARD, "pcb", (UW)CH4toW('m','m','c','d'),
			PC_DRV + 1,		0,	0	}
};

/* ROM disk definition */
LOCAL	SDInfo	rd_drv[] = {
	{3,	ROM_DISK, "rda", (UW)CH4toW('m','d','s','k'),
			MEM_DRV + 0,		0,	0	},
};
/* RAM disk definition */
LOCAL	SDInfo	md_drv[] = {
	{4,	RAM_DISK, "mda", (UW)CH4toW('m','a','d','k'),
			MEM_DRV + 1,		0,	0	},
};

/*
 *	Create task
 */
LOCAL	ID	CreTask(FP entry, INT par, UW name4)
{
	T_CTSK	ctsk;
	ID	tskid;
	ER	er;
	static int dssn=0;

	/* Task creation */
	ctsk.exinf = (void*)name4;
	ctsk.task = entry;
	ctsk.itskpri = SYSDISK_PRI;
	ctsk.stksz = DRVTSK_STKSZ;
	ctsk.tskatr = TA_HLNG | TA_RNG0;
	
	ctsk.dsname[0] = 's';
	ctsk.dsname[1] = 'd';
	ctsk.dsname[2] = 'd';
	ctsk.dsname[3] = 0x30 + dssn++;
	ctsk.dsname[4] = '\0';
	
	tskid = er = tk_cre_tsk(&ctsk);
	if (er >= E_OK) {
		/* Task start-up */
		er = tk_sta_tsk(tskid, par);
		if (er < E_OK) tk_del_tsk(tskid);
	}
	return (er >= E_OK) ? tskid : E_SYS;
}

/*
 *	System disk driver : common task (accept the common request)
 */
LOCAL	void	ExecTask(SDInfo	*inf)
{
	ER	er;
	DrvTab	*drv, *ndrv;
	T_IDEV	idev;
	GDefDev	ddev = {
		NULL,		/* exinf */
		"",		/* devnm */
		1,		/* maxreqq */
		0,		/* drvatr */
		0,		/* devatr */
		0,		/* nsub */
		-1,		/* blksz */
		sdOpenFn,	/* open */
		sdCloseFn,	/* close */
		sdAbortFn,	/* abort */
		sdEventFn,	/* event */
	};

	/* Create the data for drive processing */
	for (ndrv = NULL; ; inf++, ndrv = drv) {

		/* Get & Clear the drive information area  */
		if (!(drv = (DrvTab*)Kcalloc(1, sizeof(DrvTab)))) {
			goto E_EXIT;
		}
		
		drv->Next = ndrv;
		
		/* Set the configuration data */
		drv->Spec = inf->spec;
		strncpy(drv->DevName, inf->devnm, L_DEVNM);
		drv->DrvNo = inf->drvno;
		drv->DrvBit = 0x01 << (inf->drvno & 0x1F);
		drv->IOB = inf->iob;
		drv->MiscPar = inf->misc;

		/* Create the lock for common processing */
		er = CreateLock(&drv->ProcLock, (B*)&inf->name4);
		if (er < E_OK) goto E_EXIT;

		/* Execute a lock until the completion of initialization */
		Lock(&drv->ProcLock);

		/* Get the physical device ID */
		ddev.exinf = drv;
		strncpy(ddev.devnm, inf->devnm, L_DEVNM);
		er = GDefDevice(&ddev, &idev, &drv->Gdi);
		if (er < E_OK) goto E_EXIT;

		/* Set the device-support processing function */
		sdSetUpAccFn(drv);

		/* Other initializations */
		drv->MbfId = idev.evtmbfid;
		drv->DevId = GDI_devid(drv->Gdi);
		drv->ReqTmout = TMO_FEVR;
		drv->SecSize = drv->Spec.blocksz << 9;

		/* The processing in accordance with device specification */
		if (drv->Spec.pccard) {		/* PC card type */
			/* Initialize the card */
			er = sdInitCard(drv);

		} else if (drv->IOB != 0) {	/* Fixation I/F */
			/* Register the interrupt handler */
			er = sdDefIntHdr(drv, TRUE);
			//printf("sdDefIntHdr : %d\n", er);
			if (er >= E_OK) {
				/* Check the existence of disk & Register the device */
				er = (*(drv->Identify))(drv, FALSE);
				//printf("drv->Identify : %d\n", er);
				if (er >= E_OK) {
					drv->DriveOK = TRUE;
					sdRegistDevice(drv);
				} else {
					/* Release the interrupt handler */
					sdDefIntHdr(drv, FALSE);
				}
			}
		} else {			/* Others */
			vd_printf("others 0x%08X\n", drv->IOB);
			er = E_OK;
		}
		if (er < E_OK) {		/* Release the drive information */
			Kfree((void*)drv);
			drv = ndrv;
		}
		if (inf->num != 0) break;
	}
	if (drv == NULL) {er = E_NOEXS; goto E_EXIT;}
	/* Set the top drive */
	for (ndrv = drv; ndrv != NULL; ndrv = ndrv->Next) ndrv->Top = drv;
	TopDrvT[inf->num - 1] = drv;

	/* Initialize the device-support task */
	if (drv->Misc) {
		(*(drv->Misc))(drv, DC_TSKINIT);
	}
	/* Release the lock */
	for (ndrv = drv; ndrv != NULL; ndrv = ndrv->Next) {
		Unlock(&ndrv->ProcLock);
	}
	
	/* Start the acceptance of device processing request */
	sdAcceptRequest(drv);

	/* It does not come here because it does not exit */
E_EXIT:
	vd_printf("exit\n");
	while ((ndrv = drv) != NULL) {
		DeleteLock(&ndrv->ProcLock);
		GDelDevice(ndrv->Gdi);
		drv = ndrv->Next;
		Kfree((void*)ndrv);
	}
	tk_exd_tsk();
}

/*
 *	System disk driver : entry
 */
EXPORT	ER	SysDiskDrv(INT ac, UB *av[])
{
	W	v[L_DEVCONF_VAL];
	UW	*mp;

	if (ac < 0) {
		/* End processing: execute especially nothing because it is end of system */
		return E_OK;
	}
	
	/* Driver for MMC/SD card */
	CreTask((FP)ExecTask, (INT)&mmc_drv[0], mmc_drv[0].name4);
	
#if 0
	/* Driver for ROM disk (rda): fetch the information from T-Monitor */
	if (tm_extsvc(0x01, (INT)&mp, 0, 0) >= E_OK) {
		/*	mp[0] : ROM disk types
		 *	mp[1] : ROM disk block size
		 *	mp[2] : ROM disk start address
		 *	mp[3] : ROM disk end address
		 */
		if (mp[0] == 1 && mp[1] == 512) {
			rd_drv[0].iob = (UW)mp[2];
			rd_drv[0].misc = mp[3] - mp[2];
			CreTask((FP)ExecTask, (INT)&rd_drv[0],rd_drv[0].name4);
		}
	}
#endif
	/* Driver for RAM disk (mda) : DEVCONF: RAMDISK  size */
#if _STD_X86_
	if (isInitramfs()) {
		vd_printf("ramdisk ");
		md_drv[0].iob = (UW)getInitramfsAddress();
		md_drv[0].misc = getInitramfsSize();
		vd_printf("CreTask:%d\n", CreTask((FP)ExecTask, (INT)&md_drv[0], md_drv[0].name4));
		vd_printf("E_SYS:%d\n", E_SYS);
	}
#else
	if (GetDevConf("RAMDISK", v) > 0) {
		mp = (v[0] <= 0) ? NULL : (UW*)Kcalloc(1, v[0]);
		md_drv[0].iob = (UW)mp;
		md_drv[0].misc = (mp == NULL) ? 0 : v[0];
		CreTask((FP)ExecTask, (INT)&md_drv[0], md_drv[0].name4);
	}
#endif


	return E_OK;
}
