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
 *	@(#)syscall.h (T-Kernel)
 *
 *	T-Kernel/OS (Common parts)
 */

#ifndef __TK_SYSCALL_H__
#define __TK_SYSCALL_H__

#include <basic.h>
#include <tk/typedef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Task creation */
#define TSK_SELF	0		/* Its own task specify */
#define TPRI_INI	0		/* Specify priority at task startup */
#define TPRI_RUN	0		/* Specify highest priority during
					   running */

#define TA_ASM		0x00000000U	/* Program by assembler */
#define TA_HLNG		0x00000001U	/* Program by high level programming
					   language */
#define TA_SSTKSZ	0x00000002U	/* Set system stack size */
#define TA_USERSTACK	0x00000004U	/* Set user stack size */
#define TA_TASKSPACE	0x00000008U	/* Specify task space */
#define TA_RESID	0x00000010U	/* Specify resource group */
#define TA_DSNAME	0x00000040U	/* Use object name */

#define TA_RNG0		0x00000000U	/* Execute by protection level 0 */
#define TA_RNG1		0x00000100U	/* Execute by protection level 1 */
#define TA_RNG2		0x00000200U	/* Execute by protection level 2 */
#define TA_RNG3		0x00000300U	/* Execute by protection level 3 */

#define TA_COP0		0x00001000U	/* Use coprocessor (ID=0) */
#define TA_COP1		0x00002000U	/* Use coprocessor (ID=1) */
#define TA_COP2		0x00004000U	/* Use coprocessor (ID=2) */
#define TA_COP3		0x00008000U	/* Use coprocessor (ID=3) */

/* Task state tskstat */
#define TTS_RUN		0x00000001U	/* RUN */
#define TTS_RDY		0x00000002U	/* READY */
#define TTS_WAI		0x00000004U	/* WAIT */
#define TTS_SUS		0x00000008U	/* SUSPEND */
#define TTS_WAS		0x0000000cU	/* WAIT-SUSPEND */
#define TTS_DMT		0x00000010U	/* DORMANT */
#define TTS_NODISWAI	0x00000080U	/* Wait disable rejection state */

/* Wait factor tskwait */
#define TTW_SLP		0x00000001U	/* Wait caused by wakeup wait */
#define TTW_DLY		0x00000002U	/* Wait caused by task delay */
#define TTW_SEM		0x00000004U	/* Semaphore wait */
#define TTW_FLG		0x00000008U	/* Event flag wait */
#define TTW_MBX		0x00000040U	/* Mail box wait */
#define TTW_MTX		0x00000080U	/* Mutex wait */
#define TTW_SMBF	0x00000100U	/* Message buffer send wait */
#define TTW_RMBF	0x00000200U	/* Message buffer receive wait */
#define TTW_CAL		0x00000400U	/* Rendezvous call wait */
#define TTW_ACP		0x00000800U	/* Rendezvous accept wait */
#define TTW_RDV		0x00001000U	/* Rendezvous end wait */
#define TTW_MPF		0x00002000U	/* Fixed size memory pool wait */
#define TTW_MPL		0x00004000U	/* Variable size memory pool wait */
#define TTW_EV1		0x00010000U	/* Task event # 1 wait */
#define TTW_EV2		0x00020000U	/* Task event # 2 wait */
#define TTW_EV3		0x00040000U	/* Task event # 3 wait */
#define TTW_EV4		0x00080000U	/* Task event # 4 wait */
#define TTW_EV5		0x00100000U	/* Task event # 5 wait */
#define TTW_EV6		0x00200000U	/* Task event # 6 wait */
#define TTW_EV7		0x00400000U	/* Task event # 7 wait */
#define TTW_EV8		0x00800000U	/* Task event # 8 wait */

#define TTX_SVC		0x80000000U	/* Extended SVC call disable
					   (tk_dis_wai) */

/* Semaphore generation */
#define TA_TFIFO	0x00000000U	/* Manage wait task by FIFO */
#define TA_TPRI		0x00000001U	/* Manage wait task by priority
					   order */
#define TA_FIRST	0x00000000U	/* Give priority to task at head of
					   wait queue */
#define TA_CNT		0x00000002U	/* Give priority to task whose
					   request counts is less */
#define TA_DSNAME	0x00000040U	/* Use object name */
#define TA_NODISWAI	0x00000080U	/* Wait disable rejection */

/* Mutex */
#define TA_TFIFO	0x00000000U	/* Manage wait task by FIFO */
#define TA_TPRI		0x00000001U	/* Manage wait task by priority
					   order */
#define TA_INHERIT	0x00000002U	/* Priority inherited protocol */
#define TA_CEILING	0x00000003U	/* Upper limit priority protocol */
#define TA_DSNAME	0x00000040U	/* Use object name */
#define TA_NODISWAI	0x00000080U	/* Wait disable rejection */

/* Event flag */
#define TA_TFIFO	0x00000000U	/* Manage wait task by FIFO */
#define TA_TPRI		0x00000001U	/* Manage wait task by priority
					   order */
#define TA_WSGL		0x00000000U	/* Disable multiple tasks wait */
#define TA_WMUL		0x00000008U	/* Enable multiple tasks wait */
#define TA_DSNAME	0x00000040U	/* Use object name */
#define TA_NODISWAI	0x00000080U	/* Wait disable rejection */

/* Event flag wait mode */
#define TWF_ANDW	0x00000000U	/* AND wait */
#define TWF_ORW		0x00000001U	/* OR wait */
#define TWF_CLR		0x00000010U	/* All clear specify */
#define TWF_BITCLR	0x00000020U	/* Only condition bit clear specify */

/* Mail box */
#define TA_TFIFO	0x00000000U	/* Manage wait task by FIFO */
#define TA_TPRI		0x00000001U	/* Manage wait task by priority
					   order */
#define TA_MFIFO	0x00000000U	/* Manage messages by FIFO */
#define TA_MPRI		0x00000002U	/* Manage messages by priority
					   order */
#define TA_DSNAME	0x00000040U	/* Use object name */
#define TA_NODISWAI	0x00000080U	/* Wait disable rejection */

/* Message buffer */
#define TA_TFIFO	0x00000000U	/* Manage wait task by FIFO */
#define TA_TPRI		0x00000001U	/* Manage wait task by priority
					   order */
#define TA_DSNAME	0x00000040U	/* Use object name */
#define TA_NODISWAI	0x00000080U	/* Wait disable rejection */

/* Rendezvous */
#define TA_TFIFO	0x00000000U	/* Manage wait task by FIFO */
#define TA_TPRI		0x00000001U	/* Manage wait task by priority
					   order */
#define TA_DSNAME	0x00000040U	/* Use object name */
#define TA_NODISWAI	0x00000080U	/* Wait disable rejection */

/* Handler */
#define TA_ASM		0x00000000U	/* Program by assembler */
#define TA_HLNG		0x00000001U	/* Program by high level programming
					   language */

/* Variable size memory pool */
#define TA_TFIFO	0x00000000U	/* Manage wait task by FIFO */
#define TA_TPRI		0x00000001U	/* Manage wait task by priority
					   order */
#define TA_NORESIDENT	0x00000010U	/* Non resident */
#define TA_DSNAME	0x00000040U	/* Use object name */
#define TA_NODISWAI	0x00000080U	/* Wait disable rejection */
#define TA_RNG0		0x00000000U	/* Protection level 0 */
#define TA_RNG1		0x00000100U	/* Protection level 1 */
#define TA_RNG2		0x00000200U	/* Protection level 2 */
#define TA_RNG3		0x00000300U	/* Protection level 3 */

/* Fixed size memory pool */
#define TA_TFIFO	0x00000000U	/* Manage wait task by FIFO */
#define TA_TPRI		0x00000001U	/* Manage wait task by priority
					   order */
#define TA_NORESIDENT	0x00000010U	/* Non-resident */
#define TA_DSNAME	0x00000040U	/* Use object name */
#define TA_NODISWAI	0x00000080U	/* Wait disable rejection */
#define TA_RNG0		0x00000000U	/* Protection level 0 */
#define TA_RNG1		0x00000100U	/* Protection level 1 */
#define TA_RNG2		0x00000200U	/* Protection level 2 */
#define TA_RNG3		0x00000300U	/* Protection level 3 */

/* Cycle handler */
#define TA_ASM		0x00000000U	/* Program by assembler */
#define TA_HLNG		0x00000001U	/* Program by high level programming
					   language */
#define TA_STA		0x00000002U	/* Cycle handler startup */
#define TA_PHS		0x00000004U	/* Save cycle handler phase */
#define TA_DSNAME	0x00000040U	/* Use object name */

#define TCYC_STP	0x00U		/* Cycle handler is not operating */
#define TCYC_STA	0x01U		/* Cycle handler is operating */

/* Alarm handler address */
#define TA_ASM		0x00000000U	/* Program by assembler */
#define TA_HLNG		0x00000001U	/* Program by high level programming
					   language */
#define TA_DSNAME	0x00000040U	/* Use object name */

#define TALM_STP	0x00U		/* Alarm handler is not operating */
#define TALM_STA	0x01U		/* Alarm handler is operating */

/* System state */
#define TSS_TSK		0x00U	/* During execution of task part(context) */
#define TSS_DDSP	0x01U	/* During dispatch disable */
#define TSS_DINT	0x02U	/* During Interrupt disable */
#define TSS_INDP	0x04U	/* During execution of task independent part */
#define TSS_QTSK	0x08U	/* During execution of semi-task part */

/* Power-saving mode */
#define TPW_DOSUSPEND	1	/* Transit to suspend state */
#define TPW_DISLOWPOW	2	/* Power-saving mode switch disable */
#define TPW_ENALOWPOW	3	/* Power-saving mode switch enable */

#ifdef __cplusplus
}
#endif

/* System dependencies */
#include <tk/sysdepend/syscall_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Task creation information 		tk_cre_tsk
 */
typedef struct t_ctsk {
	void	*exinf;		/* Extended information */
	ATR	tskatr;		/* Task attribute */
	FP	task;		/* Task startup address */
	PRI	itskpri;	/* Priority at task startup */
	INT	stksz;		/* User stack size (byte) */
	INT	sstksz;		/* System stack size (byte) */
	void	*stkptr;		/* User stack pointer */
	void	*uatb;		/* Task space page table */
	INT	lsid;		/* Logical space ID */
	ID	resid;		/* Resource ID */
	UB	dsname[8];	/* Object name */
#if TA_GP
	void	*gp;		/* Global pointer (gp) */
#endif
} T_CTSK;

/*
 * Task space information		tk_get_tsp, tk_set_tsp
 */
typedef struct t_tskspc {
	void	*uatb;		/* Address of task space page table */
	INT	lsid;		/* Logical space ID */
} T_TSKSPC;

/*
 * Task statistics information		tk_inf_tsk
 */
typedef struct t_itsk {
	RELTIM	stime;		/* Cumulative system execution time
				   (milliseconds) */
	RELTIM	utime;		/* Cumulative user execution time
				   (milliseconds) */
} T_ITSK;
typedef struct t_itsk_u {
	RELTIM_U stime_u;	/* Cumulative system execution time (us) */
	RELTIM_U utime_u;	/* Cumulative user execution time (us) */
} T_ITSK_U;

/*
 * Task state information 		tk_ref_tsk
 */
typedef	struct t_rtsk {
	void	*exinf;		/* Extended information */
	PRI	tskpri;		/* Current priority */
	PRI	tskbpri;	/* Base priority */
	UINT	tskstat;	/* Task state */
	UINT	tskwait;	/* Wait factor */
	ID	wid;		/* Wait object ID */
	INT	wupcnt;		/* Number of wakeup requests queuing */
	INT	suscnt;		/* Number of SUSPEND request nests */
	RELTIM	slicetime;	/* Maximum continuous execution time
				   (millisecond) */
	UINT	waitmask;	/* Disabled wait factor */
	UINT	texmask;	/* Enabled task exception */
	UINT	tskevent;	/* Occurring task event */
} T_RTSK;
typedef	struct t_rtsk_u {
	void	*exinf;		/* Extended information */
	PRI	tskpri;		/* Current priority */
	PRI	tskbpri;	/* Base priority */
	UINT	tskstat;	/* Task state */
	UINT	tskwait;	/* Wait factor */
	ID	wid;		/* Wait object ID */
	INT	wupcnt;		/* Number of wakeup requests queuing */
	INT	suscnt;		/* Number of SUSPEND request nests */
	RELTIM_U slicetime_u;	/* Maximum continuous execution time (us) */
	UINT	waitmask;	/* Disabled wait factor */
	UINT	texmask;	/* Enabled task exception */
	UINT	tskevent;	/* Occurring task event */
} T_RTSK_U;

/*
 * Task exception handler definition information	tk_def_tex
 */
typedef struct t_dtex {
	ATR	texatr;		/* Task exception handler attribute */
	FP	texhdr;		/* Task exception handler address */
} T_DTEX;

/*
 * Task exception state information		tk_ref_tex
 */
typedef struct t_rtex {
	UINT	pendtex;	/* Occurring task exception */
	UINT	texmask;	/* Enabled task exception */
} T_RTEX;

/*
 * Semaphore creation information		tk_cre_sem
 */
typedef	struct t_csem {
	void	*exinf;		/* Extended information */
	ATR	sematr;		/* Semaphore attribute */
	INT	isemcnt;	/* Semaphore initial count value */
	INT	maxsem;		/* Semaphore maximum count value */
	UB	dsname[8];	/* Object name */
} T_CSEM;

/*
 * Semaphore state information		tk_ref_sem
 */
typedef	struct t_rsem {
	void	*exinf;		/* Extended information */
	ID	wtsk;		/* Wait task ID */
	INT	semcnt;		/* Current semaphore value */
} T_RSEM;

/*
 * Mutex creation information		tk_cre_mtx
 */
typedef struct t_cmtx {
	void	*exinf;		/* Extended information */
	ATR	mtxatr;		/* Mutex attribute */
	PRI	ceilpri;	/* Upper limit priority of mutex */
	UB	dsname[8];	/* Object name */
} T_CMTX;

/*
 * Mutex state information		tk_ref_mtx
 */
typedef struct t_rmtx {
	void	*exinf;		/* Extended information */
	ID	htsk;		/* Locking task ID */
	ID	wtsk;		/* Lock wait task ID */
} T_RMTX;

/*
 * Event flag creation information	tk_cre_flg
 */
typedef	struct t_cflg {
	void	*exinf;		/* Extended information */
	ATR	flgatr;		/* Event flag attribute */
	UINT	iflgptn;	/* Event flag initial value */
	UB	dsname[8];	/* Object name */
} T_CFLG;

/*
 * Event flag state information		tk_ref_flg
 */
typedef	struct t_rflg {
	void	*exinf;		/* Extended information */
	ID	wtsk;		/* Wait task ID */
	UINT	flgptn;		/* Current event flag pattern */
} T_RFLG;

/*
 * Mail box creation information	tk_cre_mbx
 */
typedef	struct t_cmbx {
	void	*exinf;		/* Extended information */
	ATR	mbxatr;		/* Mail box attribute */
	UB	dsname[8];	/* Object name */
} T_CMBX;

/*
 * Mail box message header
 */
typedef struct t_msg {
	void	*msgque[1];	/* Area for message queue */
} T_MSG;

typedef struct t_msg_pri {
	T_MSG	msgque;		/* Area for message queue */
	PRI	msgpri;		/* Message priority */
} T_MSG_PRI;

/*
 * Mail box state information		tk_ref_mbx
 */
typedef	struct t_rmbx {
	void	*exinf;		/* Extended information */
	ID	wtsk;		/* Wait task ID */
	T_MSG	*pk_msg;	/* Next received message */
} T_RMBX;

/*
 * Message buffer creation information	tk_cre_mbf
 */
typedef	struct t_cmbf {
	void	*exinf;		/* Extended information */
	ATR	mbfatr;		/* Message buffer attribute */
	INT	bufsz;		/* Message buffer size (byte) */
	INT	maxmsz;		/* Maximum length of message (byte) */
	UB	dsname[8];	/* Object name */
} T_CMBF;

/*
 * Message buffer state information 	tk_ref_mbf
 */
typedef struct t_rmbf {
	void	*exinf;		/* Extended information */
	ID	wtsk;		/* Receive wait task ID */
	ID	stsk;		/* Send wait task ID */
	INT	msgsz;		/* Next received message size (byte) */
	INT	frbufsz;	/* Free buffer size (byte) */
	INT	maxmsz;		/* Maximum length of message (byte) */
} T_RMBF;

/*
 * Rendezvous port creation information	tk_cre_por
 */
typedef	struct t_cpor {
	void	*exinf;		/* Extended information */
	ATR	poratr;		/* Port attribute */
	INT	maxcmsz;	/* Maximum length of call message (byte) */
	INT	maxrmsz;	/* Maximum length of replay message (byte) */
	UB	dsname[8];	/* Object name */
} T_CPOR;

/*
 * Rendezvous port state information	tk_ref_por
 */
typedef struct t_rpor {
	void	*exinf;		/* Extended information */
	ID	wtsk;		/* Call wait task ID */
	ID	atsk;		/* Receive wait task ID */
	INT	maxcmsz;	/* Maximum length of call message (byte) */
	INT	maxrmsz;	/* Maximum length of replay message (byte) */
} T_RPOR;

/*
 * Interrupt handler definition information	tk_def_int
 */
typedef struct t_dint {
	ATR	intatr;		/* Interrupt handler attribute */
	FP	inthdr;		/* Interrupt handler address */
#if TA_GP
	void	*gp;		/* Global pointer (gp) */
#endif
} T_DINT;

/*
 * Variable size memory pool creation information	tk_cre_mpl
 */
typedef	struct t_cmpl {
	void	*exinf;		/* Extended information */
	ATR	mplatr;		/* Memory pool attribute */
	INT	mplsz;		/* Size of whole memory pool (byte) */
	UB	dsname[8];	/* Object name */
} T_CMPL;

/*
 * Variable size memory pool state information	tk_ref_mpl
 */
typedef struct t_rmpl {
	void	*exinf;		/* Extended information */
	ID	wtsk;		/* Wait task ID */
	INT	frsz;		/* Total size of free area (byte) */
	INT	maxsz;		/* Size of maximum continuous free area
				   (byte) */
} T_RMPL;

/*
 * Fixed size memory pool state information	tk_cre_mpf
 */
typedef	struct t_cmpf {
	void	*exinf;		/* Extended information */
	ATR	mpfatr;		/* Memory pool attribute */
	INT	mpfcnt;		/* Number of blocks in whole memory pool */
	INT	blfsz;		/* Fixed size memory block size (byte) */
	UB	dsname[8];	/* Object name */
} T_CMPF;

/*
 * Fixed size memory pool state information	tk_ref_mpf
 */
typedef struct t_rmpf {
	void	*exinf;		/* Extended information */
	ID	wtsk;		/* Wait task ID */
	INT	frbcnt;		/* Number of free area blocks */
} T_RMPF;

/*
 * Cycle handler creation information 	tk_cre_cyc
 */
typedef struct t_ccyc {
	void	*exinf;		/* Extended information */
	ATR	cycatr;		/* Cycle handler attribute */
	FP	cychdr;		/* Cycle handler address */
	RELTIM	cyctim;		/* Cycle interval */
	RELTIM	cycphs;		/* Cycle phase */
	UB	dsname[8];	/* Object name */
#if TA_GP
	void	*gp;		/* Global pointer (gp) */
#endif
} T_CCYC;
typedef struct t_ccyc_u {
	void	*exinf;		/* Extended information */
	ATR	cycatr;		/* Cycle handler attribute */
	FP	cychdr;		/* Cycle handler address */
	RELTIM_U cyctim_u;	/* Cycle interval */
	RELTIM_U cycphs_u;	/* Cycle phase */
	UB	dsname[8];	/* Object name */
#if TA_GP
	void	*gp;		/* Global pointer (gp) */
#endif
} T_CCYC_U;

/*
 * Cycle handler state information	tk_ref_cyc
 */
typedef struct t_rcyc {
	void	*exinf;		/* Extended information */
	RELTIM	lfttim;		/* Remaining time until next handler startup */
	UINT	cycstat;	/* Cycle handler status */
} T_RCYC;
typedef struct t_rcyc_u {
	void	*exinf;		/* Extended information */
	RELTIM_U lfttim_u;	/* Remaining time until next handler startup */
	UINT	cycstat;	/* Cycle handler status */
} T_RCYC_U;

/*
 * Alarm handler creation information		tk_cre_alm
 */
typedef struct t_calm {
	void	*exinf;		/* Extended information */
	ATR	almatr;		/* Alarm handler attribute */
	FP	almhdr;		/* Alarm handler address */
	UB	dsname[8];	/* Object name */
#if TA_GP
	void	*gp;		/* Global pointer (gp) */
#endif
} T_CALM;

/*
 * Alarm handler state information	tk_ref_alm
 */
typedef struct t_ralm {
	void	*exinf;		/* Extended information */
	RELTIM	lfttim;		/* Remaining time until handler startup */
	UINT	almstat;	/* Alarm handler state */
} T_RALM;
typedef struct t_ralm_u {
	void	*exinf;		/* Extended information */
	RELTIM_U lfttim_u;	/* Remaining time until handler startup */
	UINT	almstat;	/* Alarm handler state */
} T_RALM_U;

/*
 * Version information		tk_ref_ver
 */
typedef struct t_rver {
	UH	maker;		/* OS manufacturer */
	UH	prid;		/* OS identification number */
	UH	spver;		/* Specification version */
	UH	prver;		/* OS product version */
	UH	prno[4];	/* Product number, Product management
				   information */
} T_RVER;

/*
 * System state information		tk_ref_sys
 */
typedef struct t_rsys {
	INT	sysstat;	/* System state */
	ID	runtskid;	/* ID of task in execution state */
	ID	schedtskid;	/* ID of the task that should be the
				   execution state */
} T_RSYS;

/*
 * Subsystem definition information 		tk_def_ssy
 */
typedef struct t_dssy {
	ATR	ssyatr;		/* Subsystem attribute */
	PRI	ssypri;		/* Subsystem priority */
	FP	svchdr;		/* Extended SVC handler address */
	FP	breakfn;	/* Break function address */
	FP	startupfn;	/* Startup function address */
	FP	cleanupfn;	/* Cleanup function address */
	FP	eventfn;	/* Event function address */
	INT	resblksz;	/* Resource management block size (byte) */
#if TA_GP
	void	*gp;		/* Global pointer (gp) */
#endif
} T_DSSY;

/*
 * Subsystem state information		tk_ref_ssy
 */
typedef struct t_rssy {
	PRI	ssypri;		/* Subsystem priority */
	INT	resblksz;	/* Resource management block size (byte) */
} T_RSSY;

/* ------------------------------------------------------------------------ */
/*
 * Definition for interface library automatic generation (mktksvc)
 */
/*** DEFINE_TKSVC ***/

/* [BEGIN SYSCALLS] */
IMPORT ID tk_cre_tsk( CONST T_CTSK *pk_ctsk );
IMPORT ER tk_del_tsk( ID tskid );
IMPORT ER tk_sta_tsk( ID tskid, INT stacd );
IMPORT void tk_ext_tsk( void );
IMPORT void tk_exd_tsk( void );
IMPORT ER tk_ter_tsk( ID tskid );
IMPORT ER tk_dis_dsp( void );
IMPORT ER tk_ena_dsp( void );
IMPORT ER tk_chg_pri( ID tskid, PRI tskpri );
IMPORT ER tk_chg_slt( ID tskid, RELTIM slicetime );
IMPORT ER tk_rot_rdq( PRI tskpri );
IMPORT ER tk_rel_wai( ID tskid );
IMPORT ID tk_get_tid( void );
IMPORT ER tk_get_tsp( ID tskid, T_TSKSPC *pk_tskspc );
IMPORT ER tk_set_tsp( ID tskid, CONST T_TSKSPC *pk_tskspc );
IMPORT ID tk_get_rid( ID tskid );
IMPORT ID tk_set_rid( ID tskid, ID resid );
IMPORT ER tk_get_reg( ID tskid, T_REGS *pk_regs, T_EIT *pk_eit, T_CREGS *pk_cregs );
IMPORT ER tk_set_reg( ID tskid, CONST T_REGS *pk_regs, CONST T_EIT *pk_eit, CONST T_CREGS *pk_cregs );
IMPORT ER tk_get_cpr( ID tskid, INT copno, T_COPREGS *pk_copregs );
IMPORT ER tk_set_cpr( ID tskid, INT copno, CONST T_COPREGS *pk_copregs );
IMPORT ER tk_inf_tsk( ID tskid, T_ITSK *pk_itsk, BOOL clr );
IMPORT ER tk_ref_tsk( ID tskid, T_RTSK *pk_rtsk );
IMPORT ER tk_def_tex( ID tskid, CONST T_DTEX *pk_dtex );
IMPORT ER tk_ena_tex( ID tskid, UINT texptn );
IMPORT ER tk_dis_tex( ID tskid, UINT texptn );
IMPORT ER tk_ras_tex( ID tskid, INT texcd );
IMPORT INT tk_end_tex( BOOL enatex );
IMPORT ER tk_ref_tex( ID tskid, T_RTEX *pk_rtex );
IMPORT ER tk_sus_tsk( ID tskid );
IMPORT ER tk_rsm_tsk( ID tskid );
IMPORT ER tk_frsm_tsk( ID tskid );
IMPORT ER tk_slp_tsk( TMO tmout );
IMPORT ER tk_wup_tsk( ID tskid );
IMPORT INT tk_can_wup( ID tskid );
IMPORT ER tk_sig_tev( ID tskid, INT tskevt );
IMPORT INT tk_wai_tev( INT waiptn, TMO tmout );
IMPORT INT tk_dis_wai( ID tskid, UINT waitmask );
IMPORT ER tk_ena_wai( ID tskid );
IMPORT ID tk_cre_sem( CONST T_CSEM *pk_csem );
IMPORT ER tk_del_sem( ID semid );
IMPORT ER tk_sig_sem( ID semid, INT cnt );
IMPORT ER tk_wai_sem( ID semid, INT cnt, TMO tmout );
IMPORT ER tk_ref_sem( ID semid, T_RSEM *pk_rsem );
IMPORT ID tk_cre_mtx( CONST T_CMTX *pk_cmtx );
IMPORT ER tk_del_mtx( ID mtxid );
IMPORT ER tk_loc_mtx( ID mtxid, TMO tmout );
IMPORT ER tk_unl_mtx( ID mtxid );
IMPORT ER tk_ref_mtx( ID mtxid, T_RMTX *pk_rmtx );
IMPORT ID tk_cre_flg( CONST T_CFLG *pk_cflg );
IMPORT ER tk_del_flg( ID flgid );
IMPORT ER tk_set_flg( ID flgid, UINT setptn );
IMPORT ER tk_clr_flg( ID flgid, UINT clrptn );
IMPORT ER tk_wai_flg( ID flgid, UINT waiptn, UINT wfmode, UINT *p_flgptn, TMO tmout );
IMPORT ER tk_ref_flg( ID flgid, T_RFLG *pk_rflg );
IMPORT ID tk_cre_mbx( CONST T_CMBX* pk_cmbx );
IMPORT ER tk_del_mbx( ID mbxid );
IMPORT ER tk_snd_mbx( ID mbxid, T_MSG *pk_msg );
IMPORT ER tk_rcv_mbx( ID mbxid, T_MSG **ppk_msg, TMO tmout );
IMPORT ER tk_ref_mbx( ID mbxid, T_RMBX *pk_rmbx );
IMPORT ID tk_cre_mbf( CONST T_CMBF *pk_cmbf );
IMPORT ER tk_del_mbf( ID mbfid );
IMPORT ER tk_snd_mbf( ID mbfid, CONST void *msg, INT msgsz, TMO tmout );
IMPORT INT tk_rcv_mbf( ID mbfid, void *msg, TMO tmout );
IMPORT ER tk_ref_mbf( ID mbfid, T_RMBF *pk_rmbf );
IMPORT ID tk_cre_por( CONST T_CPOR *pk_cpor );
IMPORT ER tk_del_por( ID porid );
IMPORT INT tk_cal_por( ID porid, UINT calptn, void *msg, INT cmsgsz, TMO tmout );
IMPORT INT tk_acp_por( ID porid, UINT acpptn, RNO *p_rdvno, void *msg, TMO tmout );
IMPORT ER tk_fwd_por( ID porid, UINT calptn, RNO rdvno, void *msg, INT cmsgsz );
IMPORT ER tk_rpl_rdv( RNO rdvno, void *msg, INT rmsgsz );
IMPORT ER tk_ref_por( ID porid, T_RPOR *pk_rpor );
IMPORT ER tk_def_int( UINT dintno, CONST T_DINT *pk_dint );
IMPORT void tk_ret_int( void );
IMPORT ID tk_cre_mpl( CONST T_CMPL *pk_cmpl );
IMPORT ER tk_del_mpl( ID mplid );
IMPORT ER tk_get_mpl( ID mplid, INT blksz, void **p_blk, TMO tmout );
IMPORT ER tk_rel_mpl( ID mplid, void *blk );
IMPORT ER tk_ref_mpl( ID mplid, T_RMPL *pk_rmpl );
IMPORT ID tk_cre_mpf( CONST T_CMPF *pk_cmpf );
IMPORT ER tk_del_mpf( ID mpfid );
IMPORT ER tk_get_mpf( ID mpfid, void **p_blf, TMO tmout );
IMPORT ER tk_rel_mpf( ID mpfid, void *blf );
IMPORT ER tk_ref_mpf( ID mpfid, T_RMPF *pk_rmpf );
IMPORT ER tk_set_tim( CONST SYSTIM *pk_tim );
IMPORT ER tk_get_tim( SYSTIM *pk_tim );
IMPORT ER tk_get_otm( SYSTIM *pk_tim );
IMPORT ER tk_dly_tsk( RELTIM dlytim );
IMPORT ID tk_cre_cyc( CONST T_CCYC *pk_ccyc );
IMPORT ER tk_del_cyc( ID cycid );
IMPORT ER tk_sta_cyc( ID cycid );
IMPORT ER tk_stp_cyc( ID cycid );
IMPORT ER tk_ref_cyc( ID cycid, T_RCYC *pk_rcyc );
IMPORT ID tk_cre_alm( CONST T_CALM *pk_calm );
IMPORT ER tk_del_alm( ID almid );
IMPORT ER tk_sta_alm( ID almid, RELTIM almtim );
IMPORT ER tk_stp_alm( ID almid );
IMPORT ER tk_ref_alm( ID almid, T_RALM *pk_ralm );
IMPORT ER tk_ref_ver( T_RVER *pk_rver );
IMPORT ER tk_ref_sys( T_RSYS *pk_rsys );
IMPORT ER tk_def_ssy( ID ssid, CONST T_DSSY *pk_dssy );
IMPORT ER tk_sta_ssy( ID ssid, ID resid, INT info );
IMPORT ER tk_cln_ssy( ID ssid, ID resid, INT info );
IMPORT ER tk_evt_ssy( ID ssid, INT evttyp, ID resid, INT info );
IMPORT ER tk_ref_ssy( ID ssid, T_RSSY *pk_rssy );
IMPORT ID tk_cre_res( void );
IMPORT ER tk_del_res( ID resid );
IMPORT ER tk_get_res( ID resid, ID ssid, void **p_resblk );
IMPORT ER tk_set_pow( UINT powmode );

/* T-Kernel 2.0 */
IMPORT ER tk_chg_slt_u( ID tskid, RELTIM_U slicetime_u );
IMPORT ER tk_inf_tsk_u( ID tskid, T_ITSK_U *pk_itsk_u, BOOL clr );
IMPORT ER tk_ref_tsk_u( ID tskid, T_RTSK_U *pk_rtsk_u );
IMPORT ER tk_slp_tsk_u( TMO_U tmout_u );
IMPORT INT tk_wai_tev_u( INT waiptn, TMO_U tmout_u );
IMPORT ER tk_dly_tsk_u( RELTIM_U dlytim_u );
IMPORT ER tk_wai_sem_u( ID semid, INT cnt, TMO_U tmout_u );
IMPORT ER tk_wai_flg_u( ID flgid, UINT waiptn, UINT wfmode, UINT *p_flgptn, TMO_U tmout_u );
IMPORT ER tk_rcv_mbx_u( ID mbxid, T_MSG **ppk_msg, TMO_U tmout_u );
IMPORT ER tk_loc_mtx_u( ID mtxid, TMO_U tmout_u );
IMPORT ER tk_snd_mbf_u( ID mbfid, CONST void *msg, INT msgsz, TMO_U tmout_u );
IMPORT INT tk_rcv_mbf_u( ID mbfid, void *msg, TMO_U tmout_u );
IMPORT INT tk_cal_por_u( ID porid, UINT calptn, void *msg, INT cmsgsz, TMO_U tmout_u );
IMPORT INT tk_acp_por_u( ID porid, UINT acpptn, RNO *p_rdvno, void *msg, TMO_U tmout_u );
IMPORT ER tk_get_mpl_u( ID mplid, INT blksz, void **p_blk, TMO_U tmout_u );
IMPORT ER tk_get_mpf_u( ID mpfid, void **p_blf, TMO_U tmout_u );
IMPORT ER tk_set_tim_u( SYSTIM_U tim_u );
IMPORT ER tk_get_tim_u( SYSTIM_U *tim_u, UINT *ofs );
IMPORT ER tk_get_otm_u( SYSTIM_U *tim_u, UINT *ofs );
IMPORT ID tk_cre_cyc_u( CONST T_CCYC_U *pk_ccyc_u );
IMPORT ER tk_ref_cyc_u( ID cycid, T_RCYC_U *pk_rcyc_u );
IMPORT ER tk_sta_alm_u( ID almid, RELTIM_U almtim_u );
IMPORT ER tk_ref_alm_u( ID almid, T_RALM_U *pk_ralm_u );

/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif

#endif /* __TK_SYSCALL_H__ */
