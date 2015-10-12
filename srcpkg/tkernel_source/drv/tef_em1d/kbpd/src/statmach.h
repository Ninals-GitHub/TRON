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
 *	statmach.h
 *
 *       KB/PD device manager
 *      defitinition for enableware function (state machine)
 */

#define	MaxKey	20	/* maximum number of simultaneous key presses */

/*
 * alarm management information
 */
#define	NumOfAlmMsgBuf	(2)

typedef struct {
	ID		almno;			/* alarm handler number */
	UW		setTime;		/* alarm set time */
	TimeoutMsg	msg[NumOfAlmMsgBuf];	/* timeout message */
} AlarmState;

/*
 * PD button status
 */
typedef enum {
	BS_RELEASE,	/* release status */
	BS_ONTIME,	/* ON effective time wait */
	BS_PRESS,	/* press state */
	BS_OFFTIME,	/* OFF effective time wait */
	BS_INVTIME,	/* ineffective time wait */
	BS_ERROR	/* error */
} BState;

/*
 * PD button status
 */
typedef struct {
	BState		state;		/* button status */
	AlarmState	alm;		/* alarm management information */
	ButtonKind	button;		/* button kind */
	BOOL		butdown;	/* TRUE while the button is down */
} PdButState;

/*
 * key state
 */
typedef enum {
        /* state of direct key input */
	KS_RELEASE	= 0x00,		/* release status */
	KS_ONTIME,			/* ON effective time wait */
	KS_CONTIME,			/* wait for simultaneous presses */
	KS_PRESS,			/* press state */
	KS_OFFTIME,			/* OFF effective time wait */
	KS_INVTIME,			/* ineffective time wait */
	KS_ERROR,			/* error */

        /* shift key status */
	SS_RELEASE	= 0x10,		/* release status */
	SS_ONTIME,			/* ON effective time wait */
	SS_PRESS,			/* press state */
	SS_OFFTIME,			/* OFF effective time wait */
	SS_ERROR			/* error */
} KState;

/*
 * state of direct key input
 */
typedef struct {
	KpMetaBut	meta;		/* meta key status */
	BOOL		keydown:1;	/* TRUE while the key is down */
	BOOL		invpress:1;	/* key press while waiting during ineffective time */
} InkeyState;

/*
 * shift key status
 */
typedef struct {
	ShiftKeyKind	skind:8;	/* type of shift key */
	UW		pressCount:8;	/* press count */
	UW		pressTime;	/* the initial time at which the key was pressed */
} SKeyState;

/*
 * key status
 */
typedef struct {
	QUEUE		q;
	KeyTop		keytop;		/* key top information */
	H /*KState*/	state;		   /* key state */
	UW		kbsel:1;	/* keyboard selection when the key was pressed */
	AlarmState	alm;		/* alarm management information */
	union {
		InkeyState	i;	/* state of direct key input */
		SKeyState	s;	/* shift key status */
	} u;
} KeyState;

/*
 * state of the state machine
 */
typedef struct {
	KeyState	key[MaxKey];		/* key status */
	QUEUE		useq;			/* used KeyState queue */
	QUEUE		freq;			/* unused KeyState queue */
	CommonMsg	spressMsg;		/* PIC_SPRESS message area */
	CommonMsg	keyupMsg;		/* PIC_KEYUP message area */
	W		keyPress:8;		/* number of ordinary keys pressed */
	PdButState	pdBut[NumOfPdBut];	/* PD button status */
	KpPdInStat	pdInStat;		/* PD attribute data */
} StateMachine;
#define	StatMach	( kpMgrInfo.statMach )
