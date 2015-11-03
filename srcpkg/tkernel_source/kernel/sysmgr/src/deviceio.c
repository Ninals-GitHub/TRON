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
 *	deviceio.c (T-Kernel/SM)
 *	Device Management Function: Input/Output
 */

#include "sysmgr.h"
#include <sys/rominfo.h>
#include <sys/ssid.h>

LOCAL	OpnCB	*OpnCBtbl;	/* Open management information table */
LOCAL	QUEUE	FreeOpnCB;	/* Unused queue */
LOCAL	INT	MaxOpnDev;	/* Maximum number of devices open */

#define DD(opncb)		( (opncb) - OpnCBtbl + 1 )
#define OPNCB(dd)		( OpnCBtbl + ((dd) - 1) )

LOCAL	ReqCB	*ReqCBtbl;	/* Request management information table */
LOCAL	QUEUE	FreeReqCB;	/* Unused queue */
LOCAL	INT	MaxReqDev;	/* Maximum number of device requests */

#define REQID(reqcb)		( (reqcb) - ReqCBtbl + 1 )
#define REQCB(reqid)		( ReqCBtbl + ((reqid) - 1) )

#define DEVREQ_REQCB(devreq)	((ReqCB*)((B*)(devreq) - offsetof(ReqCB, req)))

/*
 * Get resource management information
 */
LOCAL ResCB* GetResCB( ID ssid, ID tskid )
{
	ResCB	*rescb;

	rescb = GetResBlk(ssid, tskid);
	if ( rescb == NULL ) {
		return NULL;
	}

	LockDM();

	/* If the startup function is not called, initialize at this point */
	if ( rescb->openq.next == NULL ) {
		/* Initialization of open device management queue */
		QueInit(&rescb->openq);
	}

	UnlockDM();

	return rescb;
}

/*
 * Verify validity of device descriptor
 */
EXPORT ER check_devdesc( ID dd, UINT mode, OpnCB **p_opncb )
{
	OpnCB	*opncb;

	if ( dd < 1 || dd > MaxOpnDev ) {
		return E_ID;
	}
	opncb = OPNCB(dd);
	if ( opncb->resid <= 0 ) {
		return E_ID;
	}

	if ( opncb->resid != tk_get_rid(TSK_SELF) ) {
		return E_OACV;
	}

	if ( mode != 0 ) {
		if ( (opncb->omode & mode) == 0 ) {
			return E_OACV;
		}
	}

	*p_opncb = opncb;
	return E_OK;
}

/*
 * Verify validity of request ID
 */
LOCAL ReqCB* check_reqid( ID reqid, OpnCB *opncb )
{
	ReqCB	*reqcb;

	if ( reqid < 1 || reqid > MaxReqDev ) {
		return NULL;
	}
	reqcb = REQCB(reqid);
	if ( reqcb->opncb != opncb ) {
		return NULL;
	}

	return reqcb;
}

/*
 * Get open management block
 */
LOCAL OpnCB* newOpnCB( DevCB *devcb, INT unitno, UINT omode, ResCB *rescb )
{
	OpnCB	*opncb;

	/* Get space in open management block */
	opncb = (OpnCB*)QueRemoveNext(&FreeOpnCB);
	if ( opncb == NULL ) {
		return NULL; /* No space */
	}

	/* Register as open device */
	QueInsert(&opncb->q, &devcb->openq);
	QueInsert(&opncb->resq, &rescb->openq);

	opncb->devcb  = devcb;
	opncb->unitno = unitno;
	opncb->omode  = omode;
	QueInit(&opncb->requestq);
	opncb->syncreq = 0;
	opncb->waitone = 0;
	opncb->nwaireq = 0;
	opncb->abort_tskid = 0;

	opncb->resid  = 0; /* Indicate that open processing is not completed */

	return opncb;
}

/*
 * Free open management block
 */
LOCAL void delOpnCB( OpnCB *opncb )
{
	QueRemove(&opncb->q);
	QueRemove(&opncb->resq);

	QueInsert(&opncb->q, &FreeOpnCB);

	opncb->resid = 0;
}

/*
 * Get request management block
 */
LOCAL ReqCB* newReqCB( OpnCB *opncb )
{
	ReqCB	*reqcb;

	/* Get space in request management block */
	reqcb = (ReqCB*)QueRemoveNext(&FreeReqCB);
	if ( reqcb == NULL ) {
		return NULL; /* No space */
	}

	/* Register as requested open device */
	QueInsert(&reqcb->q, &opncb->requestq);

	reqcb->opncb = opncb;

	return reqcb;
}

/*
 * Free request management block
 */
LOCAL void delReqCB( ReqCB *reqcb )
{
	QueRemove(&reqcb->q);

	QueInsert(&reqcb->q, &FreeReqCB);
	reqcb->opncb = NULL;
}

/* ------------------------------------------------------------------------ */

/*
 * Enter Synchronization Queue
 *	entry is registered as the task that has entered synchronization queue
 *	return value
 *	FALSE	entry is the first task that has entered the synchronization queue
 *	TRUE	there were other tasks in the synchronization queue
 */
LOCAL BOOL enterSyncWait( QUEUE *syncq, WaitQ *entry )
{
	BOOL	wait;

	wait = !isQueEmpty(syncq);

	entry->tskid = tk_get_tid();
	QueInsert(&entry->q, syncq);

	return wait;
}

/*
 * Erase entry from synchronization queue
 *     Erase entry from the synchronization queue.
 *	If there is another in the synchronizaton queue, wake it up.
 */
LOCAL void leaveSyncWait( QUEUE *syncq, WaitQ *entry )
{
	QueRemove(&entry->q);

	if ( !isQueEmpty(syncq) ) {
		SyncSignalDM(((WaitQ*)syncq->next)->tskid);
	}
}

/*
 * Check open mode
 */
LOCAL ER chkopenmode( DevCB *devcb, INT unitno, UINT omode )
{
	QUEUE	*q;
	OpnCB	*opncb;
	INT	read, write, rexcl, wexcl;

	if ( (omode & TD_UPDATE) == 0 ) {
		return E_PAR;
	}

	/* Check current open state */
	read = write = rexcl = wexcl = 0;
	for ( q = devcb->openq.next; q != &devcb->openq; q = q->next ) {
		opncb = (OpnCB*)q;

		if ( unitno == 0 || opncb->unitno == 0 || opncb->unitno == unitno ) {
			if ( (opncb->omode & TD_READ)  != 0 ) {
				read++;
			}
			if ( (opncb->omode & TD_WRITE) != 0 ) {
				write++;
			}
			if ( (opncb->omode & (TD_EXCL|TD_REXCL)) != 0) {
				rexcl++;
			}
			if ( (opncb->omode & (TD_EXCL|TD_WEXCL)) != 0) {
				wexcl++;
			}
		}
	}

	/* Is it able to open? */
	if ( (omode & (TD_EXCL|TD_REXCL)) != 0 && read  > 0 ) {
		return E_BUSY;
	}
	if ( (omode & (TD_EXCL|TD_WEXCL)) != 0 && write > 0 ) {
		return E_BUSY;
	}
	if ( (omode & TD_READ)  != 0 && rexcl > 0 ) {
		return E_BUSY;
	}
	if ( (omode & TD_WRITE) != 0 && wexcl > 0 ) {
		return E_BUSY;
	}

	return E_OK;
}

/*
 * If the designated device is opened, return TRUE.
 *	However, for devices opend by opncb, or
 *	devices for the open processing is not complete, return FALSE.
 *	For devices being closed, return TRUE.
 */
LOCAL BOOL chkopen( DevCB *devcb, INT unitno, OpnCB *opncb )
{
	QUEUE	*q;

	for ( q = devcb->openq.next; q != &devcb->openq; q = q->next ) {
		if ( (OpnCB*)q == opncb ) continue;
		if ( ((OpnCB*)q)->unitno == unitno
		  && ((OpnCB*)q)->resid != 0 ) return TRUE;
	}
	return FALSE;
}

/*
 * Device open
 */
EXPORT ID _tk_opn_dev( CONST UB *devnm, UINT omode )
{
	OPNFN	openfn;
	void	*exinf;
#if TA_GP
	void	*gp;
#endif
	UB	pdevnm[L_DEVNM + 1];
	INT	unitno;
	ResCB	*rescb;
	DevCB	*devcb;
	OpnCB	*opncb;
	WaitQ	waiq;
	BOOL	wait;
	ER	ercd;

	ercd = ChkSpaceBstrR(devnm, 0);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}

	unitno = phydevnm(pdevnm, devnm);

	/* Get resource management information */
	rescb = GetResCB(DEVICE_SVC, TSK_SELF);
	if ( rescb == NULL ) {
		ercd = E_CTX;
		goto err_ret1;
	}

	LockDM();

	/* Search device to open */
	devcb = searchDevCB(pdevnm);
	if ( devcb == NULL || unitno > devcb->ddev.nsub ) {
		ercd = E_NOEXS;
		vd_printf("3 ");
		goto err_ret2;
	}

	/* Check open mode */
	ercd = chkopenmode(devcb, unitno, omode);
	if ( ercd < E_OK ) {
		goto err_ret2;
	}

	/* Get open management block */
	opncb = newOpnCB(devcb, unitno, omode, rescb);
	if ( opncb == NULL ) {
		ercd = E_LIMIT;
		goto err_ret2;
	}

	/* Multiple tasks can initiate open/close processing,
	   so ensure processing only by one task at a time. */
	wait = enterSyncWait(&devcb->syncq, &waiq);

	UnlockDM();

	/* Wait for synchronization for concurrent open/close */
	if ( wait ) SyncWaitDM();

	LockDM();

	openfn = (OPNFN)devcb->ddev.openfn;
	exinf = devcb->ddev.exinf;
#if TA_GP
	gp = devcb->ddev.gp;
#endif

	/* Is device driver call required? */
	if ( chkopen(devcb, unitno, opncb) && (devcb->ddev.drvatr & TDA_OPENREQ) == 0 ) {
		openfn = NULL;
	}

	UnlockDM();

	if ( openfn != NULL ) {
		/* Device driver call */
#if TA_GP
		ercd = CallDeviceDriver(DEVID(devcb, unitno), omode, exinf, 0,
								(FP)openfn, gp);
#else
		ercd = (*openfn)(DEVID(devcb, unitno), omode, exinf);
#endif
		if ( ercd < E_OK ) {
			printf("openfn failed\n");
			goto err_ret3;
		}
	}

	LockDM();
	opncb->resid = tk_get_rid(TSK_SELF); /* Indicate that open processing is completed */

       /* Wake up task waiting for synchronization for concurrent open/close */
	leaveSyncWait(&devcb->syncq, &waiq);
	UnlockDM();

	return DD(opncb);

err_ret3:
	LockDM();
	delOpnCB(opncb);
	leaveSyncWait(&devcb->syncq, &waiq);
err_ret2:
	UnlockDM();
err_ret1:
	DEBUG_PRINT(("_tk_opn_dev ercd = %d\n", ercd));
	return ercd;
}

/*
 * Abort all requests
 */
LOCAL void abort_allrequest( OpnCB *opncb )
{
	ABTFN	abortfn;
	WAIFN	waitfn;
	ATR	drvatr;
	void	*exinf;
#if TA_GP
	void	*gp;
#endif
	DevCB	*devcb;
	ReqCB	*reqcb;
	QUEUE	*q;

	/* If 'execfn' and 'waitfn' are called, execute abort request. */
	LockDM();

	devcb = opncb->devcb;
	abortfn = (ABTFN)devcb->ddev.abortfn;
	waitfn  = (WAIFN)devcb->ddev.waitfn;
	drvatr  = devcb->ddev.drvatr;
	exinf   = devcb->ddev.exinf;
#if TA_GP
	gp = devcb->ddev.gp;
#endif

	opncb->abort_tskid = tk_get_tid();
	opncb->abort_cnt = 0;

	if ( opncb->nwaireq > 0 ) {
		/* Multiple requests wait */
		reqcb = DEVREQ_REQCB(opncb->waireqlst);

		/* Device driver call */
#if TA_GP
		CallDeviceDriver(reqcb->tskid, opncb->waireqlst,
					opncb->nwaireq, exinf, (FP)abortfn, gp);
#else
		(*abortfn)(reqcb->tskid, opncb->waireqlst, opncb->nwaireq,
								exinf);
#endif
		opncb->abort_cnt++;
	} else {
		/* Start request or single request wait */
		for ( q = opncb->requestq.next; q != &opncb->requestq; q = q->next ) {
			reqcb = (ReqCB*)q;
			if ( reqcb->tskid == 0 ) {
				continue;
			}

			reqcb->req.c.abort = TRUE;

			/* Device driver call */
#if TA_GP
			CallDeviceDriver(reqcb->tskid, &reqcb->req, 1, exinf,
								(FP)abortfn, gp);
#else
			(*abortfn)(reqcb->tskid, &reqcb->req, 1, exinf);
#endif
			opncb->abort_cnt++;
		}
	}

	UnlockDM();

	if ( opncb->abort_cnt > 0 ) {
		/* Wait for completion of abort request processing */
		SyncWaitDM();
	}
	opncb->abort_tskid = 0;

	/* Abort remaining requests and wait for completion */
	LockDM();
	while ( !isQueEmpty(&opncb->requestq) ) {
		reqcb = (ReqCB*)opncb->requestq.next;
		reqcb->req.c.abort = TRUE;

		UnlockDM();

		/* Device driver call */
#if TA_GP
		CallDeviceDriver(&reqcb->req, 1, TMO_FEVR, exinf, (FP)waitfn, gp);
#else
		if ( (drvatr & TDA_TMO_U) == 0 ) {
			(*waitfn)(&reqcb->req, 1, TMO_FEVR, exinf);
		} else {
			(*(WAIFN_U)waitfn)(&reqcb->req, 1, TMO_FEVR, exinf);
		}
#endif

		LockDM();

		/* Unregister completed request */
		delReqCB(reqcb);
	}
	UnlockDM();
}

/*
 * Device close processing
 */
LOCAL ER close_device( OpnCB *opncb, UINT option )
{
	CLSFN	closefn;
	void	*exinf;
#if TA_GP
	void	*gp;
#endif
	ID	devid;
	DevCB	*devcb;
	INT	unitno;
	WaitQ	waiq;
	BOOL	wait;
	ER	ercd = E_OK;

	/* Abort all requests during processing */
	abort_allrequest(opncb);

	LockDM();

	devcb  = opncb->devcb;
	unitno = opncb->unitno;
	devid = DEVID(devcb, unitno);

	/* Multiple tasks can initiate open/close processing,
	   so ensure processing only by one task at a time. */
	wait = enterSyncWait(&devcb->syncq, &waiq);

	UnlockDM();

       /* Wake up task waiting for synchronization for concurrent open/close */
	if ( wait ) SyncWaitDM();

	LockDM();

	closefn = (CLSFN)devcb->ddev.closefn;
	exinf = devcb->ddev.exinf;
#if TA_GP
	gp = devcb->ddev.gp;
#endif

	/* Is device driver call required? */
	if ( chkopen(devcb, unitno, opncb) ) {
		option &= ~TD_EJECT;
		if ( (devcb->ddev.drvatr & TDA_OPENREQ) == 0 ) {
			closefn = NULL;
		}
	}

	UnlockDM();

	if ( closefn != NULL ) {
		/* Device driver call */
#if TA_GP
		ercd = CallDeviceDriver(devid, option, exinf, 0, (FP)closefn, gp);
#else
		ercd = (*closefn)(devid, option, exinf);
#endif
	}

	LockDM();

	/* Free open management block */
	delOpnCB(opncb);

       /* Wake up task waiting for synchronization for concurrent open/close */
	leaveSyncWait(&devcb->syncq, &waiq);

	UnlockDM();

#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("close_device ercd = %d\n", ercd));
	}
#endif
	return ercd;
}

/*
 * Device close
 */
EXPORT ER _tk_cls_dev( ID dd, UINT option )
{
	OpnCB	*opncb;
	ER	ercd;

	LockDM();

	ercd = check_devdesc(dd, 0, &opncb);
	if ( ercd < E_OK ) {
		UnlockDM();
		goto err_ret;
	}

	opncb->resid = -1; /* Indicate that it is during close processing */

	UnlockDM();

	/* Device close processing */
	ercd = close_device(opncb, option);

err_ret:
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("_tk_cls_dev ercd = %d\n", ercd));
	}
#endif
	return ercd;
}

/* ------------------------------------------------------------------------ */

enum ReqType {
	AsyncReq,	/* asynchronous I/O (tk_rea_dev tk_wri_dev tk_wai_dev) */
	SyncReq		/* synchronous I/O  (tk_srea_dev tk_swri_dev) */
};

/*
 * Check break request
 */
LOCAL ER check_break( void )
{
	T_RTEX	rtex;
	ER	ercd;

	ercd = tk_ref_tex(TSK_SELF, &rtex);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return ( rtex.pendtex != 0 )? E_ABORT: E_OK;

err_ret:
	DEBUG_PRINT(("check_break ercd = %d\n", ercd));
	return ercd;
}

/*
 * Request for starting input/output to device
 */
LOCAL ID request( ID dd, D start, void *buf, INT size, TMO_U tmout, INT cmd, enum ReqType syncreq )
{
	EXCFN	execfn;
	ATR	drvatr;
	void	*exinf;
#if TA_GP
	void	*gp;
#endif
	OpnCB	*opncb;
	DevCB	*devcb;
	ReqCB	*reqcb;
	UINT	m;
	ER	ercd;

	LockDM();

	/* Check whether there is a break request */
	ercd = check_break();
	if ( ercd < E_OK ) {
		goto err_ret1;
	}

	if ( start <= -0x00010000 && start >= -0x7fffffff ) {
		m = 0; /* Ignore open mode */
	} else {
		m = ( cmd == TDC_READ )? TD_READ: TD_WRITE;
	}
	ercd = check_devdesc(dd, m, &opncb);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}

	if ( syncreq == SyncReq ) {
		/* synchronous I/O is prohibited while there are pending I/O requests for completion. */
		if ( opncb->nwaireq > 0 ) { ercd = E_OBJ; goto err_ret1; }
		opncb->syncreq++;
	}

	devcb = opncb->devcb;
	execfn = (EXCFN)devcb->ddev.execfn;
	drvatr = devcb->ddev.drvatr;
	exinf  = devcb->ddev.exinf;
#if TA_GP
	gp = devcb->ddev.gp;
#endif

	/* check the range of parameter value*/
	if ( (drvatr & TDA_DEV_D) == 0 ) {
		if ( start > 0x7fffffff || start < (-0x7fffffff-1) ) {
			ercd = E_PAR; goto err_ret2;
		}
	}

	/* Get request management block */
	reqcb = newReqCB(opncb);
	if ( reqcb == NULL ) {
		ercd = E_LIMIT;
		goto err_ret2;
	}

	/* Set request packet */
	reqcb->req.c.next   = NULL;
	reqcb->req.c.exinf  = NULL;
	reqcb->req.c.devid  = DEVID(devcb, opncb->unitno);
	reqcb->req.c.cmd    = cmd;
	reqcb->req.c.abort  = FALSE;
	reqcb->req.c.nolock = ( (opncb->omode & TD_NOLOCK) != 0 )? TRUE: FALSE;
	reqcb->req.c.rsv    = 0;
	if ( (drvatr & TDA_DEV_D) == 0 ) {
		reqcb->req.s.start   = start;
		reqcb->req.s.size    = size;
		reqcb->req.s.buf     = buf;
		reqcb->req.s.asize   = 0;
		reqcb->req.s.error   = 0;
	} else {
		reqcb->req.l.start_d = start;
		reqcb->req.l.size    = size;
		reqcb->req.l.buf     = buf;
		reqcb->req.l.asize   = 0;
		reqcb->req.l.error   = 0;
	}
	ercd = tk_get_tsp(TSK_SELF, &reqcb->req.c.tskspc);
	if ( ercd < E_OK ) {
		goto err_ret3;
	}

	/* Indicate that it is during processing */
	reqcb->tskid = tk_get_tid();

	UnlockDM();

	/* Device driver call */
#if TA_GP
	ercd = CallDeviceDriver(&reqcb->req, tmout, exinf, 0, (FP)execfn, gp);
#else
	if ( (drvatr & TDA_TMO_U) == 0 ) {
		ercd = (*execfn)(&reqcb->req, to_msec_tmo(tmout), exinf);
	} else {
		ercd = (*(EXCFN_U)execfn)(&reqcb->req, tmout, exinf);
	}
#endif

	LockDM();

	/* Indicate that it is not during processing */
	reqcb->tskid = 0;

	/* If there is an abort completion wait task,
	   notify abort completion */
	if ( opncb->abort_tskid > 0 && --opncb->abort_cnt == 0 ) {
		SyncSignalDM(opncb->abort_tskid);
	}

	if ( ercd < E_OK ) {
		goto err_ret3;
	}

	UnlockDM();

	return REQID(reqcb);

err_ret3:
	delReqCB(reqcb);
err_ret2:
	if ( syncreq == SyncReq ) opncb->syncreq--;
err_ret1:
	UnlockDM();
	DEBUG_PRINT(("request ercd = %d\n", ercd));
	return ercd;
}

/*
 * wait for I/O completion for a device
 */
LOCAL ID waitcomplete( ID dd, ID reqid, W *asize, ER *ioer, TMO_U tmout, enum ReqType syncreq )
{
	WAIFN	waitfn;
	ATR	drvatr;
	void	*exinf;
#if TA_GP
	void	*gp;
#endif
	OpnCB	*opncb;
	DevCB	*devcb;
	ReqCB	*reqcb;
	DEVREQ	*devreq;
	INT	reqno, nreq;
	ID	tskid;
	BOOL	abort;
	ER	ercd;

	tskid = tk_get_tid();

	LockDM();

	ercd = check_devdesc(dd, 0, &opncb);
	if ( ercd < E_OK ) {
		goto err_ret1;
	}

	/* Check whether there is a break request */
	abort = FALSE;
	ercd = check_break();
	if ( ercd < E_OK ) {
		if ( syncreq == AsyncReq ) goto err_ret1;
		abort = TRUE;
	}

	devcb = opncb->devcb;
	waitfn = (WAIFN)devcb->ddev.waitfn;
	drvatr = devcb->ddev.drvatr;
	exinf  = devcb->ddev.exinf;
#if TA_GP
	gp = devcb->ddev.gp;
#endif

	if ( reqid == 0 ) {
		/* When waiting for completion of any of requests for 'dd' */
		if ( opncb->nwaireq > 0 || opncb->waitone > 0
		     || opncb->syncreq > 0 ) {
			ercd = E_OBJ;
			goto err_ret1;
		}
		if ( isQueEmpty(&opncb->requestq) ) {
			ercd = E_NOEXS;
			goto err_ret1;
		}

		/* Create wait request list */
		reqcb = (ReqCB*)opncb->requestq.next;
		for ( nreq = 1;; nreq++ ) {
			reqcb->tskid = tskid;
			devreq = &reqcb->req;
			reqcb = (ReqCB*)reqcb->q.next;
			if ( reqcb == (ReqCB*)&opncb->requestq ) {
				break;
			}
			devreq->c.next = &reqcb->req;
		}
		devreq->c.next = NULL;
		devreq = &((ReqCB*)opncb->requestq.next)->req;

		opncb->waireqlst = devreq;
		opncb->nwaireq = nreq;
	} else {
		/* Wait for completion of abort request processing */
		reqcb = check_reqid(reqid, opncb);
		if ( reqcb == NULL ) {
			ercd = E_ID;
			goto err_ret1;
		}
		if ( opncb->nwaireq > 0 || reqcb->tskid > 0 ) {
			ercd = E_OBJ;
			goto err_ret1;
		}

		/* Create waiting request list */
		reqcb->tskid = tskid;
		devreq = &reqcb->req;
		devreq->c.next = NULL;
		if ( abort ) devreq->c.abort = TRUE;
		nreq = 1;

		opncb->waitone++;
	}

	UnlockDM();

	/* Device driver call */
#if TA_GP
	reqno = CallDeviceDriver(devreq, nreq, tmout, exinf, (FP)waitfn, gp);
#else
	if ( (drvatr & TDA_TMO_U) == 0 ) {
		reqno = (*waitfn)(devreq, nreq, to_msec_tmo(tmout), exinf);
	} else {
		reqno = (*(WAIFN_U)waitfn)(devreq, nreq, tmout, exinf);
	}
#endif
	if ( reqno <  E_OK ) {
		ercd = reqno;
	}
	if ( reqno >= nreq ) {
		ercd = E_SYS;
	}

	LockDM();

	/* Free wait processing */
	if ( reqid == 0 ) {
		opncb->nwaireq = 0;
	} else {
		opncb->waitone--;
	}

	/* If there is an abort completion wait task,
	   notify abort completion */
	if ( opncb->abort_tskid > 0 && --opncb->abort_cnt == 0 ) {
		SyncSignalDM(opncb->abort_tskid);
	}

	/* Get processing result */
	while ( devreq != NULL ) {
		reqcb = DEVREQ_REQCB(devreq);
		if ( reqno-- == 0 ) {
			reqid = REQID(reqcb);
			if ( (drvatr & TDA_DEV_D) == 0 ) {
				*asize = devreq->s.asize;
				*ioer  = devreq->s.error;
			} else {
				*asize = devreq->l.asize;
				*ioer  = devreq->l.error;
			}
		}
		reqcb->tskid = 0;
		devreq = devreq->c.next;
	}

	if ( ercd < E_OK ) {
		goto err_ret1;
	}

	if ( syncreq == SyncReq ) opncb->syncreq--;

	/* Unregister completed request */
	delReqCB(REQCB(reqid));

	UnlockDM();

	return reqid;

err_ret1:
	if ( syncreq == SyncReq ) opncb->syncreq--;
	UnlockDM();
	DEBUG_PRINT(("waitcomplete ercd = %d\n", ercd));
	return ercd;
}

/*
 * Start reading from device
 */
EXPORT ID _tk_rea_dev( ID dd, W start, void *buf, W size, TMO tmout )
{
	return _tk_rea_dev_du(dd, start, buf, size, to_usec_tmo(tmout));
}
EXPORT ID _tk_rea_dev_du( ID dd, D start, void *buf, W size, TMO_U tmout )
{
	ER	ercd;

	ercd = request(dd, start, buf, size, tmout, TDC_READ, AsyncReq);

#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("_tk_rea_dev ercd = %d\n", ercd));
	}
#endif
	return ercd;
}

/*
 * Synchronous reading from device
 */
EXPORT ER _tk_srea_dev( ID dd, W start, void *buf, W size, W *asize )
{
	return _tk_srea_dev_d(dd, start, buf, size, asize);
}
EXPORT ER _tk_srea_dev_d( ID dd, D start, void *buf, W size, W *asize )
{
	ER	ercd, ioercd;

	ercd = ChkSpaceRW(asize, sizeof(W));
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	ercd = request(dd, start, buf, size, TMO_FEVR, TDC_READ, SyncReq);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	ercd = waitcomplete(dd, ercd, asize, &ioercd, TMO_FEVR, SyncReq);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return ioercd;

err_ret:
	DEBUG_PRINT(("_tk_srea_dev ercd = %d\n", ercd));
	return ercd;
}

/*
 * Start writing to device
 */
EXPORT ID _tk_wri_dev( ID dd, W start, CONST void *buf, W size, TMO tmout )
{
	return _tk_wri_dev_du(dd, start, buf, size, to_usec_tmo(tmout));
}
EXPORT ID _tk_wri_dev_du( ID dd, D start, CONST void *buf, W size, TMO_U tmout )
{
	ER	ercd;

	ercd = request(dd, start, (void*)buf, size, tmout, TDC_WRITE, AsyncReq);

#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("_tk_wri_dev ercd = %d\n", ercd));
	}
#endif
	return ercd;
}

/*
 * Synchronous writing to device
 */
EXPORT ER _tk_swri_dev( ID dd, W start, CONST void *buf, W size, W *asize )
{
	return _tk_swri_dev_d(dd, start, buf, size, asize);
}
EXPORT ER _tk_swri_dev_d( ID dd, D start, CONST void *buf, W size, W *asize )
{
	ER	ercd, ioercd;

	ercd = ChkSpaceRW(asize, sizeof(W));
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	ercd = request(dd, start, (void*)buf, size, TMO_FEVR, TDC_WRITE, SyncReq);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	ercd = waitcomplete(dd, ercd, asize, &ioercd, TMO_FEVR, SyncReq);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return ioercd;

err_ret:
	DEBUG_PRINT(("_tk_swri_dev ercd = %d\n", ercd));
	return ercd;
}

/*
 * Request completion wait
 */
EXPORT ID _tk_wai_dev( ID dd, ID reqid, W *asize, ER *ioer, TMO tmout )
{
	return _tk_wai_dev_u(dd, reqid, asize, ioer, to_usec_tmo(tmout));
}
EXPORT ID _tk_wai_dev_u( ID dd, ID reqid, W *asize, ER *ioer, TMO_U tmout )
{
	ER	ercd;

	ercd = ChkSpaceRW(asize, sizeof(W));
	if ( ercd < E_OK ) {
		goto err_ret;
	}
	ercd = ChkSpaceRW(ioer, sizeof(ER));
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	ercd = waitcomplete(dd, reqid, asize, ioer, tmout, AsyncReq);
	if ( ercd < E_OK ) goto err_ret;

	return ercd;

err_ret:
	DEBUG_PRINT(("_tk_wai_dev_u ercd = %d\n", ercd));
	return ercd;
}

/* ------------------------------------------------------------------------ */

/* Suspend disable request count */
LOCAL	INT	DisSusCnt;

/* Maximum number of suspend disable request counts */
#define MAX_DISSUS	0x7fff

/*
 * Send driver request event to each device
 */
LOCAL ER sendevt_alldevice( INT evttyp, BOOL disk )
{
	EVTFN	eventfn;
	QUEUE	*q;
	DevCB	*devcb;
	BOOL	d;
	ER	ercd = E_OK;

	for ( q = UsedDevCB.next; q != &UsedDevCB; q = q->next ) {
		devcb = (DevCB*)q;

		d = ( (devcb->ddev.devatr & TD_DEVTYPE) == TDK_DISK )?
							TRUE: FALSE;
		if ( disk != d ) {
			continue;
		}

		/* Device driver call */
		eventfn = (EVTFN)devcb->ddev.eventfn;
#if TA_GP
		ercd = CallDeviceDriver(evttyp, NULL, devcb->ddev.exinf, 0,
						(FP)eventfn, devcb->ddev.gp);
#else
		ercd = (*eventfn)(evttyp, NULL, devcb->ddev.exinf);
#endif
	}

#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("sendevt_alldevice ercd = %d\n", ercd));
	}
#endif
	return ercd;
}

/*
 * Suspend
 */
LOCAL ER do_suspend( void )
{
	ER	ercd;

	/* Stop accepting device registration/unregistration */
	LockREG();

	/* Processing before starting subsystem suspend */
	ercd = tk_evt_ssy(0, TSEVT_SUSPEND_BEGIN, 0, 0);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("1. do_suspend -> tk_evt_ssy ercd = %d\n", ercd));
	}
#endif

	/* Suspend processing of device except for disks */
	ercd = sendevt_alldevice(TDV_SUSPEND, FALSE);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("2. do_suspend -> sendevt_alldevice ercd = %d\n", ercd));
	}
#endif

	/* Suspend processing of disk device */
	ercd = sendevt_alldevice(TDV_SUSPEND, TRUE);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("3. do_suspend -> sendevt_alldevice ercd = %d\n", ercd));
	}
#endif

	/* Stop accepting new requests */
	LockDM();

	/* Processing after completion of subsystem suspend */
	ercd = tk_evt_ssy(0, TSEVT_SUSPEND_DONE, 0, 0);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("4. do_suspend -> tk_evt_ssy ercd = %d\n", ercd));
	}
#endif

	/* Transit to suspend state */
	ercd = tk_set_pow(TPW_DOSUSPEND);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("5. do_suspend -> tk_set_pow ercd = %d\n", ercd));
	}
#endif
	/* Return from suspend state */

	/* Processing before starting subsystem resume */
	ercd = tk_evt_ssy(0, TSEVT_RESUME_BEGIN, 0, 0);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("6. do_suspend -> tk_evt_ssy ercd = %d\n", ercd));
	}
#endif

	/* Resume accepting requests */
	UnlockDM();

	/* Resume processing of disk device */
	ercd = sendevt_alldevice(TDV_RESUME, TRUE);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("7. do_suspend -> sendevt_alldevice ercd = %d\n", ercd));
	}
#endif

	/* Resume processing of device except for disks */
	ercd = sendevt_alldevice(TDV_RESUME, FALSE);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("8. do_suspend -> sendevt_alldevice ercd = %d\n", ercd));
	}
#endif

	/* Resume accepting device registration/unregistration */
	UnlockREG();

	/* Processing after completion of subsystem resume */
	ercd = tk_evt_ssy(0, TSEVT_RESUME_DONE, 0, 0);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("9. do_suspend -> tk_evt_ssy ercd = %d\n", ercd));
	}
#endif

	return ercd;
}

/*
 * Suspend processing
 */
EXPORT INT _tk_sus_dev( UINT mode )
{
	ResCB	*rescb;
	BOOL	suspend = FALSE;
	ER	ercd;

	/* Get resource management information */
	rescb = GetResCB(DEVICE_SVC, TSK_SELF);
	if ( rescb == NULL ) {
		ercd = E_CTX;
		goto err_ret1;
	}

	LockDM();

	switch ( mode & 0xf ) {
	  case TD_SUSPEND:	/* Suspend */
		if ( DisSusCnt > 0 && (mode & TD_FORCE) == 0 ) {
			ercd = E_BUSY;
			goto err_ret2;
		}
		suspend = TRUE;
		break;

	  case TD_DISSUS:	/* Disable suspend */
		if ( DisSusCnt >= MAX_DISSUS ) {
			ercd = E_QOVR;
			goto err_ret2;
		}
		DisSusCnt++;
		rescb->dissus++;
		break;
	  case TD_ENASUS:	/* Enable suspend */
		if ( rescb->dissus > 0 ) {
			rescb->dissus--;
			DisSusCnt--;
		}
		break;

	  case TD_CHECK:	/* Get suspend disable request count */
		break;

	  default:
		ercd = E_PAR;
		goto err_ret2;
	}

	UnlockDM();

	if ( suspend ) {
		/* Suspend */
		ercd = do_suspend();
		if ( ercd < E_OK ) {
			goto err_ret1;
		}
	}

	return DisSusCnt;

err_ret2:
	UnlockDM();
err_ret1:
	DEBUG_PRINT(("_tk_sus_dev ercd = %d\n", ercd));
	return ercd;
}

/* ------------------------------------------------------------------------ */

/*
 * Device driver abort function call
 */
Inline ER call_abortfn( DevCB *devcb, ID tskid, DEVREQ *devreq, INT nreq )
{
	ABTFN	abortfn;

	abortfn = (ABTFN)devcb->ddev.abortfn;

#if TA_GP
	return CallDeviceDriver(tskid, devreq, nreq, devcb->ddev.exinf,
						(FP)abortfn, devcb->ddev.gp);
#else
	return (*abortfn)(tskid, devreq, nreq, devcb->ddev.exinf);
#endif
}

/*
 * Device management break function
 */
EXPORT void devmgr_break( ID tskid )
{
	ResCB	*rescb;
	OpnCB	*opncb;
	ReqCB	*reqcb;
	QUEUE	*q, *r;
	DEVREQ	*devreq;

	/* Get resource management block */
	rescb = GetResCB(DEVICE_SVC, tskid);
	if ( rescb == NULL ) {
		return;
	}

	LockDM();

	/* Search the request if 'tskid' task is executing and request abort */
	for ( q = rescb->openq.next; q != &rescb->openq; q = q->next ) {
		opncb = RESQ_OPNCB(q);

		if ( opncb->nwaireq > 0 ) {
			/* Multiple requests wait */
			reqcb = DEVREQ_REQCB(opncb->waireqlst);
			if ( reqcb->tskid == tskid ) {
				/* Abort request: Device driver call */
				call_abortfn(opncb->devcb, tskid,
					opncb->waireqlst, opncb->nwaireq);
				goto error_exit;
			}
		} else {
			/* Start request or single request wait */
			for ( r = opncb->requestq.next; r != &opncb->requestq; r = r->next ) {
				reqcb = (ReqCB*)r;
				if ( reqcb->tskid != tskid ) {
					continue;
				}

				/* Abort request: Device driver call */
				devreq = &reqcb->req;
				devreq->c.abort = TRUE;
				call_abortfn(opncb->devcb, tskid, devreq, 1);
				goto error_exit;
			}
		}
	}
error_exit:
	UnlockDM();
}

/*
 * Device management startup function
 */
EXPORT void devmgr_startup( ID resid, INT info )
{
	ResCB	*rescb;
	ER	ercd;

	ercd = tk_get_res(resid, DEVICE_SVC, (void**)&rescb);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	LockDM();

	/* Initialization of open device management queue */
	QueInit(&rescb->openq);
	rescb->dissus = 0;

	UnlockDM();

	return;

err_ret:
	DEBUG_PRINT(("devmgr_cleanup ercd = %d\n", ercd));
	return;
}

/*
 * Device management cleanup function
 */
EXPORT void devmgr_cleanup( ID resid, INT info )
{
	ResCB	*rescb;
	OpnCB	*opncb;
	ER	ercd;

	ercd = tk_get_res(resid, DEVICE_SVC, (void**)&rescb);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* Do nothing if it is not used even once */
	if ( rescb->openq.next == NULL ) {
		return;
	}

	LockDM();

	/* Free suspend disable request */
	DisSusCnt -= rescb->dissus;
	rescb->dissus = 0;

	/* Close all open devices */
	while ( !isQueEmpty(&rescb->openq) ) {
		opncb = RESQ_OPNCB(rescb->openq.next);

		/* Indicate that it is during close processing */
		opncb->resid = -1;

		UnlockDM();

		/* Device close processing */
		close_device(opncb, 0);

		LockDM();
	}
	UnlockDM();

	return;

err_ret:
	DEBUG_PRINT(("devmgr_cleanup ercd = %d\n", ercd));
	return;
}

/*
 * Initialization sequence of device input/output-related
 */
EXPORT ER initDevIO( void )
{
	INT	i;
	ER	ercd;

	i = _tk_get_cfn(SCTAG_TMAXOPNDEV, &MaxOpnDev, 1);
	if ( i < 1 ) {
		ercd = E_SYS;
		goto err_ret;
	}
	i = _tk_get_cfn(SCTAG_TMAXREQDEV, &MaxReqDev, 1);
	if ( i < 1 ) {
		ercd = E_SYS;
		goto err_ret;
	}

	/* Generate open management information table */
	OpnCBtbl = Imalloc((UINT)MaxOpnDev * sizeof(OpnCB));
	if ( OpnCBtbl == NULL ) {
		ercd = E_NOMEM;
		goto err_ret;
	}

	QueInit(&FreeOpnCB);
	for ( i = 0; i < MaxOpnDev; ++i ) {
		OpnCBtbl[i].resid = 0;
		QueInsert(&OpnCBtbl[i].q, &FreeOpnCB);
	}

	/* Generate request management information table */
	ReqCBtbl = Imalloc((UINT)MaxReqDev * sizeof(ReqCB));
	if ( ReqCBtbl == NULL ) {
		ercd = E_NOMEM;
		goto err_ret;
	}

	QueInit(&FreeReqCB);
	for ( i = 0; i < MaxReqDev; ++i ) {
		ReqCBtbl[i].opncb = NULL;
		QueInsert(&ReqCBtbl[i].q, &FreeReqCB);
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("initDevIO ercd = %d\n", ercd));
	return ercd;
}

/*
 * Finalization sequence of device input/output-related
 */
EXPORT ER finishDevIO( void )
{
	/* Delete each table */
	if ( OpnCBtbl != NULL ) {
		Ifree(OpnCBtbl);
		OpnCBtbl = NULL;
	}
	if ( ReqCBtbl != NULL ) {
		Ifree(ReqCBtbl);
		ReqCBtbl = NULL;
	}

	return E_OK;
}
