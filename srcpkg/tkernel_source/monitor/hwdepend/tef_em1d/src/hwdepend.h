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
 *	hwdepend.h
 *
 *       T-Monitor hardware-dependent processing
 */

#ifndef __MONITOR_CMDSVC_HWDEPEND_H__
#define __MONITOR_CMDSVC_HWDEPEND_H__

#include <tmonitor.h>
#include "sysdepend.h"

IMPORT UW		DipSw;		/* dip switch status */

/*
 * system configuration information
 */
IMPORT MEMSEG		MemSeg[];	/* memory area definition */
IMPORT W		N_MemSeg;	/* number of memory areas */

IMPORT const CFGSIO	ConfigSIO[];	/* serial port configuration definition */
IMPORT const W		N_ConfigSIO;	/* serial port number */

IMPORT const CFGDISK	ConfigDisk[];	/* disk drive configuration definition */
IMPORT const W		N_ConfigDisk;	/* nuber of disk drives */

/*
 * initial processing after reset
 */
IMPORT void procReset( void );

/*
 * initialize hardware (peripherals)
 */
IMPORT void initHardware( void );

/*
 * setting up the initial count for micro-wait()
 */
IMPORT void setupWaitUsec( void );

/*
 * obtain the console port number
 *       console port number (0 - )
 *       if there is no console port, return -1.
 */
IMPORT W getConPort( void );

#endif /* __MONITOR_CMDSVC_HWDEPEND_H__ */
