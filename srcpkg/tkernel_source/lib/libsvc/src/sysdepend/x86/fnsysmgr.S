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
#include <sys/svc/fnsysmgr.h>
#include <sys/svc/svc.h>

define_svc_arg3 tk_get_cfn SYSTEM_TK_GET_CFN_FN
define_svc_arg3 tk_get_cfs SYSTEM_TK_GET_CFS_FN
define_svc_arg3 tk_get_smb SYSTEM_TK_GET_SMB_FN
define_svc_arg1 tk_rel_smb SYSTEM_TK_REL_SMB_FN
define_svc_arg1 tk_ref_smb SYSTEM_TK_REF_SMB_FN
define_svc_arg1 _GetKernelCommonArea SYSTEM__GETKERNELCOMMONAREA_FN
define_svc_arg1 _syslog_send SYSTEM__SYSLOG_SEND_FN
