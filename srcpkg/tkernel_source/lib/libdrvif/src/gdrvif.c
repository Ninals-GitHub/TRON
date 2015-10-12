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
 *	gdrvif.c
 *
 *	General-purpose device driver I/F layer
 */

#include <basic.h>
#include <device/gdrvif.h>
#include <tk/tkernel.h>
#include <tk/util.h>
#include <sys/queue.h>
#include <libstr.h>
#include <sys/debug.h>

typedef enum {
	NotDone	= 0,	/* Unused or during processing */
	Abort	= 1,	/* During abort*/
	Done	= 2	/* Completion*/
} DONE;

/*
 * Management queue for device request
 */
typedef struct DeviceRequestQueue {
	QUEUE		q;	/* For connection with queue */
	T_DEVREQ	*req;	/* Device request packet */
	INT		memsz;	/* Area size of I/O buffer */
	ID		wtid;	/* Wait-for-completion task ID */
	DONE		done;	/* Completion status */
} DRQ;

/*
 * Driver I/F access handle (GDI)
 */
struct GeneralDriverInterface {
	FastMLock	lock;	/* Lock for exclusive access control */
	GDefDev		def;	/* Device registration information */
	ID		devid;	/* Device ID */

	DRQ		*drq;	/* Buffer for DRQ */
	QUEUE		freeq;	/* Free queue*/
	QUEUE		acpq;	/* Wait-for-accepting queue*/
	ID		flgid;	/* Event flag for queue insert notification */

	INT		limit;	/* Maximum number of queuings per request type */
	INT		rreq;	/* The number of TDC_READ requests during the accept-wait and the processing */
	INT		wreq;	/* The number of TDC_WRITE requests during the accept-wait and the processing */

	INT		racpd;	/* The number of requests of the accept-wait TDC_READ (specific data) */
	INT		racpa;	/* The number of requests of the accept-wait TDC_READ (attribute data) */
	INT		wacpd;	/* The number of requests of the accept-wait TDC_WRITE (specific data) */
	INT		wacpa;	/* The number of requests of the accept-wait TDC_WRITE (attribute data) */
};

/* Count the number of the accept-wait requests */
#define	cntacpq(req, gdi, ope)					\
	if ( (req)->cmd == TDC_READ ) {				\
		if ( (req)->start < 0 ) (gdi)->racpa ope;	\
		else			(gdi)->racpd ope;	\
	} else {						\
		if ( (req)->start < 0 ) (gdi)->wacpa ope;	\
		else			(gdi)->wacpd ope;	\
	}

/* True if "q" is a valid DRQ */
#define	validDRQ(q, gdi) \
	( (q) >= (gdi)->drq && (q) < (gdi)->drq + (gdi)->def.maxreqq )

/*
 * Event flag pattern for queue insert notification
 *	Use "DEVREQ_ACPPTN(cmd)" for insertion into "acpq"
 */
#define	FREEQ		(FREEQ_RD|FREEQ_WR)	/* Insertion into "freeq"*/
#define	FREEQ_RD	0x40000000		/* Acceptance of "TDC_READ" request is enabled */
#define	FREEQ_WR	0x80000000		/* Acceptance of "TDC_WRITE" request is enabled */

/*
 * Lock for exclusive access control over an access to management information
 */
#define	LockGDI(gdi)	MLock(&(gdi)->lock, 0)
#define	UnlockGDI(gdi)	MUnlock(&(gdi)->lock, 0)

/*
 * Lock for exclusive access control over a processing function call
 */
#define	LockCALL(gdi)	MLock(&(gdi)->lock, 1)
#define	UnlockCALL(gdi)	MUnlock(&(gdi)->lock, 1)

/*
 * Task event for completion notification
 */
LOCAL INT	GDI_TEV;

#define	TEVPTN(tev)	( 1 << ((tev) - 1) )
#define	TTWTEV(tev)	( TTW_EV1 << ((tev) - 1) )

/* ------------------------------------------------------------------------ */
/*
 *	Get information from GDI
 *	These functions shall be call-ready even at the independent part,
 *	during disabling the dispatch, and during disabling the interrupts.
 */

/*
 * Get physical device ID
 */
EXPORT ID GDI_devid( GDI gdi )
{
	return gdi->devid;
}

/*
 * Get the extended information (GDefDev.exinf)
 */
EXPORT void* GDI_exinf( GDI gdi )
{
	return gdi->def.exinf;
}

/*
 * Get the registration information
 */
EXPORT const GDefDev* GDI_ddev( GDI gdi )
{
	return &gdi->def;
}

/* ------------------------------------------------------------------------ */
/*
 *	Management queue of device request
 */

/*
 * Fetch from the free queue
 *	When DRQ is obtained, execute a return in the state of executing "LockGDI()".
 */
LOCAL ER gdi_getDRQ( DRQ **p_drq, T_DEVREQ *req, TMO tmout, GDI gdi )
{
	DRQ	*drq;
	SYSTIM	stime, ctime;
	TMO	tmo;
	INT	n;
	UINT	ptn, waiptn;
	ER	err;

	waiptn = ( req->cmd == TDC_READ )? FREEQ_RD: FREEQ_WR;
	tmo = ( tmout == TMO_FEVR )? TMO_FEVR: 0;
	stime.hi = stime.lo = 0;
	for ( ;; ) {
		/* Fetch one from the free queue */
		LockGDI(gdi);
		n = ( req->cmd == TDC_READ )? gdi->rreq: gdi->wreq;
		if ( n < gdi->limit ) {
			drq = (DRQ*)QueRemoveNext(&gdi->freeq);
			if ( drq != NULL ) break; /* Obtained */
		}
		UnlockGDI(gdi);

		/* Remaining waiting time */
		if ( tmout != TMO_FEVR ) {
			err = tk_get_otm(&ctime);
			if ( err < E_OK ) goto err_ret;

			if ( tmo == 0 ) {
				stime = ctime;
				tmo = tmout;
			} else {
				tmo = tmout - (ctime.lo - stime.lo);
			}
			if ( tmo <= 0 ) { err = E_TMOUT; goto err_ret; }
		}

		/* Wait for DRQ to be returned to the free queue */
		err = tk_wai_flg(gdi->flgid, waiptn, TWF_ORW | TWF_BITCLR,
							&ptn, tmo);
		if ( err < E_OK ) {
			if ( err == E_DISWAI ) err = E_ABORT;
			goto err_ret;
		}
	}
	( req->cmd == TDC_READ )? gdi->rreq++: gdi->wreq++;

	/* Set the device request to DRQ */
	drq->req = req;
	if ( req->exinf != NULL ) {
		/* There is the task that waits for completion (gdi_waitfn) */
		drq->wtid = *(ID*)req->exinf;
	}
	req->exinf = drq;

	*p_drq = drq;
	return E_OK;  /* Return while executing "LockGDI()" */

err_ret:
	DEBUG_PRINT(("gdi_getDRQ err = %d\n", err));
	return err;
}

/*
 * Return to the free queue
 *	Call by executing "LockGDI()"
 */
LOCAL void gdi_relDRQ( DRQ *drq, GDI gdi )
{
	UINT	ptn = 0;

	if ( drq->req->cmd == TDC_READ ) {
		if ( gdi->rreq-- == gdi->limit ) ptn |= FREEQ_RD;
	} else {
		if ( gdi->wreq-- == gdi->limit ) ptn |= FREEQ_WR;
	}
	if ( isQueEmpty(&gdi->freeq) ) ptn |= FREEQ;

	if ( ptn != 0 ) {
		tk_set_flg(gdi->flgid, ptn);
	}

	QueInsert((QUEUE*)drq, &gdi->freeq);

	drq->req  = NULL;
	drq->wtid = 0;
	drq->done = NotDone;
}

/*
 * An insertion into the queue that waits for accepting a request
 *	Call by executing "LockGDI()"
 */
LOCAL ER gdi_sndreq( DRQ *drq, GDI gdi )
{
	T_DEVREQ	*req = drq->req;
	UINT		setptn;
	ER		err;

	if ( req->abort ) { err = E_ABORT; goto err_ret1; }

	/* Insert into the queue that waits for accepting */
	setptn = 0;
	if ( req->cmd == TDC_READ ) {
		if ( req->start < 0 ) {
			if ( gdi->racpa++ == 0 ) setptn |= DRP_AREAD;
		} else {
			if ( gdi->racpd++ == 0 ) setptn |= DRP_DREAD;
		}
	} else {
		if ( req->start < 0 ) {
			if ( gdi->wacpa++ == 0 ) setptn |= DRP_AWRITE;
		} else {
			if ( gdi->wacpd++ == 0 ) setptn |= DRP_DWRITE;
		}
	}
	QueInsert((QUEUE*)drq, &gdi->acpq);

	if ( setptn != 0 ) {
		/* Notify the insertion into the queue that waits for accepting  */
		err = tk_set_flg(gdi->flgid, setptn);
		if ( err < E_OK ) goto err_ret2;
	}

	return E_OK;

err_ret2:
	cntacpq(req, gdi, --);
	QueRemove((QUEUE*)drq);
err_ret1:
	DEBUG_PRINT(("gdi_sndreq err = %d\n", err));
	return err;
}

/*
 * Accept requests
 *	Fetch one request from the request accept queue.
 *	When there is no request in queue, the device driver shall enter into a wait status until request is made.
 *	Specify "acpptn" by ORing the pattern of accepted request type (TDC_READ/TDC_WRITE)
 *	or the value obtained in "DEVREQ_ACPPTN()" in the pattern of user command.
 *	In addition, the request regarding specific data and attribute data can be individually accepted.
 *	(Refer to the following descriptions of the accept-wait-extended function).
 *	As for the acceptance of request, normal request (including the case of individual specification of "TDC_READ", "TDC_WRITE", "specific data", and "attribute data")
 *	is prioritized over the user command.
 *	However, normal request and user command may be simultaneously received.
 *	Specify a time-out time in milli second for "tmout".
 *	"TMO_POL" and "TMO_FEVR" also can be specified.
 *	Return the received request pattern or error to the return value.
 *	Return the pattern of the accepted request in the format specified for "acpptn".
 *	Briefly, when the accept-wait-extended function is used,
 *	the request for the specific data and the request for attribute are individually indicated.
 *	In this case," DRP_ADSEL" is also set.
 *
 *	In the case of Normal request
 *	  Return the pattern, which indicates the type of the accepted request, to the return value.
 *	  Return the accepted request to "*devreq"
 *	In the case of User command
 *	  Return the pattern, which indicates the accepted user command, to the return value.
 *	  When several kinds of user command are accumulated,
 *	  the pattern, in which all the user commands specified at "acpptn" are collectively accepted and ORed, shall be returned
 *	
 *	  Return NULL to "*devreq"
 *	In the case where normal request and user command are simultaneously accepted.
 *	  Return the pattern, which ORed both the accepted normal request and user command
 *	  , to the return value.
 *	  Return the accepted normal request to "*devreq".
 *	In the case of time-out or error
 *	  Return the error code to the return value. "E_TMOUT" is returned in case of time out.
 *	  "*devreq" is indeterminate.
 *
 *	"exinf" of the accepted request(T_DEVREQ) shall never be changed.
 *	Checking of "buf" space (ChkSpace) is already executed in driver I/F
 *	Reply to user command (GDI_Reply) is unnecessary.
 *
 *	Generally, the next request is newly received after one request is accepted and processed
 *	and the result is returned.
 *	However, several requests may be received and simultaneously processed.
 *	In the case of processing several requests simultaneously, the severalrequest processing tasks may execute each "GDI_Accept()" to be processed in parallel.
 *	Or one processing task may execute several "GDI_Accept()" to be processed simultaneously.
 *	
 *	It does not matter that the order of accepting request and the order of returning result are not necessarily the same.
 *
 *Accept-wait-extended function  (Specify the accept-wait for specific data and attribute data iIndividually)
 *	DRP_DREAD	Read the specific data
 *	DRP_DWRITE	Write the specific data
 *	DRP_AREAD	Read the attribute data
 *	DRP_AWRITE	Write the attribute data
 *	These shall be specified by ORing these above.
 *
 *	"DRP_DREAD |DRP_AREAD" is equivalent to "DRP_READ".
 *	"DRP_DWRITE|DRP_AWRITE" is equivalent to "DRP_WRITE".
 *
 *	These specific data and attribute data individual specification cannot be simultaneously combined
 *	and used with "DRP_READ" and "DRP_WRITE" .
 */
EXPORT INT GDI_Accept( T_DEVREQ **devreq, INT _acpptn, TMO tmout, GDI gdi )
{
	DRQ		*drq;
	T_DEVREQ	*req;
	QUEUE		*q;
	INT		reqptn, rptn;
	UINT		acpptn, aptn;
	SYSTIM		stime, ctime;
	TMO		tmo;
	ER		err;

	if ( (_acpptn & DRP_ADSEL) == 0 ) {
		/* Normal specification */
		if ( (_acpptn & ~(DRP_NORMREQ|DRP_USERCMD)) != 0 )
					{ err = E_PAR; goto err_ret; }
		acpptn = ((_acpptn & DRP_NORMREQ) << 8) | _acpptn;
	} else {
		/* Extended specification*/
		if ( (_acpptn & ~(DRP_REQMASK|DRP_USERCMD)) != 0 )
					{ err = E_PAR; goto err_ret; }
		acpptn = _acpptn & ~DRP_ADSEL;
	}

	aptn = 0;
	tmo = TMO_FEVR;
	stime.hi = stime.lo = 0;
	for ( ;; ) {
		/* Fetch the request from queue */
		LockGDI(gdi);
		drq = NULL;
		req = NULL; rptn = 0; /* "warning" measures */
		for ( q = gdi->acpq.next; q != &gdi->acpq; q = q->next ) {
			req = ((DRQ*)q)->req;
			rptn = DEVREQ_ACPPTN(req->cmd);
			if ( req->start < 0 ) rptn <<= 8;
			if ( (rptn & acpptn) == 0 ) continue;

			drq = (DRQ*)q;
			cntacpq(req, gdi, --);
			QueRemove(q);
			QueInit(q);
			break;
		}
		UnlockGDI(gdi);
		reqptn = aptn & DRP_USERCMD;
		if ( drq != NULL ) {
			if ( req->abort ) {
				/* Abort processing*/
				req->error = E_ABORT;
				GDI_Reply(req, gdi);
				continue;
			}

			/* Normal request was accepted */
			*devreq = req;
			reqptn |= rptn;

			if ( (aptn &= ~rptn & DRP_REQMASK) != 0 ) {
				/* Reset the unnecessarily cleared flag */
				tk_set_flg(gdi->flgid, aptn);
			}
			break;
		}
		if ( reqptn != 0 ) {
			/* Only the user command is accepted */
			*devreq = NULL;
			break;
		}

		/* Remaining waiting time */
		if ( tmout != TMO_FEVR ) {
			err = tk_get_otm(&ctime);
			if ( err < E_OK ) goto err_ret;
			if ( tmo < 0 ) {
				stime = ctime;
				tmo = tmout;
			} else {
				tmo = tmout - (ctime.lo - stime.lo);
				if ( tmo < 0 ) tmo = 0;
			}
		}

		/* Wait for the request to come */
		err = tk_wai_flg(gdi->flgid, acpptn, TWF_ORW | TWF_BITCLR,
								&aptn, tmo);
		if ( err < E_OK ) goto err_ret;
		aptn &= acpptn;
	}

	if ( (reqptn & DRP_REQMASK) != 0 ) {
		if ( (_acpptn & DRP_ADSEL) == 0 ) {
			/* Normal specification */
			reqptn |= (reqptn & (DRP_NORMREQ << 8)) >> 8;
			reqptn &= ~(DRP_NORMREQ << 8);
		} else {
			/* Extended specification*/
			reqptn |= DRP_ADSEL;
		}
	}

	return reqptn;

err_ret:
	DEBUG_PRINT(("GDI_Accept err = %d\n", err));
	return err;
}

/*
 * Issue the user command
 *	Issue the user command specified in "cmd".
 *	Only the values ranging from 16 to 23 can be specified for "cmd".
 *	The issued user command is accepted by "GDI_Accept()".
 *	When the user command, which is not accepted by "GDI_Accept()",
 *	is accumulated, the several identical commands shall not be accumulated.
 *	Even if the identical command is issued several times, only the amount
 *	of one time is accumulated.
 *	And the command accumulated by amount of one time of "GDI_Accept()" will disappear.
 *	Meanwhile, "GDI_SendCmd()" only issues the user command,
 *      and returns without waiting to be accepted by "GDI_Accept()".
 *	User command is generally used to release the waiting of "GDI_Accept()"
 *	at any given point in time.
 */
EXPORT ER GDI_SendCmd( INT cmd, GDI gdi )
{
	ER	err;

	if ( cmd < 16 || cmd > 23 ) { err = E_PAR; goto err_ret; }

	err = tk_set_flg(gdi->flgid, DEVREQ_ACPPTN(cmd));
	if ( err < E_OK ) goto err_ret;

	return E_OK;

err_ret:
	DEBUG_PRINT(("GDI_SendCmd err = %d\n", err));
	return err;
}

/*
 * Reply to request
 *	Return the processing result of the request accepted in "GDI_Accept()".
 *	The task that executed "GDI_Accept()" and the task to execute "GDI_Reply()"
 *	may not be the same.
 */
EXPORT void GDI_Reply( T_DEVREQ *req, GDI gdi )
{
	DRQ	*drq = req->exinf;

	LockGDI(gdi);
	drq->done = Done;

	if ( drq->wtid > 0 ) {
		/* Completion shall be notified to the task that waits for completion. */
		tk_sig_tev(drq->wtid, GDI_TEV);
	}
	UnlockGDI(gdi);
}

/* ------------------------------------------------------------------------ */
/*
 *	Processing function
 */

/*
 * Open function
 */
LOCAL ER gdi_openfn( ID devid, UINT omode, GDI gdi )
{
	ER	err;

	if ( gdi->def.open == NULL ) return E_OK;

	LockCALL(gdi);
	err = (*gdi->def.open)(devid, omode, gdi);
	UnlockCALL(gdi);

	return err;
}

/*
 * Close function
 */
LOCAL ER gdi_closefn( ID devid, UINT option, GDI gdi )
{
	ER	err;

	if ( gdi->def.close == NULL ) return E_OK;

	LockCALL(gdi);
	err = (*gdi->def.close)(devid, option, gdi);
	UnlockCALL(gdi);

	return err;
}

/*
 * Processing start function
 */
LOCAL ER gdi_execfn( T_DEVREQ *req, TMO tmout, GDI gdi )
{
	DRQ	*drq;
	INT	memsz;
	ER	err;

	/* I/O size */
	if ( req->start < 0 ) {
		/* Attribute data */
		memsz = req->size;
	} else {
		/* Specific data */
		if ( gdi->def.blksz <= 0 ) { err = E_PAR; goto err_ret1; }
		memsz = req->size * gdi->def.blksz;
	}
	if ( memsz < 0 ) { err = E_PAR; goto err_ret1; }

	if ( memsz > 0 ) {
		/* Check the "buf" area */
		if ( req->cmd == TDC_READ ) {
			err = ChkSpaceRW(req->buf, memsz);
		} else {
			err = ChkSpaceR(req->buf, memsz);
		}
		if ( err < E_OK ) goto err_ret1;

		if ( (gdi->def.drvatr & TDA_LOCKREQ) != 0 && !req->nolock ) {
			/* Lock the "buf" area (residentialization) */
			err = LockSpace(req->buf, memsz);
			if ( err < E_OK ) goto err_ret1;
		}
	}

	/* Get the "DRQ"*/
	err = gdi_getDRQ(&drq, req, tmout, gdi);
	if ( err < E_OK ) goto err_ret2;
	/* "LockGDI()" is in execution */

	drq->memsz = memsz;

	/* Insert it into the queue that waits for accepting the request */
	err = gdi_sndreq(drq, gdi);
	if ( err < E_OK ) goto err_ret3;

	UnlockGDI(gdi);

	return E_OK;

err_ret3:
	gdi_relDRQ(drq, gdi);
	UnlockGDI(gdi);
err_ret2:
	if ( (gdi->def.drvatr & TDA_LOCKREQ) != 0 && !req->nolock ) {
		if ( memsz > 0 ) UnlockSpace(req->buf, memsz);
	}
err_ret1:
	DEBUG_PRINT(("gdi_execfn err = %d\n", err));
	return err;
}

/*
 * Abort in the wait-for-completion
 *	Return 1 if request is completed by an abort
 *	Return 0 if wait-for-completion is necessary.
 */
LOCAL INT gdi_waitfn_abort( DRQ *drq, GDI gdi )
{
	T_DEVREQ	*req = drq->req;
	INT		done;

	if ( !isQueEmpty(&drq->q) ) {
		/* Wait for accepting the processing (inside queue) : delete from the queue that waits for accepting */
		cntacpq(req, gdi, --);
		QueRemove(&drq->q);
		QueInit(&drq->q);
		req->error = E_ABORT;
		drq->done = Done;
		done = 1;
	} else {
		/* During processing : call the abort function */
		drq->done = Abort;
		if ( gdi->def.abort != NULL ) {
			(*gdi->def.abort)(req, gdi);
		}
		done = 0;
	}

	return done;
}

/*
 * Wait-for-completion function
 */
LOCAL INT gdi_waitfn( T_DEVREQ *devreq, INT nreq, TMO tmout, GDI gdi )
{
	T_DEVREQ	*req;
	DRQ		*drq, *complete = NULL;
	ID		mytid;
	INT		i, n, abort = 0;
	INT		memsz;
	void		*buf;
	ER		err;

	mytid = tk_get_tid();

	LockGDI(gdi);

	/* Check if there is an already completed request */
	req = devreq;
	for ( n = 0; n < nreq; ++n ) {
		drq = req->exinf;
		if ( drq != NULL && drq->done == Done ) {
			complete = drq;  /* Completed request */
			break;
		}
		req = req->next;
	}
	if ( complete == NULL ) {
		/* Mark the request targeting the wait-for-completion */
		req = devreq;
		for ( i = 0; i < nreq; ++i ) {
			drq = req->exinf;
			if ( drq == NULL ) {
				/* Before a request-accept (not queued) */
				req->exinf = &mytid;
			} else {
				/* Wait-for-accepting a processing, or during processing */
				if ( req->abort && drq->done == NotDone ) {
					/* Abort processing*/
					abort |= gdi_waitfn_abort(drq, gdi);
				}
				drq->wtid = mytid;
			}
			req = req->next;
		}
	}

	UnlockGDI(gdi);

	if ( complete == NULL ) {
		/* Wait-for-completion */
		if ( abort == 0 ) {
			err = tk_wai_tev(TEVPTN(GDI_TEV), tmout);
		} else {
			err = E_ABORT;
		}

		/* Search the completed request, and release the completion-wait-mark */
		req = devreq;
		LockGDI(gdi);
		for ( i = 0; i < nreq; ++i ) {
			drq = req->exinf;
			if ( validDRQ(drq, gdi) ) {
				drq->wtid = 0;
				if ( complete == NULL && drq->done == Done ) {
					complete = drq;  /* Completed request*/
					n = i;
				}
			} else {
				req->exinf = NULL;
			}
			req = req->next;
		}
		UnlockGDI(gdi);

		/* Execute the clear because a task event may remain */
		tk_wai_tev(TEVPTN(GDI_TEV), TMO_POL);

		if ( complete == NULL ) {
			if ( err == E_DISWAI ) err = E_ABORT;
			if ( err >= E_OK ) err = E_SYS; /* It should be impossible */
			goto err_ret;
		}
	}

	/* "memsz > 0" when "buf" space needs to be unlocked */
	memsz = ( (gdi->def.drvatr & TDA_LOCKREQ) != 0
			&& !complete->req->nolock )? complete->memsz: 0;
	buf = complete->req->buf;

	/* Release "DRQ"*/
	LockGDI(gdi);
	gdi_relDRQ(complete, gdi);
	UnlockGDI(gdi);

	/* Unlock "buf" space: executing after the release of "DRQ" is required */
	if ( memsz > 0 ) UnlockSpace(buf, memsz);

	return n;

err_ret:
	DEBUG_PRINT(("gdi_waitfn err = %d\n", err));
	return err;
}

/*
 * Abort processing
 *	Call by executing "LockGDI()"
 */
LOCAL ER gdi_abortreq( ID tskid, T_DEVREQ *req, GDI gdi )
{
	DRQ	*drq = req->exinf;
	ER	err;

	if ( drq == NULL ) {
		/* Before the request of acceptance: release "execfn"-wait */
		tk_dis_wai(tskid, TTW_FLG);
		return E_OK;
	}

	if ( !isQueEmpty(&drq->q) ) {
		/* Wait-for-acceptance: delete from the accept-wait-queue */
		cntacpq(req, gdi, --);
		QueRemove(&drq->q);
		QueInit(&drq->q);
		req->error = E_ABORT;
		drq->done = Done;
	}

	if ( drq->done == Done ) {
		/* Processed: release the completion-wait */
		tk_dis_wai(tskid, TTWTEV(GDI_TEV));
		return E_OK;
	}

	/* Request the abort with device driver */
	if ( drq->done != Abort ) {
		drq->done = Abort;
		if ( gdi->def.abort != NULL ) {
			err = (*gdi->def.abort)(req, gdi);
			if ( err < E_OK ) goto err_ret;
		}
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("gdi_abortreq err = %d\n", err));
	return err;
}

/*
 * Abort processing function
 */
LOCAL ER gdi_abortfn( ID tskid, T_DEVREQ *req, INT nreq, GDI gdi )
{
	LockGDI(gdi);
	if ( nreq == 1 ) {
		/* Wait-for-acceptance or only one wait-for-completion */
		gdi_abortreq(tskid, req, gdi);
	} else {
		/* Several completion-waits */
		tk_dis_wai(tskid, TTWTEV(GDI_TEV));
	}
	UnlockGDI(gdi);

	return E_OK;
}

/*
 * Event function
 */
LOCAL INT gdi_eventfn( INT evttyp, void *evtinf, GDI gdi )
{
	INT	ret;

	if ( gdi->def.event == NULL ) return E_OK;

	LockCALL(gdi);
	ret = (*gdi->def.event)(evttyp, evtinf, gdi);
	UnlockCALL(gdi);

	return ret;
}

/* ------------------------------------------------------------------------ */
/*
 *	Device registration
 */

/*
 * Device registration processing
 */
LOCAL ER gdi_defdevice( GDI gdi, T_IDEV *idev )
{
	T_DDEV	ddev;
	ER	err;

	ddev.exinf   = gdi;
	ddev.drvatr  = gdi->def.drvatr & ~(TDA_LOCKREQ|TDA_LIMITEDREQ);
	ddev.devatr  = gdi->def.devatr;
	ddev.nsub    = gdi->def.nsub;
	ddev.blksz   = gdi->def.blksz;
	ddev.openfn  = (FP)gdi_openfn;
	ddev.closefn = (FP)gdi_closefn;
	ddev.execfn  = (FP)gdi_execfn;
	ddev.waitfn  = (FP)gdi_waitfn;
	ddev.abortfn = (FP)gdi_abortfn;
	ddev.eventfn = (FP)gdi_eventfn;

	err = tk_def_dev(gdi->def.devnm, &ddev, idev);
	if ( err < E_OK ) goto err_ret;

	gdi->devid = (ID)err;

	return E_OK;

err_ret:
	DEBUG_PRINT(("gdi_defdevice err = %d\n", err));
	return err;
}

/*
 * Device registration
 *	Device shall be registered in accordance with "ddev" registration information.
 *	Device initial information is returned to "idev".
 *	It is not returned when "idev = NULL".
 *	Return the driver I/F access handle to "GDI".
 */
EXPORT ER GDefDevice( GDefDev *ddev, T_IDEV *idev, GDI *p_gdi )
{
	GDI	gdi;
	DRQ	*drq;
	T_CFLG	cflg;
	INT	n;
	ER	err;

	if ( ddev->maxreqq < 1 ) { err = E_PAR; goto err_ret1; }

	if ( GDI_TEV == 0 ) {
		/* Get the allocation of task event */
		n = tk_get_cfn("TEV_GDI", &GDI_TEV, 1);
		if ( n < 1 ) { err = ( n < E_OK )? n: E_SYS; goto err_ret1; }
	}

	/* Create "GDI" */
	gdi = Kmalloc(sizeof(*gdi));
	if ( gdi == NULL ) { err = E_NOMEM; goto err_ret1; }

	gdi->def = *ddev;
	QueInit(&gdi->freeq);
	QueInit(&gdi->acpq);

	gdi->limit = ddev->maxreqq + 1;
	if ( (ddev->drvatr & TDA_LIMITEDREQ) != 0 ) gdi->limit /= 2;
	gdi->rreq = 0;
	gdi->wreq = 0;
	gdi->racpd = 0;
	gdi->racpa = 0;
	gdi->wacpd = 0;
	gdi->wacpa = 0;

	/* Allocate "DRQ" */
	n = ddev->maxreqq;
	drq = Kcalloc(n, sizeof(DRQ));
	if ( drq == NULL ) { err = E_NOMEM; goto err_ret2; }
	gdi->drq = drq;
	while ( n-- > 0 ) {
		QueInsert((QUEUE*)drq, &gdi->freeq);
		drq++;
	}

	/* Create the event flag for queue insert notification */
	strncpy((B*)&cflg.exinf, gdi->def.devnm, sizeof(void*));
	cflg.flgatr  = TA_TPRI | TA_WMUL;
	cflg.iflgptn = 0;
	err = tk_cre_flg(&cflg);
	if ( err < E_OK ) goto err_ret3;
	gdi->flgid = (ID)err;

	/* Create the lock for exclusive access control */
	err = CreateMLock(&gdi->lock, gdi->def.devnm);
	if ( err < E_OK ) goto err_ret4;

	/* Device registration */
	err = gdi_defdevice(gdi, idev);
	if ( err < E_OK ) goto err_ret5;

	*p_gdi = gdi;
	return E_OK;

err_ret5:
	DeleteMLock(&gdi->lock);
err_ret4:
	tk_del_flg(gdi->flgid);
err_ret3:
	Kfree(gdi->drq);
err_ret2:
	Kfree(gdi);
err_ret1:
	DEBUG_PRINT(("GDefDevice err = %d\n", err));
	return err;
}

/*
 * Update
 *	Update the GDI device registration in accordance with "ddev" registration information.
 *	Device name (devnm) and the maximum number of request queuings
 *	(maxreqq) cannot be changed.(must not be changed).
 *	All the requests that are accumulated in the request-accept-queue
 *	and are not accepted yet shall be aborted.
 *	Device ID is not changed when updated.
 *	"GRedefDevice()" cannot be called from the processing function
 *	(open/close/abort/event).
 */
EXPORT ER GRedefDevice( GDefDev *ddev, GDI gdi )
{
	DRQ	*drq;
	ER	err;

	LockGDI(gdi);
	gdi->def = *ddev;
	UnlockGDI(gdi);

	/* Update the device registration */
	err = gdi_defdevice(gdi, NULL);
	if ( err < E_OK ) goto err_ret;

	/* Abort all the requests accumulated in an accept-wait-queue */
	LockGDI(gdi);
	while ( (drq = (DRQ*)QueRemoveNext(&gdi->acpq)) != NULL ) {
		QueInit(&drq->q);
		drq->req->error = E_ABORT;
		drq->done = Done;
		if ( drq->wtid > 0 ) {
			tk_dis_wai(drq->wtid, TTWTEV(GDI_TEV));
		}
	}
	gdi->racpd = 0;
	gdi->racpa = 0;
	gdi->wacpd = 0;
	gdi->wacpa = 0;
	UnlockGDI(gdi);

	return E_OK;

err_ret:
	DEBUG_PRINT(("GRedefDevice err = %d\n", err));
	return err;
}

/*
 * Deregistration
 *	Deregister "GDI" device
 */
EXPORT ER GDelDevice( GDI gdi )
{
	ER	err, error = E_OK;

	/* Deregister device */
	err = tk_def_dev(gdi->def.devnm, NULL, NULL);
	if ( err < E_OK ) {
		error = err;
		if ( err == E_BUSY ) goto err_ret;
	}

	/* Delete the lock for exclusive access control */
	DeleteMLock(&gdi->lock);

	/* Delete the event flag for queue insert notification */
	err = tk_del_flg(gdi->flgid);
	if ( err < E_OK ) error = err;

	/* Delete "DRQ" */
	Kfree(gdi->drq);

	/* Delete "GDI" */
	Kfree(gdi);

err_ret:
#ifdef DEBUG
	if ( error < E_OK ) DEBUG_PRINT(("GDelDevice err = %d\n", error));
#endif
	return error;
}

/* ------------------------------------------------------------------------ */
