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
 *	main.C
 *
 *       KB/PD device manager
 *       manager initialization
 */

#include "kbpd.h"

/* Get "DEVCONF" entry */
IMPORT W GetDevConf ( UB *name, W *val );

/*
 * manager information
 */
EXPORT MgrInfo kpMgrInfo;

/*
 * register device
 */
LOCAL ER registDevice( BOOL regist )
{
	ER	er;
	T_IDEV	idev;
	SDefDev	ddev = {
		NULL,		/* exinf */
		"kbpd",		/* devnm */
		0,		/* drvatr */
		0,		/* devatr */
		0,		/* nsub */
		1,		/* blksz */
		NULL,		/* open */
		NULL,		/* close */
		kpReadFn,	/* read */
		kpWriteFn,	/* write */
		kpEventFn,	/* event */
	};

	if ( regist ) {
                /* registration */
		er = SDefDevice(&ddev, &idev, &kpMgrInfo.Sdi);
		if ( er < E_OK ) {
			DEBUG_PRINT(("SDefDevice err= %d\n", er));
			goto err_exit;
		}
                /* configure default message buffer for event notification */
		kpMgrInfo.eventMbf = idev.evtmbfid;
	} else {
		er = SDelDevice(kpMgrInfo.Sdi);
	}
err_exit:
	return er;
}

/*
 * manager initialization
 */
LOCAL ER initialize( PRI mypri )
{
	T_CMBX	DataRcvMbx;
	W	val[L_DEVCONF_VAL];
	ER	err;

	/*
         * initialization of manager information
	 */
	memset(&kpMgrInfo, 0, sizeof(kpMgrInfo));

	/*
         * obtain configuration information
	 */
	err = GetDevConf("KBMODE", val);
	if ( err >= 1 ) {
		kpMgrInfo.kpState.stat.mode = val[0] & 0x03;
		kpMgrInfo.kpState.stat.han = (val[0] & 0x04) ? 1 : 0;
	}

	/*
         * create fast lock for exclusive control of manager
	 */
	err = CreateLock(&kpMgrInfo.lock, "kbpd");
	if ( err != E_OK ) {
		DEBUG_PRINT(("initialize fast lock err = %d\n", err));
		return err;
	}

	/*
         * create mailbox for receiving data from I/O driver
	 */
	SetOBJNAME(DataRcvMbx.exinf, "kbpd");
	DataRcvMbx.mbxatr = TA_TFIFO | TA_MFIFO;
	err = tk_cre_mbx(&DataRcvMbx);
	if ( err < E_OK ) {
		DEBUG_PRINT(("initialize DataRcvMbx. er = %d\n", err));
		return err;
	}
	kpMgrInfo.dataMbx = (ID)err;

	/*
         * start the receiving task for data from I/O driver
	 */
	err = kpStartDataReceiveTask(mypri-1);
	if ( err != E_OK ) {
		DEBUG_PRINT(("start DataReceiveTask. err = %d\n", err));
		return err;
	}

	return E_OK;
}

/*
 * manager termination processing
 */
LOCAL void finish( void )
{
	ID	id;

        /* unregister device */
	registDevice(FALSE);

        /* stop the receiving task for data from I/O driver */
	kpStopDataReceiveTask();

        /* release other resources */
	if ( (id = kpMgrInfo.dataMbx) != InvalidID )	tk_del_mbx(id);
	if ( (id = kpMgrInfo.acceptPort) != InvalidID )	tk_del_por(id);
	DeleteLock(&kpMgrInfo.lock);
}

/*
 * manager initialization processing
 */
LOCAL ER kbpdManager( PRI mypri )
{
	ER		err;

        /* manager initialization */
	err = initialize(mypri);
	if ( err != E_OK ) {
		DEBUG_PRINT(("initialize err = %d\n", err));
		goto err_exit;
	}

        /* register device */
	err = registDevice(TRUE);
	if ( err != E_OK ) {
		DEBUG_PRINT(("registDevice err = %d\n", err));
		goto err_exit;
	}

	return E_OK;

err_exit:
        /* epilog processing */
	finish();
	return err;
}

/*
 * entry
 */
EXPORT ER KbPdDrv( INT ac, UB *av[] )
{
	PRI		pri = DefaultPriority;
	char		*arg;
	W	v[L_DEVCONF_VAL];

        /* effective? */
	if (GetDevConf("KbPdDrvEnable", v) > 0 && !v[0]) return E_NOSPT;

        /* driver never terminates: ignore termination request */
	if ( ac < 0 ) return E_NOSPT;

        /* extract parameters */
	if ( ac > 1 && (arg = av[1]) != NULL ) {
		while ( *arg != NULL ) {
			switch ( *arg ) {
			case '!':	/* priority */
				arg++;
				pri = strtol(arg, &arg, 10);
				break;
			default:
				return E_PAR;
			}
		}
	}

        /* start driver */
	return kbpdManager(pri);
}
