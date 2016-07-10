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
        main.C          screen driver : main part
 *
 */
#include "screen.h"

LOCAL	BOOL	suspended;		/* suspended state     */
LOCAL	SDI	ScrSdi;			/* device driver I/F handle */

#define	Read	0x01
#define	Write	0x02
#define	R_OK	Read
#define	W_OK	Write
#define	RW_OK	(R_OK | W_OK)

/*
        check parameter & set up address space
		= E_OK : OK (size == 0)
		> E_OK : OK (size > 0)
                < E_OK : error
*/
LOCAL	ER	checkParam(INT mode, INT size, W dsz, W okpat)
{
	if (dsz <= 0) return E_OBJ;

	if ((mode & okpat) == 0) return E_PAR;

	if (size < 0 || (size > 0 && size < dsz)) return E_PAR;

	return (size == 0) ? E_OK : (E_OK + 1);
}
/*
        perform I/O requests
*/
LOCAL	INT	rwfn(INT mode, INT start, INT size, void *buf)
{
	INT	er;
	W	dsz;
	BOOL	set = (mode == Write);

	switch (start) {
	case DN_SCRSPEC:
		dsz = sizeof(DEV_SPEC);
		if ((er = checkParam(mode, size, dsz, R_OK)) > E_OK)
			er = getSCRSPEC((DEV_SPEC*)buf);
		break;
	case DN_SCRLIST:
		dsz = getSCRLIST(NULL);
		if ((er = checkParam(mode, size, dsz, R_OK)) > E_OK)
			er = getSCRLIST((TC*)buf);
		break;
	case DN_SCRNO:
		dsz = sizeof(W);
		if ((er = checkParam(mode, size, dsz, RW_OK)) > E_OK)
			er = getsetSCRNO((W*)buf, suspended, set);
		break;
	case DN_SCRCOLOR:
		dsz = getsetSCRCOLOR(NULL, FALSE);
		if ((er = checkParam(mode, size, dsz, RW_OK)) > E_OK)
			er = getsetSCRCOLOR((COLOR*)buf, set);
		break;
	case DN_SCRBMP:
		dsz = sizeof(BMP);
		if ((er = checkParam(mode, size, dsz, R_OK)) > E_OK)
			er = getSCRBMP((BMP*)buf);
		break;
	case DN_SCRBRIGHT:
		dsz = sizeof(W);
		if ((er = checkParam(mode, size, dsz, RW_OK)) > E_OK)
			er = getsetSCRBRIGHT((W*)buf, set);
		break;
	case DN_SCRUPDFN:
		dsz = sizeof(FP);
		if ((er = checkParam(mode, size, dsz, R_OK)) > E_OK)
			er = getSCRUPDFN((FP*)buf);
		break;
	case DN_SCRVFREQ:
		dsz = sizeof(W);
		if ((er = checkParam(mode, size, dsz, RW_OK)) > E_OK)
			er = getsetSCRVFREQ((W*)buf, set);
		break;
	case DN_SCRADJUST:
		dsz = sizeof(ScrAdjust);
		if ((er = checkParam(mode, size, dsz, RW_OK)) > E_OK)
			er = getsetSCRADJUST((ScrAdjust*)buf, set);
		break;
	case DN_SCRDEVINFO:
		dsz = sizeof(ScrDevInfo);
		if ((er = checkParam(mode, size, dsz, R_OK)) > E_OK)
			er = getSCRDEVINFO((ScrDevInfo*)buf);
		break;
	case DN_SCRMEMCLK:
		dsz = 0;
		er = E_NOSPT;
		break;
	case DN_SCRUPDRECT:
		dsz = sizeof(RECT);
		if ((er = checkParam(mode, size, dsz, W_OK)) > E_OK)
			er = setSCRUPDRECT((RECT*)buf);
		break;
	case DN_SCRWRITE:
		dsz = size;
		if ((er = checkParam(mode, size, dsz, W_OK)) > E_OK)
			er = setSCRWRITE(0, buf, dsz);
		break;
	default:
		if (start <= DN_SCRXSPEC(1) && start >= DN_SCRXSPEC(255)) {
			dsz = sizeof(DEV_SPEC);
			if ((er = checkParam(mode, size, dsz, R_OK)) > E_OK)
				er = getSCRXSPEC((DEV_SPEC*)buf,
						 DN_SCRXSPEC(1) - start);
		} else {
			dsz = 0;
			er = E_PAR;
		}
		break;
	}
	return (er < E_OK) ? er : dsz;
}
/*
        input processing
*/
LOCAL	INT	readfn(ID devid, INT start, INT size, void *buf, SDI sdi)
{
	return rwfn(Read, start, size, buf);
}
/*
        output processing
*/
LOCAL	INT	writefn(ID devid, INT start, INT size, void *buf, SDI sdi)
{
	return rwfn(Write, start, size, buf);
}
/*
        event processing (suspend / resume)
*/
LOCAL	INT	eventfn(INT evttyp, void *evtinf, SDI sdi)
{
	INT	er;

	switch (evttyp) {
	case	TDV_SUSPEND:
		if (suspended == FALSE) {
			suspendSCREEN();
			suspended = TRUE;
		}
		er = E_OK;
		break;

	case	TDV_RESUME:
		if (suspended == TRUE) {
			suspended = FALSE;
			resumeSCREEN();
		}
		er = E_OK;
		break;

	default:
		er = E_ILUSE;
	}

	return er;
}
/*
        startup
*/
EXPORT	ER	ScreenDrv(INT ac, UB *av[])
{
	ER	er;
	T_IDEV	idev;
	SDefDev	ddev = {
		NULL,		/* exinf */
		"SCREEN",	/* devnm */
		0,		/* drvatr */
		0,		/* devatr */
		0,		/* nsub */
		1,		/* blksz */
		NULL,		/* open */
		NULL,		/* close */
		readfn,		/* read */
		writefn,	/* write */
		eventfn,	/* event */
	};
	W	v[L_DEVCONF_VAL];

        /* effective? */
	if (GetDevConf("ScreenDrvEnable", v) > 0 && !v[0]) return E_NOSPT;

        /* epilog processing */
	if (ac < 0) {
		if (Vinf.attr & NEED_FINPROC) (*(Vinf.fn_setmode))(-1);
		return E_OK;
	}
        /* initialization */
	suspended = FALSE;

        /* device initialization processing */
	if ((er = initSCREEN()) < E_OK) goto EEXIT;

        /* register device */
	er = SDefDevice(&ddev, &idev, &ScrSdi);
	if (er < E_OK) finishSCREEN();

 EEXIT:
	return er;
}
