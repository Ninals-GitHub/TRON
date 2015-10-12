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

#define SYSTEM_TK_GET_CFN_FN	(0x00010300 | SYSTEM_SVC)
#define SYSTEM_TK_GET_CFS_FN	(0x00020300 | SYSTEM_SVC)
#define SYSTEM_TK_GET_SMB_FN	(0x01000300 | SYSTEM_SVC)
#define SYSTEM_TK_REL_SMB_FN	(0x01010100 | SYSTEM_SVC)
#define SYSTEM_TK_REF_SMB_FN	(0x01020100 | SYSTEM_SVC)
#define SYSTEM__GETKERNELCOMMONAREA_FN	(0x10000100 | SYSTEM_SVC)
#define SYSTEM__SYSLOG_SEND_FN	(0x10010200 | SYSTEM_SVC)

