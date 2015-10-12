/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	T-Kernel SVC interface library (x86)
 *
 */

#include <machine.h>
#include <tk/sysdef.h>
#include <sys/svc/fnsegment.h>
#include <sys/svc/svc.h>

define_svc_arg2 LockSpace SEG_LOCKSPACE_FN
define_svc_arg2 UnlockSpace SEG_UNLOCKSPACE_FN
define_svc_arg3 CnvPhysicalAddr SEG_CNVPHYSICALADDR_FN
define_svc_arg4 ChkSpace SEG_CHKSPACE_FN
define_svc_arg4 ChkSpaceTstr SEG_CHKSPACETSTR_FN
define_svc_arg4 ChkSpaceBstr SEG_CHKSPACEBSTR_FN
define_svc_arg5 ChkSpaceLen SEG_CHKSPACELEN_FN
define_svc_arg4 ReadMemSpace SEG_READMEMSPACE_FN
define_svc_arg4 WriteMemSpace SEG_WRITEMEMSPACE_FN
define_svc_arg4 SetMemSpaceB SEG_SETMEMSPACEB_FN
define_svc_arg3 FlushMemCache SEG_FLUSHMEMCACHE_FN
define_svc_arg3 SetCacheMode SEG_SETCACHEMODE_FN
define_svc_arg3 ControlCache SEG_CONTROLCACHE_FN
define_svc_arg3 GetSpaceInfo SEG_GETSPACEINFO_FN
define_svc_arg3 SetMemoryAccess SEG_SETMEMORYACCESS_FN
define_svc_arg4 MapMemory SEG_MAPMEMORY_FN
define_svc_arg1 UnmapMemory SEG_UNMAPMEMORY_FN
define_svc_arg4 MakeSpace SEG_MAKESPACE_FN
define_svc_arg3 UnmakeSpace SEG_UNMAKESPACE_FN
define_svc_arg4 ChangeSpace SEG_CHANGESPACE_FN


