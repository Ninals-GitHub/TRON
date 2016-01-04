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
 *	isyscall.h (T-Kernel/OS)
 *	System Call Int. Format Definition
 */

#ifndef _ISYSCALL_
#define _ISYSCALL_

/* ------------------------------------------------------------------------ */

#if TA_GP
# define _CALL(p1, p2, p3, hdr, cb)	\
		CallUserHandler((INT)(p1), (INT)(p2), (INT)(p3), (FP)(hdr), (cb)->gp)
# define CallUserHandlerP1(   p1,         hdr, cb)	_CALL(p1, 0,  0,  hdr, cb)
# define CallUserHandlerP2(   p1, p2,     hdr, cb)	_CALL(p1, p2, 0,  hdr, cb)
# define CallUserHandlerP3(   p1, p2, p3, hdr, cb)	_CALL(p1, p2, p3, hdr, cb)
# define CallUserHandlerP2_GP(p1, p2,     hdr, cb)	_CALL(p1, p2, gp, hdr, cb)
#else
# define CallUserHandlerP1(   p1,         hdr, cb)	(*(hdr))(p1)
# define CallUserHandlerP2(   p1, p2,     hdr, cb)	(*(hdr))(p1, p2)
# define CallUserHandlerP3(   p1, p2, p3, hdr, cb)	(*(hdr))(p1, p2, p3)
# define CallUserHandlerP2_GP(p1, p2,     hdr, cb)	(*(hdr))(p1, p2)
#endif

/* ------------------------------------------------------------------------ */

#if TA_GP
#define P0(void)		( int _1,int _2,int _3,int _4,int _5, void *gp )
#define P1(p1)			( p1,    int _2,int _3,int _4,int _5, void *gp )
#define P2(p1, p2)		( p1, p2,       int _3,int _4,int _5, void *gp )
#define P3(p1, p2, p3)		( p1, p2, p3,          int _4,int _5, void *gp )
#define P4(p1, p2, p3, p4)	( p1, p2, p3, p4,             int _5, void *gp )
#define P5(p1, p2, p3, p4, p5)	( p1, p2, p3, p4, p5,                 void *gp )
#define P2GP(p1, p2)		( p1, p2,			      void *gp )
#else
#define P0(void)		( void )
#define P1(p1)			( p1 )
#define P2(p1, p2)		( p1, p2 )
#define P3(p1, p2, p3)		( p1, p2, p3 )
#define P4(p1, p2, p3, p4)	( p1, p2, p3, p4 )
#define P5(p1, p2, p3, p4, p5)	( p1, p2, p3, p4, p5 )
#define P2GP(p1, p2)		( p1, p2 )
#endif

/* ------------------------------------------------------------------------ */

/* T-Kernel/OS */
IMPORT ID _tk_cre_tsk P1( CONST T_CTSK *pk_ctsk );
IMPORT ER _tk_del_tsk( ID tskid );
IMPORT ER _tk_sta_tsk( ID tskid, INT stacd );
IMPORT void _tk_ext_tsk( void );
IMPORT void _tk_exd_tsk( void );
IMPORT ER _tk_ter_tsk( ID tskid );
IMPORT ER _tk_dis_dsp( void );
IMPORT ER _tk_ena_dsp( void );
IMPORT ER _tk_chg_pri( ID tskid, PRI tskpri );
IMPORT ER _tk_chg_slt( ID tskid, RELTIM slicetime );
IMPORT ER _tk_rot_rdq( PRI tskpri );
IMPORT ER _tk_rel_wai( ID tskid );
IMPORT ID _tk_get_tid( void );
IMPORT ER _tk_get_tsp( ID tskid, T_TSKSPC *pk_tskspc );
IMPORT ER _tk_set_tsp( ID tskid, CONST T_TSKSPC *pk_tskspc );
IMPORT ID _tk_get_rid( ID tskid );
IMPORT ID _tk_set_rid( ID tskid, ID resid );
IMPORT ER _tk_get_reg( ID tskid, T_REGS *pk_regs, T_EIT *pk_eit, T_CREGS *pk_cregs );
IMPORT ER _tk_set_reg( ID tskid, CONST T_REGS *pk_regs, CONST T_EIT *pk_eit, CONST T_CREGS *pk_cregs );
IMPORT ER _tk_get_cpr( ID tskid, INT copno, T_COPREGS *pk_copregs );
IMPORT ER _tk_set_cpr( ID tskid, INT copno, CONST T_COPREGS *pk_copregs );
IMPORT ER _tk_inf_tsk( ID tskid, T_ITSK *pk_itsk, BOOL clr );
IMPORT ER _tk_ref_tsk( ID tskid, T_RTSK *pk_rtsk );
IMPORT ER _tk_def_tex( ID tskid, CONST T_DTEX *pk_dtex );
IMPORT ER _tk_ena_tex( ID tskid, UINT texptn );
IMPORT ER _tk_dis_tex( ID tskid, UINT texptn );
IMPORT ER _tk_ras_tex( ID tskid, INT texcd );
IMPORT INT _tk_end_tex( BOOL enatex );
IMPORT ER _tk_ref_tex( ID tskid, T_RTEX *pk_rtex );
IMPORT ER _tk_sus_tsk( ID tskid );
IMPORT ER _tk_rsm_tsk( ID tskid );
IMPORT ER _tk_frsm_tsk( ID tskid );
IMPORT ER _tk_slp_tsk( TMO tmout );
IMPORT ER _tk_wup_tsk( ID tskid );
IMPORT INT _tk_can_wup( ID tskid );
IMPORT ER _tk_sig_tev( ID tskid, INT tskevt );
IMPORT INT _tk_wai_tev( INT waiptn, TMO tmout );
IMPORT INT _tk_dis_wai( ID tskid, UINT waitmask );
IMPORT ER _tk_ena_wai( ID tskid );
IMPORT ER _tk_chg_slt_u( ID tskid, RELTIM_U slicetime_u );
IMPORT ER _tk_inf_tsk_u( ID tskid, T_ITSK_U *pk_itsk_u, BOOL clr );
IMPORT ER _tk_ref_tsk_u( ID tskid, T_RTSK_U *pk_rtsk_u );
IMPORT ER _tk_slp_tsk_u( TMO_U tmout_u );
IMPORT INT _tk_wai_tev_u( INT waiptn, TMO_U tmout_u );
IMPORT ER _tk_set_tim_u( SYSTIM_U tim_u );
IMPORT ER _tk_get_tim_u( SYSTIM_U *tim_u, UINT *ofs );
IMPORT ER _tk_get_otm_u( SYSTIM_U *tim_u, UINT *ofs );
IMPORT ER _tk_dly_tsk_u( RELTIM_U dlytim_u );

#ifdef NUM_SEMID
IMPORT ID _tk_cre_sem( CONST T_CSEM *pk_csem );
IMPORT ER _tk_del_sem( ID semid );
IMPORT ER _tk_sig_sem( ID semid, INT cnt );
IMPORT ER _tk_wai_sem( ID semid, INT cnt, TMO tmout );
IMPORT ER _tk_ref_sem( ID semid, T_RSEM *pk_rsem );
IMPORT ER _tk_wai_sem_u( ID semid, INT cnt, TMO_U tmout_u );

IMPORT INT _td_lst_sem( ID list[], INT nent );
IMPORT ER  _td_ref_sem( ID semid, TD_RSEM *rsem );
IMPORT INT _td_sem_que( ID semid, ID list[], INT nent );
#endif

#ifdef NUM_MTXID
IMPORT ID _tk_cre_mtx( CONST T_CMTX *pk_cmtx );
IMPORT ER _tk_del_mtx( ID mtxid );
IMPORT ER _tk_loc_mtx( ID mtxid, TMO tmout );
IMPORT ER _tk_unl_mtx( ID mtxid );
IMPORT ER _tk_ref_mtx( ID mtxid, T_RMTX *pk_rmtx );
IMPORT ER _tk_loc_mtx_u( ID mtxid, TMO_U tmout_u );

IMPORT INT _td_lst_mtx( ID list[], INT nent );
IMPORT ER  _td_ref_mtx( ID mtxid, TD_RMTX *rmtx );
IMPORT INT _td_mtx_que( ID mtxid, ID list[], INT nent );
#endif

#ifdef NUM_FLGID
IMPORT ID _tk_cre_flg( CONST T_CFLG *pk_cflg );
IMPORT ER _tk_del_flg( ID flgid );
IMPORT ER _tk_set_flg( ID flgid, UINT setptn );
IMPORT ER _tk_clr_flg( ID flgid, UINT clrptn );
IMPORT ER _tk_wai_flg( ID flgid, UINT waiptn, UINT wfmode, UINT *p_flgptn, TMO tmout );
IMPORT ER _tk_ref_flg( ID flgid, T_RFLG *pk_rflg );
IMPORT ER _tk_wai_flg_u( ID flgid, UINT waiptn, UINT wfmode, UINT *p_flgptn, TMO_U tmout_u );

IMPORT INT _td_lst_flg( ID list[], INT nent );
IMPORT ER  _td_ref_flg( ID flgid, TD_RFLG *rflg );
IMPORT INT _td_flg_que( ID flgid, ID list[], INT nent );
#endif

#ifdef NUM_MBXID
IMPORT ID _tk_cre_mbx( CONST T_CMBX* pk_cmbx );
IMPORT ER _tk_del_mbx( ID mbxid );
IMPORT ER _tk_snd_mbx( ID mbxid, T_MSG *pk_msg );
IMPORT ER _tk_rcv_mbx( ID mbxid, T_MSG **ppk_msg, TMO tmout );
IMPORT ER _tk_ref_mbx( ID mbxid, T_RMBX *pk_rmbx );
IMPORT ER _tk_rcv_mbx_u( ID mbxid, T_MSG **ppk_msg, TMO_U tmout_u );

IMPORT INT _td_lst_mbx( ID list[], INT nent );
IMPORT ER  _td_ref_mbx( ID mbxid, TD_RMBX *rmbx );
IMPORT INT _td_mbx_que( ID mbxid, ID list[], INT nent );
#endif

#ifdef NUM_MBFID
IMPORT ID _tk_cre_mbf( CONST T_CMBF *pk_cmbf );
IMPORT ER _tk_del_mbf( ID mbfid );
IMPORT ER _tk_snd_mbf( ID mbfid, CONST void *msg, INT msgsz, TMO tmout );
IMPORT INT _tk_rcv_mbf( ID mbfid, void *msg, TMO tmout );
IMPORT ER _tk_ref_mbf( ID mbfid, T_RMBF *pk_rmbf );
IMPORT ER _tk_snd_mbf_u( ID mbfid, CONST void *msg, INT msgsz, TMO_U tmout_u );
IMPORT INT _tk_rcv_mbf_u( ID mbfid, void *msg, TMO_U tmout_u );

IMPORT INT _td_lst_mbf( ID list[], INT nent );
IMPORT ER  _td_ref_mbf( ID mbfid, TD_RMBF *rmbf );
IMPORT INT _td_smbf_que( ID mbfid, ID list[], INT nent );
IMPORT INT _td_rmbf_que( ID mbfid, ID list[], INT nent );
#endif

#ifdef NUM_PORID
IMPORT ID _tk_cre_por( CONST T_CPOR *pk_cpor );
IMPORT ER _tk_del_por( ID porid );
IMPORT INT _tk_cal_por( ID porid, UINT calptn, void *msg, INT cmsgsz, TMO tmout );
IMPORT INT _tk_acp_por( ID porid, UINT acpptn, RNO *p_rdvno, void *msg, TMO tmout );
IMPORT ER _tk_fwd_por( ID porid, UINT calptn, RNO rdvno, void *msg, INT cmsgsz );
IMPORT ER _tk_rpl_rdv( RNO rdvno, void *msg, INT rmsgsz );
IMPORT ER _tk_ref_por( ID porid, T_RPOR *pk_rpor );
IMPORT INT _tk_cal_por_u( ID porid, UINT calptn, void *msg, INT cmsgsz, TMO_U tmout_u );
IMPORT INT _tk_acp_por_u( ID porid, UINT acpptn, RNO *p_rdvno, void *msg, TMO_U tmout_u );

IMPORT INT _td_lst_por( ID list[], INT nent );
IMPORT ER  _td_ref_por( ID porid, TD_RPOR *rpor );
IMPORT INT _td_cal_que( ID porid, ID list[], INT nent );
IMPORT INT _td_acp_que( ID porid, ID list[], INT nent );
#endif

IMPORT ER _tk_def_int P2( UINT dintno, CONST T_DINT *pk_dint );
IMPORT void _tk_ret_int( void );

#ifdef NUM_MPLID
IMPORT ID _tk_cre_mpl( CONST T_CMPL *pk_cmpl );
IMPORT ER _tk_del_mpl( ID mplid );
IMPORT ER _tk_get_mpl( ID mplid, INT blksz, void **p_blk, TMO tmout );
IMPORT ER _tk_rel_mpl( ID mplid, void *blk );
IMPORT ER _tk_ref_mpl( ID mplid, T_RMPL *pk_rmpl );
IMPORT ER _tk_get_mpl_u( ID mplid, INT blksz, void **p_blk, TMO_U tmout_u );

IMPORT INT _td_lst_mpl( ID list[], INT nent );
IMPORT ER  _td_ref_mpl( ID mplid, TD_RMPL *rmpl );
IMPORT INT _td_mpl_que( ID mplid, ID list[], INT nent );
#endif

#ifdef NUM_MPFID
IMPORT ID _tk_cre_mpf( CONST T_CMPF *pk_cmpf );
IMPORT ER _tk_del_mpf( ID mpfid );
IMPORT ER _tk_get_mpf( ID mpfid, void **p_blf, TMO tmout );
IMPORT ER _tk_rel_mpf( ID mpfid, void *blf );
IMPORT ER _tk_ref_mpf( ID mpfid, T_RMPF *pk_rmpf );
IMPORT ER _tk_get_mpf_u( ID mpfid, void **p_blf, TMO_U tmout_u );

IMPORT INT _td_lst_mpf( ID list[], INT nent );
IMPORT ER  _td_ref_mpf( ID mpfid, TD_RMPF *rmpf );
IMPORT INT _td_mpf_que( ID mpfid, ID list[], INT nent );
#endif

IMPORT ER _tk_set_tim( CONST SYSTIM *pk_tim );
IMPORT ER _tk_get_tim( SYSTIM *pk_tim );
IMPORT ER _tk_get_otm( SYSTIM *pk_tim );
IMPORT ER _tk_dly_tsk( RELTIM dlytim );

#ifdef NUM_CYCID
IMPORT ID _tk_cre_cyc P1( CONST T_CCYC *pk_ccyc );
IMPORT ER _tk_del_cyc( ID cycid );
IMPORT ER _tk_sta_cyc( ID cycid );
IMPORT ER _tk_stp_cyc( ID cycid );
IMPORT ER _tk_ref_cyc( ID cycid, T_RCYC *pk_rcyc );
IMPORT ID _tk_cre_cyc_u( CONST T_CCYC_U *pk_ccyc_u );
IMPORT ER _tk_ref_cyc_u( ID cycid, T_RCYC_U *pk_rcyc_u );

IMPORT INT _td_lst_cyc( ID list[], INT nent );
IMPORT ER  _td_ref_cyc( ID cycid, TD_RCYC *rcyc );
IMPORT ER  _td_ref_cyc_u( ID cycid, TD_RCYC_U *rcyc_u );
#endif

#ifdef NUM_ALMID
IMPORT ID _tk_cre_alm P1( CONST T_CALM *pk_calm );
IMPORT ER _tk_del_alm( ID almid );
IMPORT ER _tk_sta_alm( ID almid, RELTIM almtim );
IMPORT ER _tk_stp_alm( ID almid );
IMPORT ER _tk_ref_alm( ID almid, T_RALM *pk_ralm );
IMPORT ER _tk_sta_alm_u( ID almid, RELTIM_U almtim_u );
IMPORT ER _tk_ref_alm_u( ID almid, T_RALM_U *pk_ralm_u );

IMPORT INT _td_lst_alm( ID list[], INT nent );
IMPORT ER  _td_ref_alm( ID almid, TD_RALM *ralm );
IMPORT ER  _td_ref_alm_u( ID almid, TD_RALM_U *ralm_u );
#endif

IMPORT ER _tk_ref_ver( T_RVER *pk_rver );
IMPORT ER _tk_ref_sys( T_RSYS *pk_rsys );
IMPORT ER _tk_def_ssy P2( ID ssid, CONST T_DSSY *pk_dssy );
IMPORT ER _tk_sta_ssy( ID ssid, ID resid, INT info );
IMPORT ER _tk_cln_ssy( ID ssid, ID resid, INT info );
IMPORT ER _tk_evt_ssy( ID ssid, INT evttyp, ID resid, INT info );
IMPORT ER _tk_ref_ssy( ID ssid, T_RSSY *pk_rssy );
IMPORT ID _tk_cre_res( void );
IMPORT ER _tk_del_res( ID resid );
IMPORT ER _tk_get_res( ID resid, ID ssid, void **p_resblk );
IMPORT ER _tk_set_pow( UINT powmode );

/* T-Kernel/DS */
IMPORT INT _td_lst_tsk( ID list[], INT nent );
IMPORT INT _td_lst_ssy( ID list[], INT nent );
IMPORT ER  _td_ref_ssy( ID ssid, TD_RSSY *rssy );
IMPORT ER  _td_ref_tsk( ID tskid, TD_RTSK *rtsk );
IMPORT ER  _td_ref_tex( ID tskid, TD_RTEX *rtex );
IMPORT ER  _td_inf_tsk( ID tskid, TD_ITSK *itsk, BOOL clr );
IMPORT ER  _td_get_reg( ID tskid, T_REGS *regs, T_EIT *eit, T_CREGS *cregs );
IMPORT ER  _td_set_reg( ID tskid, CONST T_REGS *regs, CONST T_EIT *eit, CONST T_CREGS *cregs );
IMPORT ER  _td_ref_sys( TD_RSYS *rsys );
IMPORT ER  _td_get_tim( SYSTIM *tim, UINT *ofs );
IMPORT ER  _td_get_otm( SYSTIM *tim, UINT *ofs );
IMPORT INT _td_rdy_que( PRI pri, ID list[], INT nent );
IMPORT ER  _td_hok_svc( CONST TD_HSVC *hsvc );
IMPORT ER  _td_hok_dsp( CONST TD_HDSP *hdsp );
IMPORT ER  _td_hok_int( CONST TD_HINT *hint );
IMPORT ER  _td_ref_dsname( UINT type, ID id, UB *dsname );
IMPORT ER  _td_set_dsname( UINT type, ID id, CONST UB *dsname );
IMPORT ER  _td_ref_tsk_u( ID tskid, TD_RTSK_U *rtsk_u );
IMPORT ER  _td_inf_tsk_u( ID tskid, TD_ITSK_U *itsk_u, BOOL clr );
IMPORT ER  _td_get_tim_u( SYSTIM_U *tim_u, UINT *ofs );
IMPORT ER  _td_get_otm_u( SYSTIM_U *tim_u, UINT *ofs );

/* T-Kernel/SM */
IMPORT INT _tk_get_cfn( CONST UB *name, INT *val, INT max );
IMPORT INT _tk_get_cfs( CONST UB *name, UB *buf, INT max );

#ifdef	_BTRON_
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_bk_slp_tsk
 Input		:struct task *task
 		 < task to sleep >
 Output		:void
 Return		:ER
 		 < result >
 Description	:move a task state to wait state for btron
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER _bk_slp_tsk(struct task *task);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_bk_wup_tsk
 Input		:struct task *task
 		 < task to wake up >
 Output		:void
 Return		:ER
 		 < result >
 Description	:wake up a task for btron
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER _bk_wup_tsk(struct task *task);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_bk_sus_tsk
 Input		:struct task *task
 		 < task to suspend >
 Output		:void
 Return		:ER
 		 < result >
 Description	:suspend a task for btron
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER _bk_sus_tsk(struct task *task);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_bk_rsm_tsk
 Input		:struct task *task
 		 < task to resume from suspend >
 Output		:void
 Return		:ER
 		 < result >
 Description	:resume a task for btron
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT ER _bk_rsm_tsk(struct task *task);

#endif	// _BTRON_

#endif /* _ISYSCALL_ */
