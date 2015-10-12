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
 *	line_drv.h	Console/Low level serial I/O driver
 *
 *		Low-level driver common definition : System-independent
 */

#include <basic.h>
#include <device/serialio.h>
#include <device/rs.h>
#include <tk/tkernel.h>
#include <tk/util.h>
#include <sys/imalloc.h>
#include <sys/util.h>
#include <libstr.h>

#define	DEF_INBUFSZ	2048		/* Standard receive-buffer size	*/
#define	MIN_INBUFSZ	256		/* Minimum receive-buffer size	*/
#define OUBUFSZ		512		/* Send-buffer size:Fixed	*/

/* Error information */
typedef union {
	RsError	c;
	RsStat	s;
	ER	w;
} RsErr;

/* Flow control status */
typedef	struct {
	UB	reqchar:8;		/* Send-request character */
	UW	rsoff:1;		/* Receive-stop status by "RS = OFF" */
	UW	sndxoff:1;		/* Receive-stop status by sending "XOFF"  */
} FlowState;

/* Controller operation function groups */
struct _L_INFO;
typedef struct {
	ER	(*in)(struct _L_INFO *li, UB *buf, W len, W *alen, W tmout);
	ER	(*out)(struct _L_INFO *li, UB *buf, W len, W *alen, W tmout);
	ER	(*ctl)(struct _L_INFO *li, W kind, UW *arg);
	void	(*up)(struct _L_INFO *li);
	void	(*down)(struct _L_INFO *li);
} SC_FUNC;

/* Controller information */
typedef struct {
	SC_FUNC	*fn;			/* Controller operation function groups */
	RsHwConf_16450	c;		/* Port HW configuration */
	UB	fctl;			/* Port HW register information*/
	UB	dt[3];			/* Port HW other information */
} SC_DEFS;

/* Serial line management information */
typedef struct _L_INFO {
	SC_DEFS		scdefs;		/* Controller operation function definition */
	UW		suspend:1;	/* 1 when it is  a "suspend" status */
	UW		enbint:1;	/* 1 when it is an interrupt-enabled status */

	FastMLock	lock;		/* Lock for output and input */
	ID		flg;		/* Flag for the input-output interrupt wait */

	RsMode		mode;		/* Line mode */
	RsFlow		flow;		/* Flow control mode */
	FlowState	flowsts;	/* Flow control status */

	FUNCP		extfn;		/* External function */
	UW		extpar;		/* External function parameter */
	UB		lsts;		/* Current line error status */
	UB		lstshist;	/* Line error history status */
	UB		msts;		/* Current modem status */

	UW		in_rptr;	/* Input-buffer read pointer */
	UW		in_wptr;	/* Input-buffer writing pointer */
	UW		in_bufsz;	/* Input-buffer size */
	UB		*in_buf;	/* Top of input-buffer area */

	UW		ou_rptr;	/* Output-buffer read pointer */
	UW		ou_wptr;	/* Output-buffer writing pointer */

	UW		ou_cnt;		/* Output-data counter */
	UW		ou_len;		/* Output-data size */
	UB		ou_buf[OUBUFSZ];	/* Output-buffer */
} LINE_INFO;

#define	XOFF_MARGIN	64		/* Remaining size that sends "XOFF" */
#define	XON_MARGIN	128		/* Remaining size that send "XON" */

/* Send-buffer pointer mask */
#define	OU_PTRMASK(p, ptr)	((ptr) % OUBUFSZ)

/* Receive-buffer pointer mask */
#define	PTRMASK(p, ptr)		((ptr) % ((p)->in_bufsz))

/* Remaining size of receive-buffer */
#define	IN_BUF_REMAIN(p)	((p->in_rptr >= p->in_wptr) ?		\
				 (p->in_rptr - p->in_wptr) :		\
				 (p->in_rptr + p->in_bufsz - p->in_wptr))

/* Received size of receive-buffer */
#define	IN_BUF_SIZE(p)		((p->in_wptr >= p->in_rptr) ?		\
				 (p->in_wptr - p->in_rptr) :		\
				 (p->in_wptr + p->in_bufsz - p->in_rptr))

/* Control code */
#define	XOFF		('S'-'@')
#define	XON		('Q'-'@')

/* Return code */
typedef enum {
	RTN_OK		= 0,
	RTN_NONE	= -1,
	RTN_TMOUT	= -2,
	RTN_ABORT	= -3,
	RTN_ERR		= -4
} RTN;
typedef	W	WRTN;

/* Pattern of control flag */
#define	OU_LOCK		17
#define	IN_LOCK		16
#define	FLG_OU_ABORT	(1 << 3)
#define	FLG_OU_NORM	(1 << 2)
#define	FLG_IN_ABORT	(1 << 1)
#define	FLG_IN_NORM	(1 << 0)
#define	FLG_OU_WAIPTN	(FLG_OU_NORM | FLG_OU_ABORT)
#define	FLG_IN_WAIPTN	(FLG_IN_NORM | FLG_IN_ABORT)

/* Get/Release memory*/
#define	Malloc(len)	(void*)Imalloc(len)
#define	Free(ptr)	Ifree((void*)(ptr))

/* Default line mode */
IMPORT	RsMode		DefMode;

/* Flow status initialization constant number */
IMPORT	const	RsFlow		RsFlow0;
IMPORT	const	FlowState	FlowState0;

/* Serial line management information */
IMPORT	W		nPorts;
IMPORT	LINE_INFO	*LineInfo;
IMPORT	W		DebugPort;

/* True if it is a debug port */
#define	isDebugPort(li)		( (li) == &LineInfo[DebugPort] )

/* Lock */
IMPORT	ER	consMLock(FastMLock *lock, INT no);
IMPORT	ER	consMUnlock(FastMLock *lock, INT no);
IMPORT	ER	consCreateMLock(FastMLock *lock, UB *name);
IMPORT	ER	consDeleteMLock(FastMLock *lock);
