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
 *	@(#)ssid.h (sys)
 *
 *	Subsystem ID and subsystem priority
 *
 *	(NB) Included also from the assembler source,
 *	     so only macros can be described.
 */

#ifndef __SYS_SSID_H__
#define __SYS_SSID_H__

/*
 * T-Kernel (1 - 9)
 *	1 - 6 and 9 were previously used for other purposes.
 */
#define CONSIO_SVC	1		/* Console I/O */
#define CONSIO_PRI		1
#define DEVICE_SVC	8		/* T-Kernel/SM device management */
#define DEVICE_PRI		4
#define SYSTEM_SVC	9		/* T-Kernel/SM system management */
#define SYSTEM_PRI		1

/*
 * Subsystem (10 - )
 */
#define MM_SVC		11		/* Memory management		*/
#define MM_PRI			2
#define SM_SVC		16		/* System management		*/
#define SM_PRI			2
#define SEG_SVC		17		/* Segment management		*/
#define SEG_PRI			2

#endif /* __SYS_SSID_H__ */
