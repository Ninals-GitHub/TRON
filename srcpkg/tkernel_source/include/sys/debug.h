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
 *	@(#)debug.h (sys)
 *
 *	Debug support
 */

#ifndef	__SYS_DEBUG_H__
#define __SYS_DEBUG_H__

#include <basic.h>

#ifdef DEBUG
#include <tm/tmonitor.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Example
 *	DEBUG_PRINT(("error = %d\n", ercd));
 *
 *	DO_DEBUG( if ( ercd < ER_OK ) DEBUG_PRINT(("error = %d\n", ercd)); )
 */
#ifdef DEBUG

#ifndef DEBUG_MODULE
#define DEBUG_MODULE	""	/* Normally define like "(module name)" */
#endif

#define DEBUG_PRINT(arg)						\
	(								\
		printf("%s#%d%s:", __FILE__, __LINE__, DEBUG_MODULE),	\
		printf arg						\
	)
#define BMS_DEBUG_PRINT(arg)						\
	(								\
		bms_printf("%s#%d%s:", __FILE__, __LINE__, DEBUG_MODULE), \
		bms_printf arg						\
	)

#define DO_DEBUG(exp)	{ exp }

#else /* DEBUG */

#define DEBUG_PRINT(arg)	/* arg */
#define BMS_DEBUG_PRINT(arg)	/* arg */
#define DO_DEBUG(exp)		/* exp */

#endif /* DEBUG */

#ifdef __cplusplus
}
#endif
#endif /* __SYS_DEBUG_H__ */
