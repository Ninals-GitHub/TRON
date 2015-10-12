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
 *	winfo.h (T-Kernel/OS)
 *	Definition of Wait Information for Synchronization/Communication Object
 */

#ifndef _WINFO_
#define _WINFO_

/*
 * Semaphore wait (TTW_SEM)
 */
typedef struct {
	INT	cnt;		/* Request resource number */
} WINFO_SEM;

/*
 * Event flag wait (TTW_FLG)
 */
typedef struct {
	UINT	waiptn;		/* Wait bit pattern */
	UINT	wfmode;		/* Wait mode */
	UINT	*p_flgptn;	/* Address that has a bit pattern
				   at wait released */
} WINFO_FLG;

/*
 * Mailbox wait (TTW_MBX)
 */
typedef struct {
	T_MSG	**ppk_msg;	/* Address that has the head of a
				   message packet */
} WINFO_MBX;

/*
 * Message buffer receive/send wait (TTW_RMBF, TTW_SMBF)
 */
typedef struct {
	void	*msg;		/* Address that has a received message */
	INT	*p_msgsz;	/* Address that has a received message size */
} WINFO_RMBF;

typedef struct {
	CONST void *msg;	/* Send message head address */
	INT	msgsz;		/* Send message size */
} WINFO_SMBF;

/*
 * Rendezvous call/accept/end wait (TTW_CAL, TTW_ACP, TTW_RDV)
 */
typedef struct {
	UINT	calptn;		/* Bit pattern that indicates caller
				   select condition */
	void	*msg;		/* Address that has a message */
	INT	cmsgsz;		/* Call message size */
	INT	*p_rmsgsz;	/* Address that has a reply message size */
} WINFO_CAL;

typedef struct {
	UINT	acpptn;		/* Bit pattern that indicates receiver
				   select condition */
	void	*msg;		/* Address that has a call message */
	RNO	*p_rdvno;	/* Address that has the rendezvous number */
	INT	*p_cmsgsz;	/* Address that has the call message size */
} WINFO_ACP;

typedef struct {
	RNO	rdvno;		/* Rendezvous number */
	void	*msg;		/* Address that has a message */
	INT	maxrmsz;	/* Maximum length of reply message */
	INT	*p_rmsgsz;	/* Address that has a reply message size */
} WINFO_RDV;

/*
 * Variable size memory pool wait (TTW_MPL)
 */
typedef struct {
	INT	blksz;		/* Memory block size */
	void	**p_blk;	/* Address that has the head of a
				   memory block */
} WINFO_MPL;

/*
 * Fixed size memory pool wait (TTW_MPF)
 */
typedef struct {
	void	**p_blf;	/* Address that has the head of a
				   memory block */
} WINFO_MPF;

/*
 * Definition of wait information in task control block
 */
typedef union {
#ifdef NUM_SEMID
	WINFO_SEM	sem;
#endif
#ifdef NUM_FLGID
	WINFO_FLG	flg;
#endif
#ifdef NUM_MBXID
	WINFO_MBX	mbx;
#endif
#ifdef NUM_MBFID
	WINFO_RMBF	rmbf;
	WINFO_SMBF	smbf;
#endif
#ifdef NUM_PORID
	WINFO_CAL	cal;
	WINFO_ACP	acp;
	WINFO_RDV	rdv;
#endif
#ifdef NUM_MPLID
	WINFO_MPL	mpl;
#endif
#ifdef NUM_MPFID
	WINFO_MPF	mpf;
#endif
} WINFO;

/*
 * Definition of wait specification structure
 */
typedef struct {
	UINT	tskwait;			/* Wait factor */
	void	(*chg_pri_hook)(TCB *, INT);	/* Process at task priority
						   change */
	void	(*rel_wai_hook)(TCB *);		/* Process at task wait
						   release */
} WSPEC;

#endif /* _WINFO_ */
