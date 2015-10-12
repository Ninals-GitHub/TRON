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
 *	rs.h		Serial(RS-232C) driver definition
 */

#ifndef __DEVICE_RS_H__
#define __DEVICE_RS_H__

#include <basic.h>
#include <tk/devmgr.h>
#include <tk/syslib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RS data number */
typedef enum {
	/* Common attribute */
	DN_PCMCIAINFO	= TDN_PCMCIAINFO,
				/* PC card number 	data: PCMCIAInfo R */
	/* Individual attribute */
	DN_RSMODE	= -100,	/* Communication mode	data: RsMode	RW */
	DN_RSFLOW	= -101,	/* Flow control		data: RsFlow	RW */
	DN_RSSTAT	= -102,	/* Line status		data: RsStat	R  */
	DN_RSBREAK	= -103,	/* Send "BREAK" 	data: UW	W  */
	DN_RSSNDTMO	= -104,	/* Send time-out	data: UW	RW */
	DN_RSRCVTMO	= -105,	/* Receive time-out	data: UW	RW */
	DN_RSADDIN	= -150,	/* Additional function : unsupported	*/
	/* IBM keyboard additional function exclusive attribute:unsupported	*/
	DN_IBMKB_KBID	= -200,	/* Keyboard ID	*/
	/* Touch panel additional function exclusive attribute: unsupported	*/
	DN_TP_CALIBSTS	= -200,	/* Calib status		*/
	DN_TP_CALIBPAR	= -201,	/* Calib parameter	*/
	/* System type attribute  */
	DN_RS16450	= -300	/* Hardware configuration data: RsHwConf_16450 RW */
} RSDataNo;

/* DN_RSMODE: Communication mode (RW) */
typedef struct {
	UW	parity:2;	/* 0: non, 1: odd number, 2: even number, and  3: -	*/
	UW	datalen:2;	/* 0: 5 bit,1: 6 bit,2: 7 bit,3: 8 bit */
	UW	stopbits:2;	/* 0: 1 bit,1: 1.5 bit,2: 2 bit,3: - */
	UW	rsv:2;		/* Reserved */
	UW	baud:24;	/* Baud rate */
} RsMode;

/* DN_RSFLOW:	Flow control (RW) */
typedef	struct {
	UW	rsv:26;		/* Reserved */
	UW	rcvxoff:1;	/* "XOFF" status/Compulsory change */
	UW	csflow:1;	/* "CTS" control */
	UW	rsflow:1;	/* "RTS" control */
	UW	xonany:1;	/* "XON" by any character */
	UW	sxflow:1;	/* Send "XON/XOFF" control */
	UW	rxflow:1;	/* Receive "XON/XOFF" contorol */
} RsFlow;

/* DN_RSSTAT:	Line status (R) */
typedef	struct {
#if BIGENDIAN
	UW	rsv1:20;
	UW	BE:1;	/* Recv Buffer Overflow Error */
	UW	FE:1;	/* Framing Error	*/
	UW	OE:1;	/* Overrun Error	*/
	UW	PE:1;	/* Parity Error		*/
	UW	rsv2:2;
	UW	XF:1;	/* Recv XOFF		*/
	UW	BD:1;	/* Break Detect		*/
	UW	DR:1;	/* Dataset Ready (DSR)	*/
	UW	CD:1;	/* Carrier Detect (DCD)	*/
	UW	CS:1;	/* Clear to Send (CTS)	*/
	UW	CI:1;	/* Calling Indicator(RI)*/
#else
	UW	CI:1;	/* Calling Indicator(RI)*/
	UW	CS:1;	/* Clear to Send (CTS)	*/
	UW	CD:1;	/* Carrier Detect (DCD)	*/
	UW	DR:1;	/* Dataset Ready (DSR)	*/
	UW	BD:1;	/* Break Detect		*/
	UW	XF:1;	/* Recv XOFF		*/
	UW	rsv2:2;
	UW	PE:1;	/* Parity Error		*/
	UW	OE:1;	/* Overrun Error	*/
	UW	FE:1;	/* Framing Error	*/
	UW	BE:1;	/* Recv Buffer Overflow Error*/
	UW	rsv1:20;
#endif
} RsStat;

/* Error information*/
typedef	struct {
#if BIGENDIAN
	UW	ErrorClass:16;	/* Error class= EC_IO */
	UW	rsv1:2;
	UW	Aborted:1;	/* Aborted	*/
	UW	Timout:1;	/* Time out	*/
	/* Same as "RsStat" from here */
	UW	BE:1;		/* Recv Buffer Overflow Error */
	UW	FE:1;		/* Framing Error	*/
	UW	OE:1;		/* Overrun Error	*/
	UW	PE:1;		/* Parity Error		*/
	UW	rsv2:2;
	UW	XF:1;		/* Recv XOFF		*/
	UW	BD:1;		/* Break Detect		*/
	UW	DR:1;		/* Dataset Ready (DSR)	*/
	UW	CD:1;		/* Carrier Detect (DCD)	*/
	UW	CS:1;		/* Clear to Send (CTS)	*/
	UW	CI:1;		/* Calling Indicator(RI)*/
#else
	UW	CI:1;		/* Calling Indicator(RI)*/
	UW	CS:1;		/* Clear to Send (CTS)	*/
	UW	CD:1;		/* Carrier Detect (DCD)	*/
	UW	DR:1;		/* Dataset Ready (DSR)	*/
	UW	BD:1;		/* Break Detect		*/
	UW	XF:1;		/* Recv XOFF		*/
	UW	rsv2:2;
	UW	PE:1;		/* Parity Error		*/
	UW	OE:1;		/* Overrun Error	*/
	UW	FE:1;		/* Framing Error	*/
	UW	BE:1;		/* Recv Buffer Overflow Error */
	/* Same as "RsStat" until here */
	UW	Timout:1;	/* Time out	*/
	UW	Aborted:1;	/* Aborted	*/
	UW	rsv1:2;
	UW	ErrorClass:16;	/* Error class = "EC_IO" */
#endif
} RsError;

/* DN_RS16450:	Hardware configuration (compliant with 16450) (RW) */
typedef struct {
	UW	iobase;		/* Top address of I/O space */
	UW	iostep;		/* Interval between "I/O" addresses for each register */
	INTVEC	intvec;		/* Interrupt vector */
} RsHwConf_16450;

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_RS_H__ */
