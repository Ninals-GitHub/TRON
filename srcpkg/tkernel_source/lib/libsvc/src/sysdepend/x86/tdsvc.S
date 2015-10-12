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
#include <sys/svc/tdfncd.h>
#include <sys/svc/svc.h>

define_td_svc_arg2 td_lst_tsk TDFN_LST_TSK
define_td_svc_arg2 td_lst_sem TDFN_LST_SEM
define_td_svc_arg2 td_lst_flg TDFN_LST_FLG
define_td_svc_arg2 td_lst_mbx TDFN_LST_MBX
define_td_svc_arg2 td_lst_mtx TDFN_LST_MTX
define_td_svc_arg2 td_lst_mbf TDFN_LST_MBF
define_td_svc_arg2 td_lst_por TDFN_LST_POR
define_td_svc_arg2 td_lst_mpf TDFN_LST_MPF
define_td_svc_arg2 td_lst_mpl TDFN_LST_MPL
define_td_svc_arg2 td_lst_cyc TDFN_LST_CYC
define_td_svc_arg2 td_lst_alm TDFN_LST_ALM
define_td_svc_arg2 td_lst_ssy TDFN_LST_SSY
define_td_svc_arg2 td_ref_sem TDFN_REF_SEM
define_td_svc_arg2 td_ref_flg TDFN_REF_FLG
define_td_svc_arg2 td_ref_mbx TDFN_REF_MBX
define_td_svc_arg2 td_ref_mtx TDFN_REF_MTX
define_td_svc_arg2 td_ref_mbf TDFN_REF_MBF
define_td_svc_arg2 td_ref_por TDFN_REF_POR
define_td_svc_arg2 td_ref_mpf TDFN_REF_MPF
define_td_svc_arg2 td_ref_mpl TDFN_REF_MPL
define_td_svc_arg2 td_ref_cyc TDFN_REF_CYC
define_td_svc_arg2 td_ref_alm TDFN_REF_ALM
define_td_svc_arg2 td_ref_ssy TDFN_REF_SSY
define_td_svc_arg2 td_ref_tsk TDFN_REF_TSK
define_td_svc_arg2 td_ref_tex TDFN_REF_TEX
define_td_svc_arg3 td_inf_tsk TDFN_INF_TSK
define_td_svc_arg4 td_get_reg TDFN_GET_REG
define_td_svc_arg4 td_seg_reg TDFN_SET_REG
define_td_svc_arg1 td_ref_sys TDFN_REF_SYS
define_td_svc_arg2 td_get_tim TDFN_GET_TIM
define_td_svc_arg2 td_get_otm TDFN_GET_OTM
define_td_svc_arg3 td_rdy_que TDFN_RDY_QUE
define_td_svc_arg3 td_sem_que TDFN_SEM_QUE
define_td_svc_arg3 td_flg_que TDFN_FLG_QUE
define_td_svc_arg3 td_mbx_que TDFN_MBX_QUE
define_td_svc_arg3 td_mtx_que TDFN_MTX_QUE
define_td_svc_arg3 td_smbf_que TDFN_SMBF_QUE
define_td_svc_arg3 td_rmbf_que TDFN_RMBF_QUE
define_td_svc_arg3 td_cal_que TDFN_CAL_QUE
define_td_svc_arg3 td_acp_que TDFN_ACP_QUE
define_td_svc_arg3 td_mpf_que TDFN_MPF_QUE
define_td_svc_arg3 td_mpl_que TDFN_MPL_QUE
define_td_svc_arg1 td_hok_svc TDFN_HOK_SVC
define_td_svc_arg1 td_hok_dsp TDFN_HOK_DSP
define_td_svc_arg1 td_hok_int TDFN_HOK_INT
define_td_svc_arg3 td_ref_dsname TDFN_REF_DSNAME
define_td_svc_arg3 td_set_dsname TDFN_SET_DSNAME
define_td_svc_arg2 td_ref_cyc_u TDFN_REF_CYC_U
define_td_svc_arg2 td_ref_alm_u TDFN_REF_ALM_U
define_td_svc_arg3 td_ref_task_u TDFN_REF_TSK_U
define_td_svc_arg3 td_inf_tsk_u TDFN_INF_TSK_U
define_td_svc_arg2 td_get_tim_u TDFN_GET_TIM_U
define_td_svc_arg2 td_get_otm_u TDFN_GET_OTM_U


