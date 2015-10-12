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
 *	Extended SVC function code
 *
 */

#include <sys/ssid.h>

#define SEG_LOCKSPACE_FN	(0x01000200 | SEG_SVC)
#define SEG_UNLOCKSPACE_FN	(0x01010200 | SEG_SVC)
#define SEG_CNVPHYSICALADDR_FN	(0x01020300 | SEG_SVC)
#define SEG_CHKSPACE_FN	(0x01030400 | SEG_SVC)
#define SEG_CHKSPACETSTR_FN	(0x01040400 | SEG_SVC)
#define SEG_CHKSPACEBSTR_FN	(0x01050400 | SEG_SVC)
#define SEG_CHKSPACELEN_FN	(0x01060500 | SEG_SVC)
#define SEG_READMEMSPACE_FN	(0x01070400 | SEG_SVC)
#define SEG_WRITEMEMSPACE_FN	(0x01080400 | SEG_SVC)
#define SEG_SETMEMSPACEB_FN	(0x01090400 | SEG_SVC)
#define SEG_FLUSHMEMCACHE_FN	(0x010a0300 | SEG_SVC)
#define SEG_SETCACHEMODE_FN	(0x010b0300 | SEG_SVC)
#define SEG_CONTROLCACHE_FN	(0x010c0300 | SEG_SVC)
#define SEG_GETSPACEINFO_FN	(0x010d0300 | SEG_SVC)
#define SEG_SETMEMORYACCESS_FN	(0x010e0300 | SEG_SVC)
#define SEG_MAPMEMORY_FN	(0x02000400 | SEG_SVC)
#define SEG_UNMAPMEMORY_FN	(0x02010100 | SEG_SVC)
#define SEG_MAKESPACE_FN	(0x10000400 | SEG_SVC)
#define SEG_UNMAKESPACE_FN	(0x10010300 | SEG_SVC)
#define SEG_CHANGESPACE_FN	(0x10020400 | SEG_SVC)

