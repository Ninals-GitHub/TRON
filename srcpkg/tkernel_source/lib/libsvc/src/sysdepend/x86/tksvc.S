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
#include <sys/svc/tkfncd.h>
#include <sys/svc/svc.h>

define_svc_arg1 tk_cre_tsk TFN_CRE_TSK
define_svc_arg1 tk_del_tsk TFN_DEL_TSK
define_svc_arg2 tk_sta_tsk TFN_STA_TSK
define_svc_arg0 tk_ext_tsk TFN_EXT_TSK
define_svc_arg0 tk_exd_tsk TFN_EXD_TSK
define_svc_arg1 tk_ter_tsk TFN_TER_TSK
define_svc_arg0 tk_dis_dsp TFN_DIS_DSP
define_svc_arg0 tk_ena_dsp TFN_ENA_DSP
define_svc_arg2 tk_chg_pri TFN_CHG_PRI
define_svc_arg2 tk_chg_slt TFN_CHG_SLT
define_svc_arg1 tk_rot_rdq TFN_ROT_RDQ
define_svc_arg1 tk_rel_wai TFN_REL_WAI
define_svc_arg0 tk_get_tid TFN_GET_TID
define_svc_arg2 tk_get_tsp TFN_GET_TSP
define_svc_arg2 tk_set_tsp TFN_SET_TSP
define_svc_arg1 tk_get_rid TFN_GET_RID
define_svc_arg2 tk_set_rid TFN_SET_RID
define_svc_arg4 tk_get_reg TFN_GET_REG
define_svc_arg4 tk_set_reg TFN_SET_REG
define_svc_arg2 tk_get_cpr TFN_GET_CPR
define_svc_arg2 tk_set_cpr TFN_SET_CPR
define_svc_arg2 tk_inf_tsk TFN_INF_TSK
define_svc_arg2 tk_ref_tsk TFN_REF_TSK
define_svc_arg2 tk_def_tex TFN_DEF_TEX
define_svc_arg2 tk_ena_tex TFN_ENA_TEX
define_svc_arg2 tk_dis_tex TFN_DIS_TEX
define_svc_arg2 tk_ras_tex TFN_RAS_TEX
define_svc_arg1 tk_end_tex TFN_END_TEX
define_svc_arg2 tk_ref_tex TFN_REF_TEX
define_svc_arg1 tk_sus_tsk TFN_SUS_TSK
define_svc_arg1 tk_rsm_tsk TFN_RSM_TSK
define_svc_arg1 tk_frsm_tsk TFN_FRSM_TSK
define_svc_arg1 tk_slp_tsk TFN_SLP_TSK
define_svc_arg1 tk_wup_tsk TFN_WUP_TSK
define_svc_arg1 tk_can_wup TFN_CAN_WUP
define_svc_arg2 tk_sig_tev TFN_SIG_TEV
define_svc_arg2 tk_wai_tev TFN_WAI_TEV
define_svc_arg2 tk_dis_wai TFN_DIS_WAI
define_svc_arg1 tk_ena_wai TFN_ENA_WAI
define_svc_arg1 tk_cre_sem TFN_CRE_SEM
define_svc_arg1 tk_del_sem TFN_DEL_SEM
define_svc_arg2 tk_sig_sem TFN_SIG_SEM
define_svc_arg3 tk_wai_sem TFN_WAI_SEM
define_svc_arg2 tk_ref_sem TFN_REF_SEM
define_svc_arg1 tk_cre_mtx TFN_CRE_MTX
define_svc_arg1 tk_del_mtx TFN_DEL_MTX
define_svc_arg2 tk_loc_mtx TFN_LOC_MTX
define_svc_arg1 tk_unl_mtx TFN_UNL_MTX
define_svc_arg2 tk_ref_mtx TFN_REF_MTX
define_svc_arg1 tk_cre_flg TFN_CRE_FLG
define_svc_arg1 tk_del_flg TFN_DEL_FLG
define_svc_arg2 tk_set_flg TFN_SET_FLG
define_svc_arg2 tk_clr_flg TFN_CLR_FLG
define_svc_arg5 tk_wai_flg TFN_WAI_FLG
define_svc_arg2 tk_ref_flg TFN_REF_FLG
define_svc_arg1 tk_cre_mbx TFN_CRE_MBX
define_svc_arg1 tk_del_mbx TFN_DEL_MBX
define_svc_arg2 tk_snd_mbx TFN_SND_MBX
define_svc_arg3 tk_rcv_mbx TFN_RCV_MBX
define_svc_arg2 tk_ref_mbx TFN_REF_MBX
define_svc_arg1 tk_cre_mbf TFN_CRE_MBF
define_svc_arg1 tk_del_mbf TFN_DEL_MBF
define_svc_arg4 tk_snd_mbf TFN_SND_MBF
define_svc_arg3 tk_rcv_mbf TFN_RCV_MBF
define_svc_arg2 tk_ref_mbf TFN_REF_MBF
define_svc_arg1 tk_cre_por TFN_CRE_POR
define_svc_arg1 tk_del_por TFN_DEL_POR
define_svc_arg5 tk_cal_por TFN_CAL_POR
define_svc_arg5 tk_acp_por TFN_ACP_POR
define_svc_arg5 tk_fwd_por TFN_FWD_POR
define_svc_arg3 tk_rpl_rdv TFN_RPL_RDV
define_svc_arg2 tk_ref_por TFN_REF_POR
define_svc_arg2 tk_def_int TFN_DEF_INT
define_svc_arg0 tk_ret_int TFN_RET_INT
define_svc_arg1 tk_cre_mpl TFN_CRE_MPL
define_svc_arg1 tk_del_mpl TFN_DEL_MPL
define_svc_arg4 tk_get_mpl TFN_GET_MPL
define_svc_arg2 tk_rel_mpl TFN_REL_MPL
define_svc_arg2 tk_ref_mpl TFN_REF_MPL
define_svc_arg1 tk_cre_mpf TFN_CRE_MPF
define_svc_arg1 tk_del_mpf TFN_DEL_MPF
define_svc_arg3 tk_get_mpf TFN_GET_MPF
define_svc_arg2 tk_rel_mpf TFN_REL_MPF
define_svc_arg2 tk_ref_mpf TFN_REF_MPF
define_svc_arg1 tk_set_tim TFN_SET_TIM
define_svc_arg1 tk_get_tim TFN_GET_TIM
define_svc_arg1 tk_get_otm TFN_GET_OTM
define_svc_arg1 tk_dly_tsk TFN_DLY_TSK
define_svc_arg1 tk_cre_cyc TFN_CRE_CYC
define_svc_arg1 tk_del_cyc TFN_DEL_CYC
define_svc_arg1 tk_sta_cyc TFN_STA_CYC
define_svc_arg1 tk_stp_cyc TFN_STP_CYC
define_svc_arg2 tk_ref_cyc TFN_REF_CYC
define_svc_arg1 tk_cre_alm TFN_CRE_ALM
define_svc_arg1 tk_del_alm TFN_DEL_ALM
define_svc_arg2 tk_sta_alm TFN_STA_ALM
define_svc_arg1 tk_stp_alm TFN_STP_ALM
define_svc_arg2 tk_ref_alm TFN_REF_ALM
define_svc_arg1 tk_ref_ver TFN_REF_VER
define_svc_arg1 tk_ref_sys TFN_REF_SYS
define_svc_arg2 tk_def_ssy TFN_DEF_SSY
define_svc_arg3 tk_sta_ssy TFN_STA_SSY
define_svc_arg3 tk_cln_ssy TFN_CLN_SSY
define_svc_arg4 tk_evt_ssy TFN_EVT_SSY
define_svc_arg2 tk_ref_ssy TFN_REF_SSY
define_svc_arg0 tk_cre_res TFN_CRE_RES
define_svc_arg1 tk_del_sys TFN_DEL_RES
define_svc_arg3 tk_get_res TFN_GET_RES
define_svc_arg1 tk_set_pow TFN_SET_POW
define_svc_arg2 tk_chg_slt_u TFN_CHG_SLT_U
define_svc_arg3 tk_inf_tsk_u TFN_INF_TSK_U
define_svc_arg2 tk_ref_tsk_u TFN_REF_TSK_U
define_svc_arg1 tk_slp_tsk_u TFN_SLP_TSK_U
define_svc_arg2 tk_wai_tev_u TFN_WAI_TEV_U
define_svc_arg3 tk_dly_tsk_u TFN_DLY_TSK_U
define_svc_arg3 tk_wai_sem_u TFN_WAI_SEM_U
define_svc_arg5 tk_wai_flg_u TFN_WAI_FLG_U
define_svc_arg3 tk_rcv_mbx_u TFN_RCV_MBX_U
define_svc_arg2 tk_loc_mtx_u TFN_LOC_MTX_U
define_svc_arg4 tk_snd_mbf_u TFN_SND_MBF_U
define_svc_arg3 tk_rcv_mbf_u TFN_RCV_MBF_U
define_svc_arg5 tk_cal_por_u TFN_CAL_POR_U
define_svc_arg5 tk_acp_por_u TFN_ACP_POR_U
define_svc_arg4 tk_get_mpl_u TFN_GET_MPL_U
define_svc_arg3 tk_get_mpf_u TFN_GET_MPF_U
define_svc_arg1 tk_set_tim_u TFN_SET_TIM_U
define_svc_arg1 tk_get_tim_u TFN_GET_TIM_U
define_svc_arg2 tk_get_otm_u TFN_GET_OTM_U
define_svc_arg1 tk_cre_cyc_u TFN_CRE_CYC_U
define_svc_arg2 tk_ref_cyc_u TFN_REF_CYC_U
define_svc_arg2 tk_sta_alm_u TFN_STA_ALM_U
define_svc_arg2 tk_ref_alm_u TFN_REF_ALM_U
