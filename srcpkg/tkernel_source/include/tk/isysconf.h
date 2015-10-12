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
 *	isysconf.h (T-Kernel/OS)
 *	Default Value Definition for System Configuration File
 */

#ifndef _ISYSCONF_
#define _ISYSCONF_

/*
 * Definition of unused system call
 */
#ifndef NUM_SEMID
#define _tk_cre_sem	no_support
#define _tk_del_sem	no_support
#define _tk_sig_sem	no_support
#define _tk_wai_sem	no_support
#define _tk_ref_sem	no_support
#define	_tk_wai_sem_u	no_support

#define _td_lst_sem	no_support
#define _td_ref_sem	no_support
#define _td_sem_que	no_support
#endif

#ifndef NUM_FLGID
#define _tk_cre_flg	no_support
#define _tk_del_flg	no_support
#define _tk_set_flg	no_support
#define _tk_clr_flg	no_support
#define _tk_wai_flg	no_support
#define _tk_ref_flg	no_support
#define	_tk_wai_flg_u	no_support

#define _td_lst_flg	no_support
#define _td_ref_flg	no_support
#define _td_flg_que	no_support
#endif

#ifndef NUM_MBXID
#define _tk_cre_mbx	no_support
#define _tk_del_mbx	no_support
#define _tk_snd_mbx	no_support
#define _tk_rcv_mbx	no_support
#define _tk_ref_mbx	no_support
#define	_tk_rcv_mbx_u	no_support

#define _td_lst_mbx	no_support
#define _td_ref_mbx	no_support
#define _td_mbx_que	no_support
#endif

#ifndef NUM_MBFID
#define _tk_cre_mbf	no_support
#define _tk_del_mbf	no_support
#define _tk_snd_mbf	no_support
#define _tk_rcv_mbf	no_support
#define _tk_ref_mbf	no_support
#define	_tk_snd_mbf_u	no_support
#define	_tk_rcv_mbf_u	no_support

#define _td_lst_mbf	no_support
#define _td_ref_mbf	no_support
#define _td_smbf_que	no_support
#define _td_rmbf_que	no_support
#endif

#ifndef NUM_PORID
#define _tk_cre_por	no_support
#define _tk_del_por	no_support
#define _tk_cal_por	no_support
#define _tk_acp_por	no_support
#define _tk_fwd_por	no_support
#define _tk_rpl_rdv	no_support
#define _tk_ref_por	no_support
#define	_tk_cal_por_u	no_support
#define	_tk_acp_por_u	no_support

#define _td_lst_por	no_support
#define _td_ref_por	no_support
#define _td_acp_que	no_support
#define _td_cal_que	no_support
#endif

#ifndef NUM_MTXID
#define _tk_cre_mtx	no_support
#define _tk_del_mtx	no_support
#define _tk_loc_mtx	no_support
#define _tk_unl_mtx	no_support
#define _tk_ref_mtx	no_support
#define	_tk_loc_mtx_u	no_support

#define _td_lst_mtx	no_support
#define _td_ref_mtx	no_support
#define _td_mtx_que	no_support
#endif

#ifndef NUM_MPLID
#define _tk_cre_mpl	no_support
#define _tk_del_mpl	no_support
#define _tk_get_mpl	no_support
#define _tk_rel_mpl	no_support
#define _tk_ref_mpl	no_support
#define	_tk_get_mpl_u	no_support

#define _td_lst_mpl	no_support
#define _td_ref_mpl	no_support
#define _td_mpl_que	no_support
#endif

#ifndef NUM_MPFID
#define _tk_cre_mpf	no_support
#define _tk_del_mpf	no_support
#define _tk_get_mpf	no_support
#define _tk_rel_mpf	no_support
#define _tk_ref_mpf	no_support
#define	_tk_get_mpf_u	no_support

#define _td_lst_mpf	no_support
#define _td_ref_mpf	no_support
#define _td_mpf_que	no_support
#endif

#ifndef NUM_CYCID
#define _tk_cre_cyc	no_support
#define _tk_del_cyc	no_support
#define _tk_sta_cyc	no_support
#define _tk_stp_cyc	no_support
#define _tk_ref_cyc	no_support
#define	_tk_cre_cyc_u	no_support
#define	_tk_ref_cyc_u	no_support

#define _td_lst_cyc	no_support
#define _td_ref_cyc	no_support
#define	_td_ref_cyc_u	no_support
#endif

#ifndef NUM_ALMID
#define _tk_cre_alm	no_support
#define _tk_del_alm	no_support
#define _tk_sta_alm	no_support
#define _tk_stp_alm	no_support
#define _tk_ref_alm	no_support
#define	_tk_sta_alm_u	no_support
#define	_tk_ref_alm_u	no_support

#define _td_lst_alm	no_support
#define _td_ref_alm	no_support
#define	_td_ref_alm_u	no_support
#endif

#endif /* _ISYSCONF_ */
