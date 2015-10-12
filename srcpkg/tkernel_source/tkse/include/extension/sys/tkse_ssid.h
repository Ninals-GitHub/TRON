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
 *	ssid.h
 *
 *	Subsystem ID and subsystem priority
 *
 *	(Note) Describe only macros because data is included from
 *	     assembler source as well.
 */

#ifndef __EXTENSION_SYS_TKSE_SSID_H__
#define __EXTENSION_SYS_TKSE_SSID_H__

/*
 * T-Kernel (1 - 9)
 *	1 to 6 and 9 were used for other purposes before.
 */
#define	CONSIO_SVC	1		/* Console I/O */
#define	CONSIO_PRI		1
#define	SERIAL_SVC	5		/* Low-level serial I/O */
#define	SERIAL_PRI		1
#define	DEVICE_SVC	8		/* T-Kernel/SM device management */
#define	DEVICE_PRI		4
#define	SYSTEM_SVC	9		/* T-Kernel/SM system management */
#define	SYSTEM_PRI		1

/*
 * Subsystem (10 -)
 */
#define	PM_SVC		10		/* Process management		*/
#define	PM_PRI			2
#define	MM_SVC		11		/* Memory management		*/
#define	MM_PRI			2
#define	FM_SVC		12		/* File management		*/
#define	FM_PRI			2
#define	EM_SVC		13		/* Event management		*/
#define	EM_PRI			6
#define	DM_SVC		14		/* Device management		*/
#define	DM_PRI			6
#define	CM_SVC		15		/* Clock management		*/
#define	CM_PRI			6
#define	SM_SVC		16		/* System management		*/
#define	SM_PRI			2
#define	SEG_SVC		17		/* Segment management		*/
#define	SEG_PRI			2
#define	MSG_SVC		18		/* Message management		*/
#define	MSG_PRI			6
#define	TCM_SVC		19		/* Inter-task synchronous communication management		*/
#define	TCM_PRI			6
#define	NM_SVC		20		/* Global name management	*/
#define	NM_PRI			6
#define	KS_SVC		21		/* Kana-kanji conversion server	*/
#define	KS_PRI			8
#define	SF_SVC		22		/* Standard file IO		*/
#define	SF_PRI			8
#define	BDM_SVC		23		/* Debug manager		*/
#define	BDM_PRI			8
#define	USB_SVC		24		/* USB manager			*/
#define	USB_PRI			8
#define	H8IO_SVC	25		/* (T-Engine) H8 input/output	*/
#define	H8IO_PRI		8
#define	FP_SVC		31		/* Font manager			*/
#define	FP_PRI			12
#define	DP_SVC		32		/* Display primitive		*/
#define	DP_PRI			12
#define	HMI_SVC		33		/* HMI manager			*/
#define	HMI_PRI			12
#define	VO_SVC		34		/* Entity/avatar manager	*/
#define	VO_PRI			12
#define	TIP_SVC		35		/* Text input primitive		*/
#define	TIP_PRI			12
#define	PRM_SVC		36		/* Print manager		*/
#define	PRM_PRI			12
#define	SO_SVC		38		/* TCPIP manager		*/
#define	SO_PRI			8
#define	PCM_SVC		39		/* PCMCIA manager		*/
#define	PCM_PRI			8

#endif /* __EXTENSION_SYS_TKSE_SSID_H__ */
