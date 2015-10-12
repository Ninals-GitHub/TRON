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
 *	lowkbpd.h
 *
 *       low-level KB/PD driver
 */

#ifndef	__DEVICE_LOWKBPD_H__
#define	__DEVICE_LOWKBPD_H__

#include <basic.h>
#include <tk/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

/* transmission from real I/O driver */

typedef	enum {
	INP_PD	= 0,		/* PD data                       */
	INP_KEY = 1,		/* key data                        */
	INP_FLG = 2,		/* event flag registration */
	INP_PD2	= 3,		/* pd data 2            */
	SpecialReserve = -1	/* negative numbers are reserved for special purpose */
} InputCmd;

/* device error */
typedef enum {
	DEV_OK		= 0,	/* normal                            */
	DEV_OVRRUN	= 1,	/* data overrun during receiving             */
	DEV_FAIL	= 2,	/* hardware malfunction              */
	DEV_SYSERR	= 3,	/* problem of real I/O driver         */
	DEV_RESET	= 15	/* reset                  */
} DevError;

/* INP_PD : sending PD input    */
typedef	struct {
	UW	read:1;		/* already read flag            */
	InputCmd cmd:7;		/* = INP_PD                      */
	UW	rsv1:4;		/* reserved(0)                        */
	DevError err:4;		/* device error              */

	UW	nodsp:1;	/* hide pointer      */
	UW	absalt:1;	/* adjusting absolute coordinate (ordinary 0)      */
	UW	onebut:1;	/* one button operation */
	UW	abs:1;		/* coordinate is absolute or relative         */
	UW	norel:1;	/* relative mode not supported  */
	UW	tmout:1;	/* PD timeout is in effect  */
	UW	butrev:1;	/* enable button left-right swap            */
	UW	xyrev:1;	/* enable XY coordinate values swap            */

#if BIGENDIAN
	UW	rsv3:3;		/* reserved (0)                        */
	UW	qpress:1;	/* quick press modifiver             */
	UW	inv:1;		/* out of valid area (coordinates are invalid)        */
	UW	vst:1;		/* enter the valid area from outside        */
	UW	sub:1;		/* subbutton status              */
	UW	main:1;		/* main button status              */
#else
	UW	main:1;		/* main button status              */
	UW	sub:1;		/* subbutton status              */
	UW	vst:1;		/* enter the valid area from outside        */
	UW	inv:1;		/* out of valid area (coordinates are invalid)        */
	UW	qpress:1;	/* quick press modifiver             */
	UW	rsv3:3;		/* reserved (0)                        */
#endif
} PdInStat;

typedef	struct {
	T_MSG	head;
	PdInStat stat;
	H	xpos;		/* X coordinate (relative / absolute) */
	H	ypos;		/* Y coordinate (relative / absolute)         */
} PdInput;

/*
 * normalized coordinate
 *       if the ratio of height and width of the real screen and the ratio of the normalized coordinate is different by a large factor,
 *       pointer movement looks very strange. Be warned.
 */
#define	PDIN_XMAX	4096
#define	PDIN_YMAX	3072

/* INP_PD2: transmission 2 of PD input    */
typedef struct {
	UW	read:1;		/* already read flag            */
	InputCmd cmd:7;		/* = INP_PD2                      */
	UW	rsv1:4;		/* reserved(0)                        */
	DevError err:4;		/* device error              */
	UW	rsv2:16;	/* reserved (0)                        */
} PdIn2Stat;

typedef struct {
	T_MSG	head;
	PdIn2Stat stat;
	H	wheel;		/* wheel rotation amount      */
	H	rsv;		/* reserved(0)                        */
} PdInput2;

/* INP_KEY: transmission of key input      */
typedef	struct {
	UW	read:1;		/* already read flag            */
	InputCmd cmd:7;		/* = INP_KEY                      */
	UW	rsv1:4;		/* reserved(0)                        */
	DevError err:4;		/* device error              */
	UW	rsv2:7;		/* reserved(0)                        */
	UW	tenkey:1;	/* 1 in the case of ten key pad */
	UW	kbid:7;		/* keyboard ID              */
	UW	press:1;	/* ON : 1, OFF : 0		*/
} KeyInStat;

typedef	struct {
	T_MSG	head;
	KeyInStat stat;
	W	keytop;		/* key top code              */
} KeyInput;

/* INP_FLG: registration and de-registration of event flag for command */

typedef	struct {
	UW	read:1;		/* already read flag            */
	InputCmd cmd:7;		/* = INP_FLG                      */
	UW	rsv1:4;		/* reserved(0)                        */
	DevError err:4;		/* always DEV_OK                    */
	UW	rsv2:7;		/* reserved(0)                        */
	UW	kb:1;		/* 1 when kbid is valid                 */
	UW	kbid:7;		/* keyboard ID              */
	UW	reg:1;		/* registration : 1, de-registration : 0      */
} FlgInStat;

typedef	struct {
	T_MSG	head;
	FlgInStat stat;
	ID	flgid;		/* event flag ID            */
} FlgInput;


/* send command to real I/O driver */

/* PD scan speed command rate = 0 - 15 */
#define	ScanRateCmd(rate)	(0x01000000 | (rate))

/* PD sensivitity command :   sense = 0 - 15     sensivitity
 *                               | PD_ABS        request absolude mode
 *                               | PD_ACMSK      accelleration (0 - 7)
 *	PD_ABS
 *      if absolute mode request is sent, a PD that can operate in absolute mode must use absolute mode
 *      before sending back the data for INP_PD .
 *       if there is no absolute mode request, then if PD can operate in relative mode, use relative modes
 *      before sending back the data for INP_PD .
 *               specified: event with absolute position (PdInStat.abs = 1)
 *               not specified : event with relative postion (PdInStat.abs = 0)
 *
 *	PD_ACMSK
 *       setting up acceleration to update pointer location (effective only in relative mode)
 *       0       no acceleration
 *       1 - 7 acceleration (small to large)
 */
#define	SenseCmd(sense)		(0x02000000 | (sense))

/* input mode command  :  mode  :  value for InputMode
				(HiraMode, AlphaMode, KataMode, CapsMode) */
#define	InputModeCmd(mode)	(0x03000000 | (mode))

/* suspend / resume command */
#define	SuspendKBPD		(0x10000000)
#define	ResumeKBPD		(0x10000001)

/* flag to show that command to real I/O driver can be sent */
#define	DeviceCommandReady	(0x80000000)

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_LOWKBPD_H__ */
