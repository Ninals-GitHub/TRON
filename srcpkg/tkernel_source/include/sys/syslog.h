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
 *	@(#)syslog.h (sys)
 *
 *	System log output library
 *
 *	The system log is displayed on the debugging console.
 */

#ifndef	__SYS_SYSLOG_H__
#define __SYS_SYSLOG_H__

#include <basic.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Level
 */
#define LOG_EMERG	0		/* System down */
#define LOG_ALERT	1		/* Error -
					   immediate termination required */
#define LOG_CRIT	2		/* Error -
					   due to failure */
#define LOG_ERR		3		/* Other error */
#define LOG_WARNING	4		/* Caution */
#define LOG_NOTICE	5		/* Reference information */
#define LOG_INFO	6		/* General information */
#define LOG_DEBUG	7		/* Debugging information */

#define LOG_PRIMASK	0x0007U		/* Level mask value */

/*
 * Facility
 */
#define LOG_OTHER	(0 << 3)	/* Other */
#define LOG_KERN	(1 << 3)	/* Kernel */
#define LOG_SUBSYS	(2 << 3)	/* Subsystem */
#define LOG_DEVDRV	(3 << 3)	/* Device driver */

#define LOG_NFACILITIES	16		/* Facility number */
#define LOG_FACMASK	0x0078U		/* Facility mask value */

/*
 * Generate system log mask value
 */
#define LOG_MASK(level)		( 1 << (level) )
#define LOG_UPTO(level)		( (1 << ((level) + 1)) - 1 )

/*
 * priority = level | facility
 */
extern void syslog( int priority, const char *format, ... );
extern int setlogmask( int mask );

extern int _syslog_send( const char *string, int len );	/* T-Kernel/SM */
Inline void syslog_wait( void )
{
	_syslog_send((const char*)0, 0);
}

#ifdef __cplusplus
}
#endif
#endif /* __SYS_SYSLOG_H__ */
