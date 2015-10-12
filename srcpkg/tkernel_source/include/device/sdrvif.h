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
 *	sdrvif.h (device)
 *
 *	Simple device driver I/F layer
 *
 *	Used for simple devices which operation can be processed without
 *	switching to wait mode. Used with very simple devices.
 *
 *	Except where otherwise stated, the functions defined here cannot be
 *	called from the task-independent part or when dispatch or interrupt
 *	disabled.
 */

#ifndef	__DEVICE_SDRVIF_H__
#define	__DEVICE_SDRVIF_H__

#include <basic.h>
#include <tk/devmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Driver I/F access handle
 */
typedef struct SimpleDriverInterface *	SDI;

/*
 * Device registration information
 */
typedef struct {
	void*	exinf;		/* Extended information (may be optional) */
	UB	devnm[L_DEVNM+1]; /* Physical device name */
	ATR	drvatr;		/* Driver attributes */
	ATR	devatr;		/* Device attributes */
	INT	nsub;		/* Number of subunits */
	INT	blksz;		/* Unique data block size (-1 = unknown) */

/*
 * Processing function (set to NULL if processing function not required)
 *	In all cases, processing must be completed promptly without
 *	switching to wait mode on an irregular basis.
 *	The processing function calls only one task at a time, because
 *	the driver I/F exercises exclusive control.
 *	Processing functions are executed as quasi-task portions in the
 *	request task (application task) context. Since processing
 *	functions operate in the request task context, in the event of a
 *	change in task priority or other change, it is necessary to
 *	return them to their original state before returning from the
 *	processing function.
 *	The read/write return value is either the size of the input/output
 *	result or an error.
 *	A buf area check (ChkSpace) has been implemented in the driver I/F.
 */
	ER  (*open )( ID devid, UINT omode, SDI );
	ER  (*close)( ID devid, UINT option, SDI );
	INT (*read )( ID devid, INT start, INT size, void *buf, SDI );
	INT (*write)( ID devid, INT start, INT size, void *buf, SDI );
	INT (*event)( INT evttyp, void *evtinf, SDI );
} SDefDev;

/* Driver attributes */
#ifndef	TDA_OPENREQ
#define	TDA_OPENREQ	0x0001	/* Each time open/close */
#endif

/*
 * Device registration
 *	Registers the device in accordance with the ddev registration
 *	information.
 *	Initial device information is returned in idev. If idev = NULL,
 *	information is not returned.
 *	The driver I/F access handle is returned in SDI.
 * Update
 *	Updates the SDI device registration in accordance with the ddev
 *	registration information. The device name (devnm) cannot
 *	(and must not) be changed.
 *	The update process does not alter the device ID.
 * Delete
 *	Deletes the SDI device registration.
 */
IMPORT ER SDefDevice( SDefDev *ddev, T_IDEV *idev, SDI* );	/* Register */
IMPORT ER SRedefDevice( SDefDev *ddev, SDI );			/* Update */
IMPORT ER SDelDevice( SDI );					/* Delete */

/*
 * Get information from SDI
 *	These functions can be called from the task-independent part
 *	and when dispatch or interrupt is disabled.
 */
IMPORT ID SDI_devid( SDI );		/* Physical device ID */
IMPORT void* SDI_exinf( SDI );		/* Extended information (exinf) */
IMPORT const SDefDev* SDI_ddev( SDI );	/* Registration information */

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_SDRVIF_H__ */
