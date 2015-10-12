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
 *	kbpd.h
 *
 *       KB/PD device manager
 *       common definitions
 */

#include <tk/tkernel.h>
#include <device/lowkbpd.h>
#include <device/kbpd.h>
#include <device/keycode.h>
#include <device/devconf.h>
#include <device/sdrvif.h>
#include <tkse/extension/event.h>
#include <sys/util.h>
#include <tk/util.h>
#include <sys/queue.h>
#include <libstr.h>
#include <sys/debug.h>

#define	InvalidID	(0)		/* invalid ID */
#define	InvalidHNO	(0)		/* invalid handler number */
#define	InvalidDevID	(-1)		/* invalid device ID */
#define	InvalidKeytop	(-1)		/* invalid keytop number */
#define	InvalidKeycode	(0)		/* invalid key code */

#define	DefaultPriority	30		/* default task priority */
#define	DefaultStkSize	3072		/* default stack size */

#define	MAX_KID		0x7f		/* maximum number of keyboard ID */

/*
 * maximum number of flags that can be registered for sending commands
 */
#define	MaxCmd		4

/*
 * key top information
 */
typedef union {
	UW	w;
	struct {
		UW	rsv:8;		/* reserved */
		UW	tenkey:1;	/* 1 in the case of ten key pad */
		UW	kid:7;		/* keyboard ID */
		UW	kcode:16;	/* key top code */
	} u;
} KeyTop;

/*
 * types of direct key input
 *	In principle, two or more keys can not be pushed simultaneously,
 *	but for PD simulation, main CC (blue arrow) keys have to be pushed
 *	with other keys simultaneously, hence they are treated as differently.
 *	  * CC key: Caret Control key (i.e. Character Cursor Control key)
 */
typedef enum {
	NormalKey	=  0,	/* ordinary direct key input */
	IK_CC_U		= -1,	/* main CC UP */
	IK_CC_D		= -2,	/* main CC DOWN */
	IK_CC_R		= -3,	/* main CC RIGHT */
	IK_CC_L		= -4	/* main CC LEFT */
} InKeyKind;

/*
 * type of shift key
 */
typedef enum {
	NoShift		= 0x00,	/* other than shift key */
	SK_SHIFT_L	= 0x01,	/* shift LEFT key */
	SK_SHIFT_R	= 0x02,	/* shift RIGHT key */
	SK_EXPANSION	= 0x04,	/* expansion key */
	SK_COMMAND	= 0x08,	/* command key */
	SK_ALL		= 0x0f	/* all shift key */
} ShiftKeyKind;

/*
 * types of PD buttons
 */
typedef enum {
	BK_MAIN		= 0x01,	/* main button */
	BK_SUB		= 0x02,	/* subbutton */
	BK_ALL		= BK_MAIN | BK_SUB
} ButtonKind;

#define	NumOfPdBut	(2)	/* number of PD buttons */

#define	PdButKind(i)	((ButtonKind)(BK_MAIN << (i)))

/*
 * (note)
 *       Some compilers treat enum as signed numbers,
 *       if only the necessary bit width is used for a bit field, it may be sign extended inadvertedly,
 * and you may get unexpected values. So some values are defined as UW.
 */

/*
 * KB/PD status
 * The definitions in the specifications are modified to fit the internal processing needs.
 */
typedef union {
	MetaBut		o;	/* Definitions in the specification */

	struct _MetaBut {
#if BIGENDIAN
		UW		rsv1:8;		/* reserved (0) */
		UW		pdsim:2;	/* PD simulation */
		UW		nodsp:1;	/* Hide pointer */
		UW		rsv2:3;		/* reserved (0) */
		UW		kbsel:1;	/* keyboard selection */
		UW		han:1;		/* Hankaku mode */

	/*ShiftKeyKind*/ UW	tmpShift:4;	/* temporary shift */
	/*ShiftKeyKind*/ UW	shiftLock:4;	/* simple lock */
	/*ShiftKeyKind*/ UW	shift:4;	/* shift status */

	/*InputMode*/    UW	mode:2;		/* key input mode */

	/*ButtonKind*/   UW	button:2;	/* PD button status */
#else
	/*ButtonKind*/   UW	button:2;	/* PD button status */

	/*InputMode*/    UW	mode:2;		/* key input mode */

	/*ShiftKeyKind*/ UW	shift:4;	/* shift status */
	/*ShiftKeyKind*/ UW	shiftLock:4;	/* simple lock */
	/*ShiftKeyKind*/ UW	tmpShift:4;	/* temporary shift */

		UW		han:1;		/* Hankaku mode */
		UW		kbsel:1;	/* keyboard selection */
		UW		rsv2:3;		/* reserved (0) */
		UW		nodsp:1;	/* Hide pointer */
		UW		pdsim:2;	/* PD simulation */
		UW		rsv1:8;		/* reserved (0) */
#endif
	} u;
} KpMetaBut;

/*
 * PD input data
 * The definitions in the specifications are modified to fit the internal processing needs.
 */
typedef union {
	PdInStat	o;	/* Definitions in the specification */

	struct _PdInStat {
		UW		read:1;		/* already read flag */
		InputCmd	cmd:7;		/* = INP_PD */
		UW		rsv1:4;		/* reserved (0) */
		DevError	err:4;		/* device error */

		UW		nodsp:1;	/* hide pointer */
		UW		rsv2:1;		/* reserved (0) */
		UW		onebut:1;	/* one button operation */
		UW		abs:1;		/* coordinate is absolute or relative. */
		UW		norel:1;	/* relative operation not supported */
		UW		tmout:1;	/* PD timeout is in effect */
		UW		butrev:1;	/* enable button left-right swap */
		UW		xyrev:1;	/* enable XY coordinate reversal */

#if BIGENDIAN
		UW		rsv3:3;		/* reserved (0) */
		UW		qpress:1;	/* quick-press modifier */
		UW		inv:1;		/* out of valid area (coordinates are invalid) */
		UW		vst:1;		/* enter the valid area from outside */
	/*ButtonKind*/ UW	button:2;	/* PD button status */
#else
	/*ButtonKind*/ UW	button:2;	/* PD button status */
		UW		vst:1;		/* enter the valid area from outside */
		UW		inv:1;		/* out of valid area (coordinates are invalid) */
		UW		qpress:1;	/* quick-press modifier */
		UW		rsv3:3;		/* reserved (0) */
#endif
	} u;
} KpPdInStat;

/* ------------------------------------------------------------------------ */

#include "devevt.h"
#include "innevt.h"
#include "statmach.h"

/* ------------------------------------------------------------------------ */

/*
 * keyboard definition data
 */
typedef struct {
	W	size;		/* keyDef size (in bytes) */
	KeyDef	keyDef;		/* keyboard definition (variable length) */
} KbDef;

/*
 * keyboard attribute data
 */
typedef struct {
	KbDef	*kbDef[2][MAX_KID+1];	/* keyboard definition (NULL = undefined) */
	UW	keyID;			/* default keyboard ID */
	UW	defKeyID:2;		/* keyID setting
					  (0:unspecified 1:automatic 2:fixed) */
	KeyMap	keyMap;			/* keymap */
	KeyMode	keyMode;		/* key mode */
} kbInfo;

#define	GetKbDef(i, kid)		( kpMgrInfo.kb.kbDef[i][kid] )
#define	SetKbDef(i, kid, kbdef)		( kpMgrInfo.kb.kbDef[i][kid] = kbdef )

/*
 * PD attribute data
 */
typedef struct {
	PdMode	pdMode;		/* PD mode */
	PdRange	pdRange;	/* PD range */
	W	pdSimSpeed;	/* PD simulation speed                */
	BOOL	pdSimInh;	/* halt PD simulation temporarily */
} pdInfo;

/*
 * PD location update period during PD simulation     unit in milliseconds
 */
#define	simStartTime()		(50)		/* start time */
#define	simIntervalTime()	(50)		/* repeat time */

/*
 * PD simulation status
 */
typedef struct {
        /* direction / speed */
	H	x;	 /* left -1, stop 0, right +1 */
	H	y;	 /* up -1, stop 0, down +1 */
	W	rep;	 /* repeat count */
} PdSimState;

/*
 * manager information
 */
typedef struct {
	FastLock	lock;		/* for exclusive control of manager information */

        /* interface between common part among devices */
	ID	acceptPort;		/* port to accept device request */
	ID	eventMbf;		/* message buffer for event notification */
	SDI	Sdi;			/* device driver I/F handler */
	BOOL	suspended;		/* TRUE during suspended state */

        /* interface information for I/O driver */
	ID	dataMbx;		/* mailbox for receiving data */
	ID	cmdFlg[MaxCmd];		/* event flag for sending command */

        /* receiving task for data from I/O driver */
	ID	dataReceiveTask;	/* data receiving task */

        /* attribute data */
	KPStat	kpState;		/* KB/PD status                          */
	kbInfo	kb;			/* keyboard attribute data */
	pdInfo	pd;			/* PD attribute data */

        /* state of the state machine */
	StateMachine	statMach;

        /* other state information */
	ShiftKeyKind	spress;		/* shift key press status */
#if	0	/* we no longer support relative movement with absolute coordinate device. */
	H		firstXpos;	/* absolute coordinate device */
	H		firstYpos;	/* origin for relative mode */
#endif
	PdSimState	pdSimState;	/* direction of PD simulation movement */
} MgrInfo;
IMPORT MgrInfo kpMgrInfo;

/* ------------------------------------------------------------------------ */

/*
 * key code discovery
 */
#define	isMetaKey(c)	( ((c) & 0xffe0) == 0x1000 || (c) == KC_HAN )
#define	isShiftKey(c)	( (c) >= KC_SHT_R && (c) <= KC_CMD )
#define	isLockKey(c)	( isMetaKey(c) && !isShiftKey(c) )
#define	isMainCC(c)	( (c) >= KC_CC_U && (c) <= KC_CC_L )
#define	isSubCC(c)	( (c) >= KC_SC_U && (c) <= KC_SC_L )
#define	isCCKey(c)	( (c) >= KC_CC_U && (c) <= KC_SC_L )

/*
 * miscellaneous
 */
#define	max(a,b)	(( (a) > (b) )? (a): (b))
#define	min(a,b)	(( (a) < (b) )? (a): (b))
#define	SWAP(type,x,y)	{ type z = (x); (x) = (y); (y) = z; }

/* ----------------------------------------------------------------------- */

/*
 *       meta key, PD button status
 *       0: off 1: on
 */
#define	ES_BUT		0x00000001	/* PD main button */
#define ES_BUT2		0x00000002	/* PD subbutton */
#define	ES_ALPH		0x00000004	/* English lock key */
#define	ES_KANA		0x00000008	/* katakana lock key */
#define	ES_LSHFT	0x00000010	/* shift LEFT key */
#define	ES_RSHFT	0x00000020	/* shift RIGHT key */
#define	ES_EXT		0x00000040	/* expansion key */
#define	ES_CMD		0x00000080	/* command key */
#define	ES_LLSHFT	0x00000100	/* shift left simple lock */
#define	ES_LRSHFT	0x00000200	/* shift right simple lock */
#define	ES_LEXT		0x00000400	/* expansion simple lock */
#define	ES_LCMD		0x00000800	/* command simple lock */
#define	ES_TLSHFT	0x00001000	/* shift left temporary shift */
#define	ES_TRSHFT	0x00002000	/* shift right temporary shift */
#define	ES_TEXT		0x00004000	/* expansion temporary shift */
#define	ES_TCMD		0x00008000	/* command temporary shift */
#define ES_HAN		0x00010000	/* hankaku key */
#define	ES_KBSEL	0x00020000	/* keyboard selection */

/*
 * input mode
 *               Combination of English lock key and katakana lock key
 *               selects input mode as follows.
 */
#define	IM_HIRA		0x0000			/* Japanese Hiragana */
#define	IM_ALPH		(ES_ALPH)		/* English (lower case) */
#define	IM_KATA		(ES_KANA)		/* Japanese katakana */
#define	IM_CAPS		(ES_ALPH | ES_KANA)	/* English (upper case) */
#define	IM_MASK		(ES_ALPH | ES_KANA)

#define	KIN_KANA	0x0000			/* Kana input mode */
#define	KIN_ROMAN	0x0001			/* roman input mode */

/* ----------------------------------------------------------------------- */

#define IMPORT_DEFINE	1
#if IMPORT_DEFINE
/* accept.c */
IMPORT INT kpReadFn( ID devid, INT datano, INT datacnt, void *buf, SDI sdi );
IMPORT INT kpWriteFn( ID devid, INT datano, INT datacnt, void *buf, SDI sdi );
IMPORT INT kpEventFn( INT evttyp, void *evtinf, SDI sdi );
/* devcmd.c */
IMPORT ER kpSendDeviceCommand( UW cmd );
IMPORT ER kpChangeKbInputMode( InputMode mode );
IMPORT ER kpChangePdScanRate( W rate );
IMPORT ER kpChangePdSense( W sense );
IMPORT ER kpSendInitialDeviceCommand( ID flgid );
IMPORT ER kpSetAllDeviceStatus( void );
/* etc.c */
IMPORT ER kpNotifyEvent( void *evt, W size );
IMPORT ER kpNotifyMetaEvent( void );
IMPORT ER kpNotifyPdEvent( TDEvtTyp devEvt, UW nodsp );
IMPORT UH kpToKeycode( KeyTop keytop, KpMetaBut *meta );
IMPORT W kpGetKeyKind( KeyTop keytop );
IMPORT BOOL kpChangeShiftLock( InnEvtType type, ShiftKeyKind skind );
IMPORT W kpKeyTopCode( KeyTop keytop, UW kbsel );
IMPORT void kpSetOrResetKeyMap( KeyTop keytop, UW kbsel, UW press );
IMPORT void kpAllResetKeyMap( void );
IMPORT void kpPdPreProcess( PdInput *msg );
IMPORT BOOL kpMovePointer( W xpos, W ypos );
IMPORT ER kpMovePointerAndNotify( H xpos, H ypos );
IMPORT TDEvtTyp kpPdMoveEvent( InnerEvent *evt );
IMPORT TDEvtTyp kpPdButEvent( InnerEvent *evt );
/* key.c */
IMPORT void kpExecKeyStateMachine( KeyState *ks, InnerEvent *evt, ReceiveData *msg );
/* innevt.c */
IMPORT void kpInnerEventProcess( InnerEvent *evt, TMO *tmout );
/* main.C */
IMPORT MgrInfo kpMgrInfo;
IMPORT ER KbPdDrv ( INT ac, UB *av[] );
IMPORT ER main( INT ac, UB *av[] );
/* pdbut.c */
IMPORT void kpExecPdButStateMachine( InnerEvent *evt, ReceiveData *msg, ButtonKind button );
/* pdsim.c */
IMPORT BOOL kpExecPdSimKeyDown( InnerEvent *evt, UH code, TMO *tmout );
IMPORT BOOL kpExecPdSimKeyUp( InnerEvent *evt, UH code, TMO *tmout );
IMPORT void kpExecPdSimRepeat( TMO *tmout );
/* receive.c */
IMPORT void kpDataReceiveTask( void );
IMPORT ER kpStartDataReceiveTask( PRI pri );
IMPORT void kpStopDataReceiveTask( void );
/* statmach.c */
IMPORT ER kpSendPseudoMsg( T_MSG *msg );
IMPORT ER kpSetAlarm( AlarmState *alm, MSEC offsetTime );
IMPORT BOOL kpChkAlarm( AlarmState *alm, TimeoutMsg *msg );
IMPORT ER kpInitializeStateMachine( void );
IMPORT void kpFinishStateMachine( void );
IMPORT void kpReleaseKey( KeyState *ks );
IMPORT BOOL kpExecStateMachine( InnerEvent *evt, ReceiveData *msg );
IMPORT ER kpKeyAndButtonForceUp( void );
#endif	//IMPORT_DEFINE
