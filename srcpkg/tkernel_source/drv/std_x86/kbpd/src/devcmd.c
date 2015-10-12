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
 *	devcmd.c
 *
 *       KB/PD device manager
 *       I/O driver command transmission
 */

#include "kbpd.h"

/*
 * send command to a specified eventflag
 */
LOCAL ER sendDeviceCommandTo( ID flgid, UW cmd )
{
	UINT	rdy;
	ER	ercd;

        /* wait until transmission ready */
	ercd = tk_wai_flg(flgid, DeviceCommandReady, TWF_ORW|TWF_CLR,
			  &rdy, TMO_FEVR);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("sendDeviceCommandTo, wait err = %d\n", ercd));
		return ercd;
	}

        /* send */
	ercd = tk_set_flg(flgid, cmd);
	if ( ercd != E_OK ) {
		DEBUG_PRINT(("sendDeviceCommandTo, send err = %d\n", ercd));
		return ercd;
	}

	return E_OK;
}

/*
 * send device command
 */
EXPORT ER kpSendDeviceCommand( UW cmd )
{
	ID	id;
	ER	err = E_OK;
	ER	er;
	int	i;

	for ( i = 0; i < MaxCmd; ++i ) {
		id = kpMgrInfo.cmdFlg[i];
		if ( id == InvalidID ) continue;

                /* send command */
		er = sendDeviceCommandTo(id, cmd);
		if ( er != E_OK ) err = er;
	}

	return err;
}

/*
 * change keyboard input mode
 */
EXPORT ER kpChangeKbInputMode( InputMode mode )
{
	ER	err;

	err = kpSendDeviceCommand(InputModeCmd(mode));
	if ( err != E_OK ) {
		DEBUG_PRINT(("kpChangeKbInputMode, err = %d\n", err));
	}
	return err;
}

/*
 * change PD scan frequency
 */
EXPORT ER kpChangePdScanRate( W rate )
{
	ER	err;

	err = kpSendDeviceCommand(ScanRateCmd(rate));
	if ( err != E_OK ) {
		DEBUG_PRINT(("kpChangePdScanRate, err = %d\n", err));
	}
	return err;
}

/*
 * change sensitivity
 */
EXPORT ER kpChangePdSense( W sense )
{
	ER	err;

	err = kpSendDeviceCommand(SenseCmd(sense));
	if ( err != E_OK ) {
		DEBUG_PRINT(("kpChangePdSense, err = %d\n", err));
	}
	return err;
}

/*
 * send a command to set the current status
 * to a specific device (eventflag)
 */
EXPORT ER kpSendInitialDeviceCommand( ID flgid )
{
	UW	cmd;
	ER	err;
	union {
		PdAttr	attr;
		UW	uw;
	} u;

        /* set keyboard input mode */
	cmd = InputModeCmd(kpMgrInfo.kpState.stat.mode);
	err = sendDeviceCommandTo(flgid, cmd);
	if ( err != E_OK ) {
		DEBUG_PRINT(("kpSendInitialDeviceCommand, err = %d\n", err));
		return err;
	}

        /* set PD scan frequency */
	cmd = ScanRateCmd(kpMgrInfo.pd.pdMode.attr.rate);
	err = sendDeviceCommandTo(flgid, cmd);
	if ( err != E_OK ) {
		DEBUG_PRINT(("kpSendInitialDeviceCommand, err = %d\n", err));
		return err;
	}

        /* set PD sensitivity */
	u.attr = kpMgrInfo.pd.pdMode.attr;
	cmd = SenseCmd(u.uw & (PD_ACMSK|PD_ABS|PD_SNMSK));
	err = sendDeviceCommandTo(flgid, cmd);
	if ( err != E_OK ) {
		DEBUG_PRINT(("kpSendInitialDeviceCommand, err = %d\n", err));
		return err;
	}

	return E_OK;
}

/*
 * send a command to set the current status
 * to all the devices (eventflags)
 */
EXPORT ER kpSetAllDeviceStatus( void )
{
	ID	id;
	int	i;
	ER	err, error = E_OK;

	for ( i = 0; i < MaxCmd; ++i ) {
		id = kpMgrInfo.cmdFlg[i];
		if ( id == InvalidID ) continue;

                /* send command to set the current status */
		err = kpSendInitialDeviceCommand(id);
		if ( err < E_OK ) error = err;
	}

	return error;
}
