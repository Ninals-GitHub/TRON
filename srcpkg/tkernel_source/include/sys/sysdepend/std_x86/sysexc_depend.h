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
 *	@(#)sysexc_depend.h (sys/EM1-D512)
 *
 *	System exception processing
 */

#ifndef __SYS_SYSEXC_DEPEND_H__
#define __SYS_SYSEXC_DEPEND_H__

#ifdef __cplusplus
extern "C" {
#endif

/* System exception message */
typedef struct {
	W	type;		/* Message type (MS_SYS1) */
	W	size;		/* Message size */
	ID	taskid;		/* Exception generated task ID */
	ID	procid;		/* Exception generated process ID */
	UW	vecno;		/* Exception vector number */
	UW	excinfo;	/* Exception information (FSR) */
	UW	excaddr;	/* Exception address (FAR) */
} EXCMESG;

#ifdef __cplusplus
}
#endif

#endif /* __SYS_SYSEXC_DEPEND_H__ */
