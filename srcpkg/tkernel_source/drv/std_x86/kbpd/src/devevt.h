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
 *	devevt.h
 *
 * KB/PD device manager
 *       definitions for data used for events from I/O driver
 *       definitions for data used for additional events used only inside manager
 */

/*
 *       data type for data used for additional events used only inside manager
 */
typedef enum {
	PIC_TIMEOUT	= -1,	/* timeout */
	PIC_SPRESS	= -2,	/* shift press */
	PIC_KEYUP	= -3	/* key up */
} PseudoInputCmd;

typedef struct {
	T_MSG		head;
	struct {
		UW	read:1;		/* already read flag */
		InputCmd cmd:7;		/* command */
		UW	rsv1:4;
		DevError err:4;		/* device error */
		UW	rsv2:16;
	} cmd;
} CommonMsg;

/*
 * types of timeouts
 */
typedef enum {
	TMO_KEY		=  0,		/* >= 0 : key (KeyState number) */
	TMO_MAIN	= -1,		/* PD main button */
	TMO_SUB		= -2		/* PD subbutton */
} TimeoutKind;

#define	KeyTmoutKind(i)		((TimeoutKind)(TMO_KEY + (i)))
#define	PdButTmoutKind(i)	((TimeoutKind)(TMO_MAIN - (i)))

/*
 * timeout status
 */
typedef struct {
	UW		read:1;	/* already read flag */
	InputCmd	cmd:7;	/* = PIC_TIMEOUT */
	UW		rsv1:4;	/* reserved(0) */
	DevError	err:4;	/* error status */
	UW		rsv2:8;	/* reserved(0) */
	TimeoutKind	kind:8;	/* types of timeouts */
} TimeoutStat;

/*
 * timeout messasges
 */
typedef struct {
	T_MSG		head;
	TimeoutStat	stat;
	UW		time;	/* timeout generated time */
} TimeoutMsg;

/*
 * receive data structure
 */
typedef union {
	CommonMsg	head;		/* commoh header */

        /* received data from I/O driver */
	PdInput		pd;		/* PD input data */
	PdInput2	pd2;		/* PD input data 2 */
	KeyInput	kb;		/* KB input data */
	FlgInput	flg;		/* register flag for command */

        /* special data used inside manager */
	TimeoutMsg	tmout;		/* timeout */
	CommonMsg	spress;		/* shift press */
} ReceiveData;

/*
 * extract keytop information from device event
 */
Inline KeyTop toKeyTop( KeyInput *msg )
{
	KeyTop	keytop;

	keytop.w = 0;
	keytop.u.tenkey = msg->stat.tenkey;
	keytop.u.kid    = msg->stat.kbid;
	keytop.u.kcode  = msg->keytop;

	return keytop;
}
