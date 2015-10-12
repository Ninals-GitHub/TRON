/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2014/07/31.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	@(#)ref_command.c
 *
 */

#include <tk/dbgspt.h>
#include <sys/misc.h>

/*
	display exinf
*/
LOCAL	void	dsp_exinf(VP exinf)
{
	INT	i;
	UB	*p = (UB*)&exinf;

	if (exinf != NULL) {
		P("%08x", exinf);
		for (i = 0; i < 4; i++) {
			if (p[i] != 0 && (p[i] < 0x20 || p[i] > 0x7E)) break;
		}
		if (i >= 4) P(" %.4s", p);
	}
	P("\n");
}
/*
	dump system memory status
*/
LOCAL	void	dump_sysmem(void)
{
	T_RSMB	ms;
	W	ratio;
	ER	er;

	er = tk_ref_smb(&ms);
	if (er < E_OK) {
		P("ERR [%#x]\n", er);
		return;
	}
	P("MEMORY: ");
	if (ms.total <= 0 || ms.free < 0) {
		P("unknown\n");
		return;
	}
	ratio = 100 - (ms.free * 100 / ms.total);
	P("Blksz=%d Total=%d (%d KB) Free=%d (%d KB) [%d %% used]\n",
		ms.blksz, ms.total, ms.total * (ms.blksz / 1024),
		ms.free, ms.free * (ms.blksz / 1024), ratio);
}
/*
	dump task status
*/
LOCAL	void	dump_task(ID tskid)
{
	T_RTSK	ref;
	T_ITSK	inf;
	ID	id, maxid;
	ER	er;
	char	*p;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxTskId", &maxid, 1);
	P("TSK STATE (MAX:%d)\n", maxid);

	if (tskid > 0)	maxid = tskid;
	else		tskid = 1;

	P("TID PRI:BPR SLT WUP SUS STS(*:NODISWAI)  ST+UT(x10) RID EXINF\n");

	for (id = tskid; id <= maxid; id++) {
		er = tk_ref_tsk(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}

		/*   TID PRI:BPR SLT WUP SUS */
		P("%3d %3d:%3d %3d %3d %3d ",
			id, ref.tskpri, ref.tskbpri, ref.slicetime,
			ref.wupcnt, ref.suscnt);

		/* STS */
		switch (ref.tskstat & ~TTS_NODISWAI) {
		  case TTS_RUN:	p = "RUN"; break;
		  case TTS_RDY:	p = "RDY"; break;
		  case TTS_WAI:	p = "WAI"; break;
		  case TTS_SUS:	p = "SUS"; break;
		  case TTS_WAS:	p = "WAS"; break;
		  case TTS_DMT:	p = "DMT"; break;
		  default:		p = "???";
		}
		P(p);

		if ((ref.tskstat & TTS_WAI) != 0) {
			UINT	tev = 0;

			switch (ref.tskwait) {
			  case TTW_SLP:  p = "SLP";  break;
			  case TTW_DLY:  p = "DLY";  break;
			  case TTW_SEM:  p = "SEM";  break;
			  case TTW_FLG:  p = "FLG";  break;
			  case TTW_MBX:  p = "MBX";  break;
			  case TTW_MTX:  p = "MTX";  break;
			  case TTW_SMBF: p = "SMBF"; break;
			  case TTW_RMBF: p = "RMBF"; break;
			  case TTW_CAL:  p = "CAL";  break;
			  case TTW_ACP:  p = "ACP";  break;
			  case TTW_RDV:  p = "RDV";  break;
			  case TTW_MPF:  p = "MPF";  break;
			  case TTW_MPL:  p = "MPL";  break;
			  default:
				tev = (ref.tskwait >> 16) & 0xff;
				p = (tev != 0)? "EV": "???";
			}
			if (tev == 0) P("-%-5s", p);
			else		P("-%2s:%02x", p, tev);

			P("%c", ((ref.tskstat & TTS_NODISWAI) != 0)? '*':' ');

			if (ref.wid > 0)	P("[%3d]", ref.wid);
			else			P("     ");
		} else {
			P("            ");
		}

		/* ST+UT RID */
		tk_inf_tsk(id, &inf, FALSE);
		P(" %5d+%-5d %3d ", inf.stime / 10,
					inf.utime / 10, tk_get_rid(id));
		dsp_exinf(ref.exinf);
	}
}
/*
	dump semaphore status
*/
LOCAL	void	dump_semaphore(ID semid)
{
	T_RSEM	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxSemId", &maxid, 1);
	P("SEM STATE (MAX:%d):\n", maxid);

	if (semid > 0)	maxid = semid;
	else		semid = 1;

	P(" ID WID CNT EXINF\n");

	for (id = semid; id <= maxid; id++) {
		er = tk_ref_sem(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3d %3d ", id, ref.wtsk, ref.semcnt);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump mutex status
*/
LOCAL	void	dump_mutex(ID mtxid)
{
	T_RMTX	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxMtxId", &maxid, 1);
	P("MTX STATE (MAX:%d):\n", maxid);

	if (mtxid > 0)	maxid = mtxid;
	else		mtxid = 1;

	P(" ID HID WID EXINF\n");

	for (id = mtxid; id <= maxid; id++) {
		er = tk_ref_mtx(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3d %3d ", id, ref.htsk, ref.wtsk);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump event flag status
*/
LOCAL	void	dump_eventflag(ID flgid)
{
	T_RFLG	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxFlgId", &maxid, 1);
	P("FLG STATE (MAX:%d):\n", maxid);

	if (flgid > 0)	maxid = flgid;
	else		flgid = 1;

	P(" ID WID PTN      EXINF\n");

	for (id = flgid; id <= maxid; id++) {
		er = tk_ref_flg(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3d %08x ", id, ref.wtsk, ref.flgptn);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump mailbox status
*/
LOCAL	void	dump_mailbox(ID mbxid)
{
	T_RMBX	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxMbxId", &maxid, 1);
	P("MBX STATE (MAX:%d):\n", maxid);

	if (mbxid > 0)	maxid = mbxid;
	else		mbxid = 1;

	P(" ID WID MSG      EXINF\n");

	for (id = mbxid; id <= maxid; id++) {
		er = tk_ref_mbx(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3d %08x ", id, ref.wtsk, ref.pk_msg);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump message buffer status
*/
LOCAL	void	dump_messagebuf(ID mbfid)
{
	T_RMBF	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxMbfId", &maxid, 1);
	P("MBF STATE (MAX:%d):\n", maxid);

	if (mbfid > 0)	maxid = mbfid;
	else		mbfid = 1;

	P(" ID WID SID  MSGSZ   FREE    MAX EXINF\n");

	for (id = mbfid; id <= maxid; id++) {
		er = tk_ref_mbf(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3d %3d %6d %6d %6d ", id, ref.wtsk, ref.stsk,
					ref.msgsz, ref.frbufsz, ref.maxmsz);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump rendezvous port status
*/
LOCAL	void	dump_rdvport(ID porid)
{
	T_RPOR	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxPorId", &maxid, 1);
	P("POR STATE (MAX:%d):\n", maxid);

	if (porid > 0)	maxid = porid;
	else		porid = 1;

	P(" ID WID AID MAXCSZ MAXRSZ EXINF\n");

	for (id = porid; id <= maxid; id++) {
		er = tk_ref_por(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3d %3d %6d %6d ", id, ref.wtsk, ref.atsk,
					ref.maxcmsz, ref.maxrmsz);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump memory pool status
*/
LOCAL	void	dump_memorypool(ID mplid)
{
	T_RMPL	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxMplId", &maxid, 1);
	P("MPL STATE (MAX:%d):\n", maxid);

	if (mplid > 0)	maxid = mplid;
	else		mplid = 1;

	P(" ID WID   FREE    MAX EXINF\n");

	for (id = mplid; id <= maxid; id++) {
		er = tk_ref_mpl(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3d %6d %6d ", id, ref.wtsk, ref.frsz, ref.maxsz);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump fixed memory pool status
*/
LOCAL	void	dump_fixmempool(ID mpfid)
{
	T_RMPF	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxMpfId", &maxid, 1);
	P("MPF STATE (MAX:%d):\n", maxid);

	if (mpfid > 0)	maxid = mpfid;
	else		mpfid = 1;

	P(" ID WID FREE EXINF\n");

	for (id = mpfid; id <= maxid; id++) {
		er = tk_ref_mpf(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3d %4d ", id, ref.wtsk, ref.frbcnt);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump cyclic handler status
*/
LOCAL	void	dump_cyclichdr(ID cycid)
{
	T_RCYC	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxCycId", &maxid, 1);
	P("CYC STATE (MAX:%d):\n", maxid);

	if (cycid > 0)	maxid = cycid;
	else		cycid = 1;

	P(" ID STS   TIME EXINF\n");

	for (id = cycid; id <= maxid; id++) {
		er = tk_ref_cyc(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3s %6d ", id, ((ref.cycstat & TCYC_STA) != 0) ?
					"STA" : "", ref.lfttim);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump alarm handler status
*/
LOCAL	void	dump_alarmhdr(ID almid)
{
	T_RALM	ref;
	ID	id, maxid;
	ER	er;

	maxid = 0;
	tk_get_cfn((UB*)"TMaxAlmId", &maxid, 1);
	P("ALM STATE (MAX:%d):\n", maxid);

	if (almid > 0)	maxid = almid;
	else		almid = 1;

	P(" ID STS   TIME EXINF\n");

	for (id = almid; id <= maxid; id++) {
		er = tk_ref_alm(id, &ref);
		if (er < E_OK) {
			if (er != E_NOEXS)
				P("ERR id = %3d [%#x]\n", id, er);
			continue;
		}
		P("%3d %3s %6d ", id, ((ref.almstat & TALM_STA) != 0) ?
					"STA" : "", ref.lfttim);
		dsp_exinf(ref.exinf);
	}
}
/*
	dump task registers
*/
LOCAL	void	dump_task_register(ID tskid)
{
	T_REGS	greg;
	T_EIT	ereg;
	T_CREGS	creg;
	TD_RTSK	rtsk;
	ER	er;

	P("TASK REGISTER (TID:%d)\n", tskid);

	er = tk_get_reg(tskid, &greg, &ereg, &creg);
	if (er < E_OK) {
		P("ERR [%#x]\n", er);
		return;
	}

	PrintTaskRegister((void*)&P, &greg, &ereg, &creg);

	er = td_ref_tsk(tskid, &rtsk);
	if (er < E_OK) return;

	P("SYSTEM STACK AREA: 0x%08x - 0x%08x\n",
			(UW)rtsk.isstack - rtsk.sstksz, (UW)rtsk.isstack);
	if (rtsk.stksz > 0) {
		P("  USER STACK AREA: 0x%08x - 0x%08x\n",
			(UW)rtsk.istack - rtsk.stksz, (UW)rtsk.istack);
	} else {
		P("  USER STACK AREA: %10s - 0x%08x\n",
			"?", (UW)rtsk.istack);
	}
}
/*
	ref command
*/
LOCAL	void	cmd_ref(INT ac, B *av[])
{
	INT	num;

	if (ac < 2) goto usage;
	num = (ac >= 3) ? strtol(av[2], NULL, 0) : 0;

	if (strcmp(av[1], "mem") == 0) {
		dump_sysmem();
	} else if (strcmp(av[1], "tsk") == 0) {
		dump_task(num);
	} else if (strcmp(av[1], "sem") == 0) {
		dump_semaphore(num);
	} else if (strcmp(av[1], "flg") == 0) {
		dump_eventflag(num);
	} else if (strcmp(av[1], "mbx") == 0) {
		dump_mailbox(num);
	} else if (strcmp(av[1], "mbf") == 0) {
		dump_messagebuf(num);
	} else if (strcmp(av[1], "por") == 0) {
		dump_rdvport(num);
	} else if (strcmp(av[1], "mtx") == 0) {
		dump_mutex(num);
	} else if (strcmp(av[1], "mpl") == 0) {
		dump_memorypool(num);
	} else if (strcmp(av[1], "mpf") == 0) {
		dump_fixmempool(num);
	} else if (strcmp(av[1], "cyc") == 0) {
		dump_cyclichdr(num);
	} else if (strcmp(av[1], "alm") == 0) {
		dump_alarmhdr(num);
	} else if (strcmp(av[1], "reg") == 0) {
		if (ac < 3) goto usage;
		dump_task_register(num);
	} else {
usage:
		P("ref  item [num]   dump kernel resources\n");
		P("  item: mem,tsk,sem,flg,mbx,mbf,por,mtx,mpl,mpf,cyc,alm\n");
		P("ref  reg tskid    dump task registers\n");
	}
}

