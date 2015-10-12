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
        main.C  KB/PD real I/O driver: main
 *
 */
#include	"kbpd.h"
#include	<device/devconf.h>

/* Get "DEVCONF" entry */
IMPORT W GetDevConf( UB *name, W *val );

EXPORT	ID	CmdTsk;			/* command processing task ID */
EXPORT	ID	CmdFlg;			/* event flag ID for sending command */
EXPORT	ID	EvtMbx;			/* MBOX ID for sending event                */
EXPORT	ID	InpMbf;			/* input MBUF ID                    */
 
EXPORT	BOOL	Suspended;		/* suspended                        */

EXPORT	UW	InpMode;		/* input mode                        */
EXPORT	UW	KbdId;			/* keyboard ID              */
EXPORT	W	PdSense;		/* sensitivity (0-15), acceleration(0-7) */

EXPORT	PRI	TaskPri;		/* task priority */
LOCAL	UB	devkbpd[] = "kbpd";	/* KB/PD device name  */
 
/*
        create & start task
*/
EXPORT	INT	kpCreTask(W name, FP entry)
{
	T_CTSK	ctsk;
	ID	tskid;
	INT	er;
	static int id_num = 0;

        /* task creation */
	ctsk.exinf = (void*)name;
	ctsk.task = entry;
	ctsk.itskpri = TaskPri;
	ctsk.stksz = TASK_STKSZ;
	ctsk.tskatr = TA_HLNG | TA_RNG0;
	ctsk.dsname[0] = 'l';
	ctsk.dsname[1] = 'w';
	ctsk.dsname[2] = 0x30 + id_num++;
	ctsk.dsname[3] = '\0';

	tskid = er = tk_cre_tsk(&ctsk);
	if (er >= E_OK) {	/* start task */
		er = tk_sta_tsk(tskid, 0);
		if (er < E_OK) tk_del_tsk(tskid);
	}
	return (er >= E_OK) ? tskid : er;
}
/*
        initial setting at driver start up time
*/
LOCAL	ER	kpStartUp(void)
{
	W	w[L_DEVCONF_VAL];
	W	dd, n;
	ER	er;
	RawEvt	evt;
	T_CMBF	cmbf;
	T_CFLG	cflg;
	ID	datatsk;
	void*	name;
	union {
		FlgInStat	stat;
		UW		uw;
	} u;

        /* extract ID of the mailbox for event notification to KB/PD driver */
	dd = er = tk_opn_dev(devkbpd, TD_READ);
	if (er >= E_OK) {
		er = tk_srea_dev(dd, DN_KPINPUT, (VB*)&EvtMbx,
						sizeof(EvtMbx), &n);
		tk_cls_dev(dd, 0);
	}
	if (er < E_OK) goto EEXIT1;

        /* KBID is extracted from DEVCONF parameter */
	KbdId = (GetDevConf("KBTYPE", w) == 1) ? w[0] : KID_IBM_JP;

        /* input message buffer creation */
	SetOBJNAME(cmbf.exinf, "lkbM");
	cmbf.mbfatr = TA_TFIFO;
	cmbf.bufsz  = sizeof(InMsg) * MAX_INMSG;
	cmbf.maxmsz = sizeof(InMsg);
	if ((er = tk_cre_mbf(&cmbf)) < E_OK) goto EEXIT1;
	InpMbf = er;

        /* creating the event flag for command */
	SetOBJNAME(cflg.exinf, "lkbC");
	cflg.flgatr  = TA_WMUL;
	cflg.iflgptn = 0;
	if ((er = tk_cre_flg(&cflg)) < E_OK) goto EEXIT2;
	CmdFlg = (ID)er;

        /* create and start data processing task */
	SetOBJNAME(name, "lkbD");
	er = kpCreTask((W)name, kpDataTask);
	if (er < E_OK) goto EEXIT3;
	datatsk = (ID)er;

        /* create and start command processing task */
	SetOBJNAME(name, "lkbC");
	er = kpCreTask((W)name, kpCmdTask);
	if (er < E_OK) goto EEXIT4;
	CmdTsk = (ID)er;

        /* registering event flag for commands */
	u.uw = 0;
	evt.f.stat = u.stat;
	evt.f.stat.cmd = INP_FLG;
	evt.f.stat.kb = 1;
	evt.f.stat.kbid = KbdId;
	evt.f.stat.reg = 1;
	evt.f.flgid = CmdFlg;
	if ((er = kpSendMsg(&evt)) < E_OK) goto EEXIT5;

        /* device initialization processing */
	er = hwInit(DC_OPEN);
	if (er < E_OK) goto EEXIT5;

	return E_OK;

EEXIT5:
	tk_ter_tsk(CmdTsk);
	tk_del_tsk(CmdTsk);
EEXIT4:
	tk_ter_tsk(datatsk);
	tk_del_tsk(datatsk);
EEXIT3:
	tk_del_flg(CmdFlg);
EEXIT2:
	tk_del_mbf(InpMbf);
EEXIT1:
	DP(("kpStartUp: err=%#x\n", er));
	return er;
}
/*
        KBPD keyboard/ PD real I/O driver entry
*/
EXPORT	ER	LowKbPdDrv(INT ac, UB *av[])
{
	char	*arg;
	W	v[L_DEVCONF_VAL];

        /* effective? */
	if (GetDevConf("LowKbPdDrvEnable", v) > 0 && !v[0]) return E_NOSPT;

	if (ac < 0) return E_OK;	/* no special epilog processor event */

        /* obtain start parameter */
	TaskPri = DEF_PRIORITY;
	if (ac > 1 && (arg = av[1]) != NULL) {
		switch (*arg++) {
		case '!':	/* priority */
			TaskPri = strtol(arg, &arg, 0);
			break;
		default:
			return E_PAR;
		}
	}

        /* driver initialization */
	return kpStartUp();
}
