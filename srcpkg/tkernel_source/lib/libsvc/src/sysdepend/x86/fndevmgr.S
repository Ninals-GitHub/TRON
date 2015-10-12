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
#include <sys/svc/fndevmgr.h>
#include <sys/svc/svc.h>

define_svc_arg2 tk_opn_dev DEVICE_TK_OPN_DEV_FN
define_svc_arg2 tk_cls_dev DEVICE_TK_CLS_DEV_FN
define_svc_arg5 tk_rea_dev DEVICE_TK_REA_DEV_FN
define_svc_arg5 tk_srea_dev DEVICE_TK_SREA_DEV_FN
define_svc_arg5 tk_wri_dev DEVICE_TK_WRI_DEV_FN
define_svc_arg5 tk_swri_dev DEVICE_TK_SWRI_DEV_FN
define_svc_arg5 tk_wai_dev DEVICE_TK_WAI_DEV_FN
define_svc_arg1 tk_sus_dev DEVICE_TK_SUS_DEV_FN
define_svc_arg2 tk_get_dev DEVICE_TK_GET_DEV_FN
define_svc_arg2 tk_ref_dev DEVICE_TK_REF_DEV_FN
define_svc_arg2 tk_oref_dev DEVICE_TK_OREF_DEV_FN
define_svc_arg3 tk_lst_dev DEVICE_TK_LST_DEV_FN
define_svc_arg3 tk_evt_dev DEVICE_TK_EVT_DEV_FN
define_svc_arg5 tk_rea_dev_du DEVICE_TK_REA_DEV_DU_FN
define_svc_arg5 tk_srea_dev_d DEVICE_TK_SREA_DEV_D_FN
define_svc_arg5 tk_wri_dev_du DEVICE_TK_WRI_DEV_DU_FN
define_svc_arg5 tk_swri_dev_d DEVICE_TK_SWRI_DEV_D_FN
define_svc_arg5 tk_wai_dev_u DEVICE_TK_WAI_DEV_U_FN
define_svc_arg4 tk_def_dev DEVICE_TK_DEF_DEV_FN
define_svc_arg1 tk_ref_idv DEVICE_TK_REF_IDV_FN

