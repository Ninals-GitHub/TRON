/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/09/22
 *
 *----------------------------------------------------------------------
 */

/*
 *	sysinit.h (sysinit)
 *	Initialize System
 */

#ifndef _SYSINIT_
#define _SYSINIT_

#include <basic.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

/*
 * Get system configuration information (SYSCONF)
 */
IMPORT W GetSysConf( UB *name, W *val );
IMPORT W GetSysConfStr( UB *name, UB *str );

/*
 * Get device configuration information (DEVCONF)
 */
IMPORT W GetDevConf( UB *name, W *val );
IMPORT W GetDevConfStr( UB *name, UB *str );

/*
 * Platform dependent sequence
 */
IMPORT ER init_device( void );
IMPORT ER start_device( void );
IMPORT ER finish_device( void );
IMPORT ER restart_device( W mode );
IMPORT void DispProgress( W n );

/* ------------------------------------------------------------------------ */

#define IMPORT_DEFINE   1
#if IMPORT_DEFINE
IMPORT ER init_system( void );
IMPORT void start_system( void );
IMPORT void shutdown_system( INT fin );
#endif

#endif /* _SYSINIT_ */
