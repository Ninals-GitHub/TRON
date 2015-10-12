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
 *	main.c		Clock driver : main
 *			System-independent
 */
#include "clkdrv.h"
#include <sys/util.h>


/* Message buffer for event notification */
LOCAL	ID	evtMbf;

/* Driver I/F access handle */
LOCAL	SDI	ClkSdi;

/*
 *	Check the parameter
 *		= E_OK : OK (size == 0)
 *		> E_OK : OK (size > 0)
 *		< E_OK : Error
 */
LOCAL	ER	CheckPar(W size, W dsz)
{
	if (size < 0 || (size > 0 && size < dsz)) return E_PAR;
	return size ? (E_OK + 1) : E_OK;
}

/*
 *	Input-output request
 */
LOCAL	INT	rwfn(BOOL rd, ID devid, INT start, INT size, void *buf, SDI sdi)
{
	INT	er;
	W	dsz;

	switch (start) {
	case DN_CKEVENT:	/* RW: Event notification message buffer ID */
		er = CheckPar(size, dsz = sizeof(W));
		if (er > E_OK) {
			if (rd) {
				*(W*)buf = evtMbf;
			} else {
				evtMbf = *(W*)buf;
			}
		}
		break;

	case DN_CKDATETIME:	/* RW: Current time */
		er = CheckPar(size, dsz = sizeof(DATE_TIM));
		if (er > E_OK) {
			if (rd) {
				er = cdGetDateTime(buf);
			} else {
				er = cdSetDateTime(buf);
			}
		}
		break;

	case DN_CKAUTOPWON:	/* RW: Automatic power-ON time	*/
		er = CheckPar(size, dsz = sizeof(DATE_TIM));
		if (er > E_OK) {
			if (rd) {
				er = cdGetAutoPwOn(buf);
			} else {
				er = cdSetAutoPwOn(buf);
			}
		}
		break;

	case DN_CKREGISTER:	/* RW: Unvolatile register */
		if (rd) {
			er = cdGetRegister(buf, size);
		} else {
			er = cdSetRegister(buf, size);
		}
		dsz = er;
		break;

	default:		/* Data number error */
		dsz = er = E_PAR;
		break;
	}

	if (er >= E_OK) {
		er = dsz;
	} else {
		DEBUG_PRINT(("rwData dn:%d err = %d\n", start, er));
	}

	return er;
}

/*
 *	Input request processing function
 */
LOCAL	INT	readfn(ID devid, INT start, INT size, void *buf, SDI sdi)
{
	return rwfn(TRUE, devid, start, size, buf, sdi);
}

/*
 *	Output request processing function
 */
LOCAL	INT	writefn(ID devid, INT start, INT size, void *buf, SDI sdi)
{
	return rwfn(FALSE, devid, start, size, buf, sdi);
}

/*
 *	Clock driver entry
 */
EXPORT	ER	ClockDrv(INT ac, UB *av[])
{
	ER	er;
	T_IDEV	idev;
static	SDefDev	ddev = {
		NULL,		/* exinf */
		"CLOCK",	/* devnm */
		0,		/* drvatr */
		0,		/* devatr */
		0,		/* nsub */
		1,		/* blksz */
		NULL,		/* open */
		NULL,		/* close */
		readfn,		/* read */
		writefn,	/* write */
		NULL,
	};

	/* End processing */
	if (ac < 0) {
		/* Hardware end processing */
		cdFinishHardware();

		/* Device deregistrtaion */
		SDelDevice(ClkSdi);
		return E_OK;
	}

	/* Hardware initial setting */
	er = cdInitHardware();
	if (er < E_OK) goto err_ret;

	/* Device registration */
	er = SDefDevice(&ddev, &idev, &ClkSdi);
	if (er < E_OK) goto err_ret;

	/* Set the message buffer for event notification to insert the event when the event named "default" occurs */
	evtMbf = idev.evtmbfid;

err_ret:
	DEBUG_PRINT(("ClockDriver er = %d\n", er));
	return er;
}
