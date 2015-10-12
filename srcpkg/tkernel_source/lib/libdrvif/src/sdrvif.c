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
 *	sdrvif.c
 *
 *	Simple device driver I/F layer
 */

#include <basic.h>
#include <device/sdrvif.h>
#include <tk/tkernel.h>
#include <tk/util.h>
#include <sys/debug.h>

/*
 * Driver I/F access handle (SDI)
 */
struct SimpleDriverInterface {
	FastLock	lock;	/* Lock for exclusive access control */
	SDefDev		def;	/* Device registration information */
	ID		devid;	/* Device ID*/
};

#define	LockSDI(sdi)	Lock(&(sdi)->lock)
#define	UnlockSDI(sdi)	Unlock(&(sdi)->lock)

/* ------------------------------------------------------------------------ */
/*
 *	Get information from "SDI"
 *	These functions shall be call-ready even at the task independent part,
 *	during disabling the dispatch, and during disabling the interrupt.
 */

/*
 * Get physical device ID
 */
EXPORT ID SDI_devid( SDI sdi )
{
	return sdi->devid;
}

/*
 * Get the extended information (SDefDev.exinf)
 */
EXPORT void* SDI_exinf( SDI sdi )
{
	return sdi->def.exinf;
}

/*
 * Get the registration information
 */
EXPORT const SDefDev* SDI_ddev( SDI sdi )
{
	return &sdi->def;
}

/* ------------------------------------------------------------------------ */
/*
 *	Processing function
 */

/*
 * Open function
 */
LOCAL ER sdi_openfn( ID devid, UINT omode, SDI sdi )
{
	ER	err;

	if ( sdi->def.open == NULL ) return E_OK;

	LockSDI(sdi);
	err = (*sdi->def.open)(devid, omode, sdi);
	UnlockSDI(sdi);

	return err;
}

/*
 * Close cunction
 */
LOCAL ER sdi_closefn( ID devid, UINT option, SDI sdi )
{
	ER	err;

	if ( sdi->def.close == NULL ) return E_OK;

	LockSDI(sdi);
	err = (*sdi->def.close)(devid, option, sdi);
	UnlockSDI(sdi);

	return err;
}

/*
 * Processing start function
 */
LOCAL ER sdi_execfn( T_DEVREQ *req, TMO tmout, SDI sdi )
{
	INT	(*fp)( ID devid, INT start, INT size, void *buf, SDI );
	INT	memsz;
	ER	err;

	fp = ( req->cmd == TDC_READ )? sdi->def.read: sdi->def.write;
	if ( fp == NULL ) { err = E_NOSPT; goto err_ret; }

	/* I/O size */
	if ( req->start < 0 ) {
		/* Attribute data */
		memsz = req->size;
	} else {
		/* Specific data */
		if ( sdi->def.blksz <= 0 ) { err = E_PAR; goto err_ret; }
		memsz = req->size * sdi->def.blksz;
	}
	if ( memsz < 0 ) { err = E_PAR; goto err_ret; }

	if ( memsz > 0 ) {
		/* Check the "buf" space */
		if ( req->cmd == TDC_READ ) {
			err = ChkSpaceRW(req->buf, memsz);
		} else {
			err = ChkSpaceR(req->buf, memsz);
		}
		if ( err < E_OK ) goto err_ret;
	}

	/* I/O processing */
	LockSDI(sdi);
	err = (*fp)(req->devid, req->start, req->size, req->buf, sdi);
	UnlockSDI(sdi);
	if ( err < E_OK ) goto err_ret;

	req->asize = (INT)err;
	req->error = E_OK;

	return E_OK;

err_ret:
	req->error = err;
	return E_OK;
}

/*
 * Wait-for-completion function
 */
LOCAL INT sdi_waitfn( T_DEVREQ *req, INT nreq, TMO tmout, SDI sdi )
{
	return 0;
}

/*
 * Abort processing function
 */
LOCAL ER sdi_abortfn( ID tskid, T_DEVREQ *req, INT nreq, SDI sdi )
{
	return E_OK;
}

/*
 * Event function
 */
LOCAL INT sdi_eventfn( INT evttyp, void *evtinf, SDI sdi )
{
	INT	ret;

	if ( sdi->def.event == NULL ) return E_OK;

	LockSDI(sdi);
	ret = (*sdi->def.event)(evttyp, evtinf, sdi);
	UnlockSDI(sdi);

	return ret;
}

/* ------------------------------------------------------------------------ */
/*
 *	Device registration
 */

/*
 * Device registration processing
 */
LOCAL ER sdi_defdevice( SDI sdi, T_IDEV *idev )
{
	T_DDEV	ddev;
	ER	err;

	ddev.exinf   = sdi;
	ddev.drvatr  = sdi->def.drvatr;
	ddev.devatr  = sdi->def.devatr;
	ddev.nsub    = sdi->def.nsub;
	ddev.blksz   = sdi->def.blksz;
	ddev.openfn  = (FP)sdi_openfn;
	ddev.closefn = (FP)sdi_closefn;
	ddev.execfn  = (FP)sdi_execfn;
	ddev.waitfn  = (FP)sdi_waitfn;
	ddev.abortfn = (FP)sdi_abortfn;
	ddev.eventfn = (FP)sdi_eventfn;

	err = tk_def_dev(sdi->def.devnm, &ddev, idev);
	if ( err < E_OK ) goto err_ret;

	sdi->devid = (ID)err;

	return E_OK;

err_ret:
	DEBUG_PRINT(("sdi_defdevice err = %d\n", err));
	return err;
}

/*
 * Device registration
 *	Device shall be registered in accordance with "ddev" registration information.
 *	Device initialization information is returned to "idev".
 *	It is not returned when "idev = NULL"
 *	Driver I/F access handle shall be returned to "SDI".
 */
EXPORT ER SDefDevice( SDefDev *ddev, T_IDEV *idev, SDI *p_sdi )
{
	SDI	sdi;
	ER	err;

	/* Create "SDI"*/
	sdi = Kmalloc(sizeof(*sdi));
	if ( sdi == NULL ) { err = E_NOMEM; goto err_ret1; }

	sdi->def = *ddev;

	/* Create the lock for exclusive access control */
	err = CreateLock(&sdi->lock, sdi->def.devnm);
	if ( err < E_OK ) goto err_ret2;

	/* Device registration */
	err = sdi_defdevice(sdi, idev);
	if ( err < E_OK ) goto err_ret3;

	*p_sdi = sdi;
	return E_OK;

err_ret3:
	DeleteLock(&sdi->lock);
err_ret2:
	Kfree(sdi);
err_ret1:
	DEBUG_PRINT(("SDefDevice err = %d\n", err));
	return err;
}

/*
 * Update
 *	SDI device registration shall be updated in accordance with "ddev" registration information.
 *	Device name (devnm) cannot be changed (must not be changed).
 *	Device ID is not changed when updated.
 */
EXPORT ER SRedefDevice( SDefDev *ddev, SDI sdi )
{
	ER	err;

	sdi->def = *ddev;

	/* Update the device registration */
	err = sdi_defdevice(sdi, NULL);
	if ( err < E_OK ) goto err_ret;

	return E_OK;

err_ret:
	DEBUG_PRINT(("SRedefDevice err = %d\n", err));
	return err;
}

/*
 * Deregistration
 *	Deregister "SDI" device
 */
EXPORT ER SDelDevice( SDI sdi )
{
	ER	err, error = E_OK;

	/* Deregister device */
	err = tk_def_dev(sdi->def.devnm, NULL, NULL);
	if ( err < E_OK ) error = err;

	/* Delete the lock for exclusive access control */
	DeleteLock(&sdi->lock);

	/* Delete "SDI" */
	Kfree(sdi);

#ifdef DEBUG
	if ( error < E_OK ) DEBUG_PRINT(("SDelDevice err = %d\n", error));
#endif
	return error;
}

/* ------------------------------------------------------------------------ */
