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
 *	sysmgr.h (T-Kernel/SM)
 *	T-Kernel/System Manager
 */

#ifndef _SYSMGR_
#define _SYSMGR_

#include <basic.h>
#include <tk/tkernel.h>
#include <libstr.h>
#include <sys/queue.h>
#include <tk/util.h>
#include <sys/debug.h>

/* ------------------------------------------------------------------------ */
/*
 *	Device management function
 */

/*
 * Lock for device management exclusive control
 */
IMPORT FastMLock	DevMgrLock;
#define LockDM()	MLock(&DevMgrLock, 0)
#define UnlockDM()	MUnlock(&DevMgrLock, 0)

/*
 * Lock for device registration exclusive control
 */
#define LockREG()	MLock(&DevMgrLock, 1)
#define UnlockREG()	MUnlock(&DevMgrLock, 1)

/*
 * Device management synchronous control
 *	Enter using semaphore, but free wait by 'tk_dis_wai()'
 *	because it frees wait by specifying the specific task.
 *	Do not execute 'tk_sig_sem().'
 */
IMPORT ID	DevMgrSync;
#define SyncWaitDM()	{			\
	tk_wai_sem(DevMgrSync, 1, TMO_FEVR);	\
	tk_ena_wai(TSK_SELF);			\
}
#define SyncSignalDM(tskid)	tk_dis_wai(tskid, TTW_SEM)

/*
 * Task Queue for Synchronization
 */
typedef struct {
	QUEUE	q;
	ID	tskid;		/* wait task ID */
} WaitQ;

/*
 * Device registration information
 */
typedef struct DeviceControlBlock {
	QUEUE	q;
	UB	devnm[L_DEVNM+1];	/* Device name */
	T_DDEV	ddev;			/* Registration information */
	QUEUE	openq;			/* Open device management queue */
	QUEUE	syncq;			/* Task Queue for Synchronization */
} DevCB;

IMPORT	DevCB		*DevCBtbl;	/* Device registration information
					   table */
IMPORT	QUEUE		UsedDevCB;	/* In-use queue */

#define DID(devcb)		( ((devcb) - DevCBtbl + 1) << 8 )
#define DEVID(devcb, unitno)	( DID(devcb) + (unitno) )
#define DEVCB(devid)		( DevCBtbl + (((devid) >> 8) - 1) )
#define UNITNO(devid)		( (devid) & 0xff )

/*
 * Device Request Packet
 */
typedef union devreq {
	struct c_devreq {		/* DEVREQ common part */
		union devreq *next;	/* I: linked list of rquest packtes(NULL: End) */
		void	*exinf;		/* X: extended information */
		ID	devid;		/* I: target device ID */
		INT	cmd:4;		/* I: request command */
		BOOL	abort:1;	/* I: if abort request is given, TRUE */
		BOOL	nolock:1;	/* I: if locking(resident) is unnecessary, TRUE */
		INT	rsv:26;		/* I: reserved (always 0) */
		T_TSKSPC tskspc;	/* I: task space of the	requesting task */
		/* subsequenst fields following `start' are shifted */
	} c;
	T_DEVREQ	s;		/* standard */
	T_DEVREQ_D	l;		/* 64 bits devices (TDA_DEV_D) */
} DEVREQ;

/*
 * Open management information
 */
typedef struct OpenControlBlock {
	QUEUE		q;
	QUEUE		resq;		/* For connection from resource
					   management */
	ID		resid;		/* Section resource ID */
	DevCB		*devcb;		/* Target device */
	INT		unitno;		/* Subunit number
					   (0: Physical device) */
	UINT		omode;		/* Open mode */
	QUEUE		requestq;	/* Request management queue */
	UH		syncreq;	/* number of pending synchronization requests */
	UH		waitone;	/* Number of individual request
					   waits */
	DEVREQ		*waireqlst;	/* List of multiple request waits */
	INT		nwaireq;	/* Number of multiple request waits */
	ID		abort_tskid;	/* Abort completion wait task */
	INT		abort_cnt;	/* Number of abort completion wait
					   requests */
} OpnCB;

#define RESQ_OPNCB(rq)		( (OpnCB*)((B*)(rq) - offsetof(OpnCB, resq)) )

/*
 * Request management information
 */
typedef struct RequestControlBlock {
	QUEUE		q;
	OpnCB		*opncb;		/* Open device */
	ID		tskid;		/* Processing task */
	DEVREQ		req;		/* Request packet */
} ReqCB;

/*
 * Resource management information
 */
typedef struct ResourceControlBlock {
	QUEUE		openq;		/* Open device management queue */
	INT		dissus;		/* Suspend disable request count */
} ResCB;

/*
 * Request function types
 */

typedef ER  (*OPNFN)( ID devid, UINT omode, void *exinf );
typedef ER  (*ABTFN)( ID tskid, DEVREQ *devreq, INT nreq, void *exinf );
typedef INT (*WAIFN)( DEVREQ *devreq, INT nreq, TMO tmout, void *exinf );
typedef INT (*WAIFN_U)( DEVREQ *devreq, INT nreq, TMO_U tmout, void *exinf );
typedef INT (*EVTFN)( INT evttyp, void *evtinf, void *exinf );
typedef ER  (*CLSFN)( ID devid, UINT option, void *exinf );
typedef ER  (*EXCFN)( DEVREQ *devreq, TMO tmout, void *exinf );
typedef ER  (*EXCFN_U)( DEVREQ *devreq, TMO_U tmout, void *exinf );

/* ------------------------------------------------------------------------ */

#if TA_GP
/*
 * Device driver call
 */
IMPORT INT _CallDeviceDriver( INT p1, INT p2, INT p3, INT p4, FP drv, void *gp );
#define CallDeviceDriver(p1, p2, p3, p4, drv, gp ) \
		_CallDeviceDriver((INT)(p1), (INT)(p2), (INT)(p3), (INT)(p4), \
							(FP)(drv), (gp))
#endif

/*
 * conversion between milliseconds and microseconds
 */
#define to_msec(usec)	( ((usec) + (D)999) / 1000 ) /* round up */
#define to_usec(msec)	( (msec) * (D)1000 )

#define to_usec_tmo(ms) ( (TMO_U)(( (ms) > 0 )? to_usec(ms): (ms)) )
#define to_msec_tmo(us) ( (TMO)  (( (us) > 0 )? to_msec(us): (us)) )

IMPORT void FlushCache( CONST void *laddr, INT len );
IMPORT void FlushCacheM( CONST void *laddr, INT len, UINT mode );
IMPORT ER ControlCacheM( void *laddr, INT len, UINT mode );
IMPORT INT GetCacheLineSize( void );

#define IMPORT_DEFINE	1
#if IMPORT_DEFINE
/* smmain.c */
IMPORT void* GetResBlk( ID ssid, ID tskid );
IMPORT ER SystemManager( INT ac, UB *av[] );
/* system.c */
IMPORT INT _tk_get_cfn( CONST UB *name, INT *val, INT max );
IMPORT INT _tk_get_cfs( CONST UB *name, UB *buf, INT max );
IMPORT ER initialize_sysmgr( void );
IMPORT ER finish_sysmgr( void );
/* imalloc.c */
IMPORT void* IAmalloc( size_t size, UINT attr );
IMPORT void* IAcalloc( size_t nmemb, size_t size, UINT attr );
IMPORT void  IAfree( void *ptr, UINT attr );
IMPORT void* Imalloc( size_t size );
IMPORT void* Icalloc( size_t nmemb, size_t size );
IMPORT void  Ifree( void *ptr );
IMPORT ER init_Imalloc( void );
/* syslog.c */
IMPORT	W	logtask_alive ;
IMPORT ER __syslog_send( const char *string, int len );
IMPORT ER initialize_syslog( void );
IMPORT ER finish_syslog( void );
/* device.c */
IMPORT	FastMLock	DevMgrLock;
IMPORT	ID		DevMgrSync;
IMPORT	DevCB		*DevCBtbl;
IMPORT	QUEUE		UsedDevCB;
IMPORT DevCB* searchDevCB( CONST UB *devnm );
IMPORT INT phydevnm( UB *pdevnm, CONST UB *ldevnm );
IMPORT ER initialize_devmgr( void );
IMPORT ER finish_devmgr( void );
/* deviceio.c */
IMPORT ER check_devdesc( ID dd, UINT mode, OpnCB **p_opncb );
IMPORT ID _tk_opn_dev( CONST UB *devnm, UINT omode );
IMPORT ER _tk_cls_dev( ID dd, UINT option );
IMPORT ID _tk_rea_dev( ID dd, W start, void *buf, W size, TMO tmout );
IMPORT ID _tk_rea_dev_du( ID dd, D start, void *buf, W size, TMO_U tmout );
IMPORT ER _tk_srea_dev( ID dd, W start, void *buf, W size, W *asize );
IMPORT ER _tk_srea_dev_d( ID dd, D start, void *buf, W size, W *asize );
IMPORT ID _tk_wri_dev( ID dd, W start, CONST void *buf, W size, TMO tmout );
IMPORT ID _tk_wri_dev_du( ID dd, D start, CONST void *buf, W size, TMO_U tmout );
IMPORT ER _tk_swri_dev( ID dd, W start, CONST void *buf, W size, W *asize );
IMPORT ER _tk_swri_dev_d( ID dd, D start, CONST void *buf, W size, W *asize );
IMPORT ID _tk_wai_dev( ID dd, ID reqid, W *asize, ER *ioer, TMO tmout );
IMPORT ID _tk_wai_dev_u( ID dd, ID reqid, W *asize, ER *ioer, TMO_U tmout );
IMPORT INT _tk_sus_dev( UINT mode );
IMPORT void devmgr_break( ID tskid );
IMPORT void devmgr_startup( ID resid, INT info );
IMPORT void devmgr_cleanup( ID resid, INT info );
IMPORT ER initDevIO( void );
IMPORT ER finishDevIO( void );
#endif
/* chkplv.c */
IMPORT ER ChkCallPLevel( void );
/* power.c */
IMPORT void low_pow( void );
IMPORT void off_pow( void );
/* memmgr.c */
IMPORT UW smPageCount( UW byte );
IMPORT void* GetSysMemBlk( INT nblk, UINT attr );
IMPORT ER RelSysMemBlk( CONST void *addr );
IMPORT ER RefSysMemInfo( T_RSMB *pk_rsmb );
IMPORT ER init_memmgr( void );
IMPORT ER start_memmgr( void );
/* segmgr.c */
IMPORT ER initialize_segmgr( void );

#endif /* _SYSMGR_ */
