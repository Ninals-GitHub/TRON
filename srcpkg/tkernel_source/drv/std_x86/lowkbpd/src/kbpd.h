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
        kbpd.h          KB/PD real I/O driver : header file
 *
 */
#include	<tk/tkernel.h>
#include	<tk/util.h>
#include	<sys/util.h>
#include	<device/lowkbpd.h>
#include	<device/kbpd.h>
#include	<device/sdrvif.h>
#include	<tkse/extension/event.h>
#include	<libstr.h>

#ifdef	DEBUG
#define	DP(exp)	printf exp
#else
#define	DP(exp)
#endif

/*
        structure of message sent to the upper KB/PD driver
*/
typedef union {
	PdInput		p;		/* PD input 			*/
	PdInput2	p2;		/* PD input 2			*/
	KeyInput	k;		/* key input 			*/
	FlgInput	f;		/* event flag registration 	*/
} RawEvt;

/*
        input message buffer (used internally)
*/
typedef	struct {
	W	id;			/* = HWKB or HWPD		*/
	UB	dt[8];			/* input data			*/
} hwMsg;

typedef	union {
	hwMsg	hw;			/* input data from H/W           */
} InMsg;

#define	HWKB		(-100)		/* H/W KB data            */
#define	HWPD		(-101)		/* H/W PD data           */

#define	MAX_INMSG	128		/* maximum number of received messages */

/*
        global variables
*/
IMPORT	ID	CmdTsk;			/* command processing task ID */
IMPORT	ID	CmdFlg;			/* event flag ID for sending command */
IMPORT	ID	EvtMbx;			/* MBOX ID for sending event                */
IMPORT	ID	InpMbf;			/* input MBUF ID                    */

IMPORT	BOOL	Suspended;		/* suspended                        */

IMPORT	UW	InpMode;		/* input mode                        */
IMPORT	UW	KbdId;			/* keyboard ID              */
IMPORT	W	PdSense;		/* sensitivity (0-15), acceleration(0-7) */

IMPORT	PRI	TaskPri;		/* task priority */

#define	NumLockON	0x10		/* NumLock ON internal input mode */

/*
        task information
*/
#define	DEF_PRIORITY		25	/* default priority */
#ifdef	DEBUG
#define	TASK_STKSZ		3072	/* stack size */
#else
#define	TASK_STKSZ		2048	/* stack size */
#endif

/*
        command
*/
#define	DC_OPEN		1
#define	DC_SUSPEND	2
#define	DC_RESUME	3

/*
        external functions
*/
/* main.C */
IMPORT	INT	kpCreTask(W name, FP entry);
IMPORT	ER	main(INT ac, UB *av[]);

/* common.c */
IMPORT	void*	MemAlloc(W size);
IMPORT	void	MemFree(void *ptr);
IMPORT	ER	kpSendMsg(RawEvt *msg);
IMPORT	void	kpScalingPos(RawEvt *evt, W x, W y, PNT *fract, W x_max);
IMPORT	void	kpSendPdEvt(RawEvt *evt, UW *lsts, RawEvt *last);
IMPORT	void	kpSendWheelEvt(W z);
IMPORT	void	kpSendMouseEvt(W but, W x, W y, W z, UW *lsts);
IMPORT	void	kpSendKeyEvt(W sts, W keycode);
IMPORT	ER	kpSendDrvCmd(UW cmd);
IMPORT	void	kpDataTask(void);
IMPORT	void	kpCmdTask(void);

/* hwkbpd.c */
IMPORT	ER	hwInit(W cmd);
IMPORT	void	hwImode(W inpmd);
IMPORT	void	hwProc(InMsg *msg);
