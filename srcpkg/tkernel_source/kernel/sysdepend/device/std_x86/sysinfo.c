/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */

#include <tk/typedef.h>
#include "sysinit.h"
#include <tk/kernel.h>
#include <tk/sysdef.h>
#include <sys/sysinfo.h>
#include <sys/rominfo.h>
#include <device/devconf.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/

/*
==================================================================================

	Management 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	system common information
----------------------------------------------------------------------------------
*/
LOCAL SysCommonArea _SCArea;
EXPORT SysCommonArea *SCArea;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:initSysCommonArea
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize ststem common area
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void initSysCommonArea(void)
{
	W	val[L_DEVCONF_VAL];
	W	n;

	_SCArea.scinfo.bm.c.basic = 1;
	_SCArea.scinfo.ramtop = (void*)0L;
	_SCArea.scinfo.ramend = (void*)((unsigned long)_SCArea.scinfo.ramtop +
						getBootInfo()->lowmem_limit);

	SCArea = (SysCommonArea*)&_SCArea;

	/* Set SYSCONF/DEVCONF */
	SCInfo.sysconf = ROMInfo->sysconf;
	SCInfo.devconf = ROMInfo->devconf;

	/* Set debug mode */
	n = GetDevConf(DCTAG_DEBUGMODE, val);
	SCInfo.bm.c.debug = ( n >= 1 && val[0] > 0 )? 1: 0;

	/* Set boot device (no boot device) */
	SCInfo.bootdev[0] = '\0';
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
