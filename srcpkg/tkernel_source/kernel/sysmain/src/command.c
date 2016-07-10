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
 *	@(#)command.c
 *
 */

#include <basic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tk/tkernel.h>
#include <t2ex/datetime.h>
#include <t2ex/fs.h>
#include <t2ex/load.h>
#include <device/clk.h>

#ifdef	USE_T2EX_FS
#define	P		printf
#else
#define	P		tm_printf
#endif

#define	N_ARGS		16

/*
	ref command
*/
#include "ref_command.c"

/*
	Get from / Set to RTC
*/
LOCAL	void	rtc_get_set(SYSTIM *stm, BOOL set)
{
	ID		dd;
	W		asz;
	ER		er;
	DATE_TIM	dt;
	struct tm	ctm;
	struct tzinfo	tz;

	/* open RTC (CLOCK) driver */
	dd = tk_opn_dev("CLOCK", (set == TRUE) ? TD_WRITE : TD_READ);
	if (dd < E_OK) {
		P("Can't open CLOCK device [%#x]\n", dd);
		goto exit0;
	}

	/* Note: The RTC is set to UTC rather than local time. because UTC
	   is not affected by time zone and daylight saving time. */

	if (set == TRUE) {	/* set date to RTC */

		/* convert system time to UTC date/time */
		dt_gmtime_ms(stm, &ctm);

		/* set local date/time to RTC */
		dt.d_year = ctm.tm_year;
		dt.d_month = ctm.tm_mon + 1;
		dt.d_day = ctm.tm_mday;
		dt.d_hour = ctm.tm_hour;
		dt.d_min = ctm.tm_min;
		dt.d_sec = ctm.tm_sec;
		dt.d_wday = ctm.tm_wday;
		er = tk_swri_dev(dd, DN_CKDATETIME, &dt, sizeof(dt), &asz);
		if (er < E_OK) {
			P("Can't set CLOCK [%#x]\n", er);
			goto exit1;
		}
	} else {		/* get date from RTC */

		/* get timezone of "UTC+0" */
		memset(&tz, 0, sizeof(tz));
		er = dt_tzset(&tz, "UTC+0");
		if (er < E_OK ) {
			P("dt_tzset(UTC+0) ERR [%#x]\n", er);
			goto exit1;
		}

		/* get UTC date/time from RTC */
		er = tk_srea_dev(dd, DN_CKDATETIME, &dt, sizeof(dt), &asz);
		if (er < E_OK) {
			P("Can't get CLOCK [%#x]\n", er);
			goto exit1;
		}

		/* convert to system time */
		ctm.tm_year = dt.d_year;
		ctm.tm_mon = dt.d_month - 1;
		ctm.tm_mday = dt.d_day;
		ctm.tm_hour = dt.d_hour;
		ctm.tm_min = dt.d_min;
		ctm.tm_sec = dt.d_sec;
		ctm.tm_wday = dt.d_wday;
		ctm.tm_usec = 0;
		ctm.tm_wday = -1;
		ctm.tm_isdst = 0;
		ctm.tm_yday = 0;
		dt_mktime_ms(&ctm, &tz, stm);
	}
exit1:
	tk_cls_dev(dd, 0);
exit0:
	return;
}

/*
	Initialize calendar date
*/
EXPORT	void	init_calendar_date(void)
{
	ER	er;
	struct tzinfo	tz;
	SYSTIM	stm;

	/* initialize system timezone to "JST-9" */
	memset(&tz, 0, sizeof(tz));
	er = dt_tzset(&tz, "JST-9");
	if (er < E_OK ) {
		P("dt_tzset(JST-9) ERR [%#x]\n", er);
	} else {
		er = dt_setsystz(&tz);
		if (er < E_OK) {
			P("dt_setsystz() ERR [%#x]\n", er);
		}
	}

	/* initialize calendar data/time from RTC */
	stm.hi = stm.lo = 0;
	rtc_get_set(&stm, FALSE);
	tk_set_tim(&stm);
}

/*
	date command
*/
LOCAL	void	cmd_date(INT ac, B *av[])
{
	ER	er;
	struct tm	ctm;
	SYSTIM	stm;
static	const B	*week[] = {"sun","mon","tue","wed","thu","fri","sat"};

	if (ac < 2) {		/* show current date */
		tk_get_tim(&stm);
		er = dt_localtime_ms(&stm, NULL, &ctm);
		if (er < E_OK) {
			P("dt_localtime() ERR [%#x]\n", er);
		} else {
			P("%d/%02d/%02d (%s) %02d:%02d:%02d\n",
				ctm.tm_year + 1900, ctm.tm_mon + 1,
				ctm.tm_mday, week[ctm.tm_wday],
				ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
		}
	} else if (ac >= 4) {	/* set current date */
		ctm.tm_year = atoi(av[1]) - 1900;
		ctm.tm_mon = atoi(av[2]) - 1;
		ctm.tm_mday = atoi(av[3]);
		ctm.tm_hour = (ac >= 5) ? atoi(av[4]) : 0;
		ctm.tm_min = (ac >= 6) ? atoi(av[5]) : 0;
		ctm.tm_sec = (ac >= 7) ? atoi(av[6]) : 0;
		ctm.tm_usec = 0;
		ctm.tm_wday = -1;
		ctm.tm_isdst = 0;
		ctm.tm_yday = 0;
		er = dt_mktime_ms(&ctm, NULL, &stm);
		if (er < E_OK) {
			P("dt_mktime() ERR [%#x]\n", er);
		} else {
			er = tk_set_tim(&stm);
			if (er < E_OK) {
				P("tk_set_tim() ERR [%#x]\n", er);
			} else {
				rtc_get_set(&stm, TRUE);
			}
		}
	} else if (ac == 2 && strcmp(av[1], "init") == 0) {
		/* initialize current date from RTC */
		init_calendar_date();
	}
}

/*
	attach command
*/
LOCAL	void	cmd_attach(INT ac, B *av[])
{
	ER	er;

	if (ac < 3) return;

	er = fs_attach(av[1], av[2], FIMP_FAT, 0, NULL);
	if (er >= E_OK) {
		P("attached '%s' at /'%s'\n", av[1], av[2]);
	} else {
		P("fs_attach(%s, %s, FIMP_FAT ..) ERR [%#x]\n",
						av[1], av[2], er);
	}
}

/*
	detach command
*/
LOCAL	void	cmd_detach(INT ac, B *av[])
{
	ER	er;

	if (ac < 2) return;

	er = fs_detach(av[1]);
	if (er >= E_OK) {
		P("detached '%s'\n", av[1]);
	} else {
		P("fs_detach(%s) ERR [%#x]\n", av[1], er);
	}
}

/*
	mkdir command
*/
LOCAL	void	cmd_mkdir(INT ac, B *av[])
{
	ER	er;
	INT	mode;

	if (ac < 2) return;

	mode =	(ac >= 3) ? strtol(av[2], NULL, 0) : 0777;
	er = fs_mkdir(av[1], mode);
	if (er >= E_OK) {
		P("directory '%s' %o created\n", av[1], mode);
	} else {
		P("fs_mkdir(%s, %o) ERR [%#x]\n", av[1], mode, er);
	}
}

/*
	rmdir command
*/
LOCAL	void	cmd_rmdir(INT ac, B *av[])
{
	ER	er;

	if (ac < 2) return;

	er = fs_rmdir(av[1]);
	if (er >= E_OK) {
		P("directory '%s' removed\n", av[1]);
	} else {
		P("fs_rmdir(%s) ERR [%#x]\n", av[1], er);
	}
}

/*
	rm command
*/
LOCAL	void	cmd_rm(INT ac, B *av[])
{
	ER	er;

	if (ac < 2) return;

	er = fs_unlink(av[1]);
	if (er >= E_OK) {
		P("file '%s' removed\n", av[1]);
	} else {
		P("fs_unlink(%s) ERR [%#x]\n", av[1], er);
	}
}

/*
	mv command
*/
LOCAL	void	cmd_mv(INT ac, B *av[])
{
	ER	er;

	if (ac < 3) return;

	er = fs_rename(av[1], av[2]);
	if (er >= E_OK) {
		P("file '%s' -> '%s' moved\n", av[1], av[2]);
	} else {
		P("fs_rename(%s,%s) ERR [%#x]\n", av[1], av[2], er);
	}
}

/*
	pwd command
*/
LOCAL	void	cmd_pwd(INT ac, B *av[])
{
	B	buf[FILENAME_MAX + 1];
	ER	er;

	er = fs_getcwd(buf, sizeof(buf) -1);
	if (er >= E_OK) {
		P("== '%s'\n", buf);
	} else {
		P("fs_getcwd(..) ERR [%#x]\n", er);
	}
}

/*
	cd command
*/
LOCAL	void	cmd_cd(INT ac, B *av[])
{
	ER	er;

	if (ac < 2) return;

	er = fs_chdir(av[1]);
	if (er >= E_OK) {
		P("=> '%s'\n", av[1]);
	} else {
		P("fs_chdir(%s) ERR [%#x]\n", av[1], er);
	}
}

/*
	ls command
*/
LOCAL	void	cmd_ls(INT ac, B *av[])
{
	ER	er;
	DIR	*dir;
	struct dirent	ent, *entp;
	B	path[FILENAME_MAX + 1];
	struct stat_ms	st;
	struct tm	ctm;
	INT	n, opt;

	opt = 0;
	for (n = 1; n < ac; n++) {
		if (*av[n] != '-') break;
		switch(*(av[n] + 1)) {
		case 't':	opt |= 0x01;	break;
		case 'a':	opt |= 0x02;	break;
		}
	}

	if (ac <= n) {
		fs_getcwd(path, sizeof(path) -1);
	} else {
		strcpy(path, av[n]);
	}

	dir = opendir(path);
	if (dir == NULL) {
		P("opendir ERR\n");
		return;
	}

	n = strlen(path);
	path[n++] = '/';

	while (readdir_r(dir, &ent, &entp) >= 0) {
		if (entp == NULL) break;
		if (strcmp(entp->d_name, ".") == 0 ||
		    strcmp(entp->d_name, "..") == 0) continue;

		strcpy(&path[n],  entp->d_name);
		er = fs_stat_ms(path, &st);
		if (er < E_OK) {
			P("fs_stat(%s) ERR [%#x]\n", path, er);
			continue;
		}
		dt_localtime_ms(&st.st_mtime, NULL, &ctm);

		if (S_ISDIR(st.st_mode))	strcat(path, "/");
		else if (! S_ISREG(st.st_mode)) strcat(path, "#");

		P("%-16s %8d %04o %d/%02d/%02d %02d:%02d:%02d",
			&path[n], st.st_size, st.st_mode & 0x1FF,
			ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday,
			ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
		if (opt & 0x01) {
			dt_localtime_ms(&st.st_ctime, NULL, &ctm);
			P(" c:%d/%02d/%02d %02d:%02d:%02d",
				ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday,
				ctm.tm_hour, ctm.tm_min, ctm.tm_sec);
			dt_localtime_ms(&st.st_atime, NULL, &ctm);
			P(" a:%d/%02d/%02d",
				ctm.tm_year + 1900, ctm.tm_mon + 1, ctm.tm_mday);
		}
		if (opt & 0x02) {
			P(" b:%-4d m:%#04x i:%d",
				(W)st.st_blocks, st.st_mode, (W)st.st_ino);
		}
		P("\n");
	}

	closedir(dir);
}

/*
	tp / tpx command
*/
LOCAL	void	cmd_tp(INT ac, B *av[])
{
	INT	hex, ofs, len, fsz, i;
	FILE	*fp;
	UB	*buf;

	if (ac < 2) return;

	hex = (av[0][2] == 'x') ? 1 : 0;

	ofs = (ac >= 3) ? strtol(av[2], NULL, 0) : 0;
	len = (ac >= 4) ? strtol(av[3], NULL, 0) : 0;

	fp = fopen(av[1], "r");
	if (!fp) {
		P("%s open ERR\n", av[1]);
		return;
	}
	fseek(fp, 0, SEEK_END);
	fsz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (fsz <= 0) {
		P("%s is empty\n", av[1]);
		goto exit1;
	}

	if (ofs >= fsz) {
		P("bad offset: %d >= %d\n", ofs, fsz);
		goto exit1;
	}
	i = fsz - ofs;
	if (len == 0 || len > i) len = i;

	buf = malloc(fsz + 1);
	if (!buf) {
		P("No memory\n");
		goto exit1;
	}
	if (fread(buf, fsz, 1, fp) != 1) {
		P("read ERR\n");
		goto exit2;
	}
	if (hex == 0) {
		for (i = 0; i < len; i++) {
			P("%c", buf[ofs + i]);
		}
	} else {
		for (i = 0; i < len; ) {
			if ((i % 16) == 0) P("%04x:", ofs + i);
			P(" %02x", buf[ofs + i]);
			if ((++i % 16) == 0) P("\n");
		}
		if ((i % 16) != 0) P("\n");
	}

exit2:
	free(buf);
exit1:
	fclose(fp);
}

/*
	cp command
*/
LOCAL	void	cmd_cp(INT ac, B *av[])
{
	INT	fsz, n, wofs, wlen;
	FILE	*sfp, *dfp;
	UB	*buf = NULL;
	B	dst[FILENAME_MAX + 1], *p;
	struct stat_ms	st;

	if (ac < 3) return;

	wofs = (ac < 4) ? 0 : strtol(av[3], NULL, 0);
	wlen = (ac < 5) ? 0 : strtol(av[4], NULL, 0);

	sfp = fopen(av[1], "r");
	if (!sfp) {
		P("%s open ERR\n", av[1]);
		goto exit0;
	}

	fseek(sfp, 0, SEEK_END);
	fsz = ftell(sfp);
	fseek(sfp, 0, SEEK_SET);
	if (fsz > 0) {
		buf = malloc(fsz + 1);
		if (!buf) {
			P("No memory\n");
			goto exit1;
		}
		if (fread(buf, fsz, 1, sfp) != 1) {
			P("read ERR\n");
			goto exit2;
		}
	}

	n = strlen(av[2]);
	if (n > FILENAME_MAX) {
		P("too long path: %s\n", av[2]);
		goto exit2;
	}
	strcpy(dst, av[2]);
	if (dst[n - 1] == '/' ||
		(fs_stat_ms(dst, &st) >= E_OK && S_ISDIR(st.st_mode)) ) {
		if (dst[n - 1] != '/') dst[n++] = '/';
		p = strrchr(av[1], '/');
		if (p != NULL)	p++;
		else		p = av[1];
		if (strlen(p) + n > FILENAME_MAX) {
			P("too long path: %s/%s\n", av[2], av[1]);
			goto exit2;
		}
		strcpy(&dst[n], p);
	}

	if (wofs > 0) {
		dfp = fopen(dst, "r+");
		if (!dfp) {
			P("%s open ERR\n", dst);
			goto exit2;
		}
		if (fseek(dfp, wofs, SEEK_SET) < 0) {
			P("%s fseek(%d) ERR\n", dst, wofs);
			goto exit3;
		}
	} else {
		dfp = fopen(dst, (wofs < 0) ? "a" : "w");
		if (!dfp) {
			P("%s open ERR\n", dst);
			goto exit2;
		}
	}
	if (fsz > 0) {
		if (wlen <= 0 || wlen > fsz) wlen = fsz;
		if (fwrite(buf, wlen, 1, dfp) != 1) {
			P("write ERR\n");
			goto exit3;
		}
		P("%s ", av[1]);
		if (fsz != wlen) P("[%d bytes] ", wlen);
		if (wofs == 0) {
			P("is copied to %s\n", dst);
		} else if (wofs < 0) {
			P("is appended to %s\n", dst);
		} else {
			P("is overwrited to %s at %d\n", dst, wofs);
		}
	}
exit3:
	fclose(dfp);
exit2:
	free(buf);
exit1:
	fclose(sfp);
exit0:
	;
}

/*
	trunc command
*/
LOCAL	void	cmd_trunc(INT ac, B *av[])
{
	ER	er;
	INT	len;

	if (ac < 3) return;

	len = strtol(av[2], NULL, 0);

	er = fs_truncate(av[1], len);
	if (er >= E_OK) {
		P("'%s' is truncated to %d\n", av[1], len);
	} else {
		P("fs_truncate(%s, %d) ERR [%#x]\n", av[1], len, er);
	}
}

/*
	df command
*/
LOCAL	void	cmd_df(INT ac, B *av[])
{
	ER			er;
	struct statvfs	buf;

	if (ac < 2) return;

	er = fs_statvfs(av[1], &buf);
	if (er < E_OK) {
		P("fs_statvfs(%s) ERR [%#x]\n", av[1], er);
		return;
	}
	P("Blocks: total = %d free = %d blksz = %d used = %d %%\n",
		(W)buf.f_blocks, (W)buf.f_bfree, buf.f_frsize,
		((buf.f_blocks - buf.f_bfree) * 100 + buf.f_blocks - 1)
			/ buf.f_blocks);
}

/*
	sync command
*/
LOCAL	void	cmd_sync(INT ac, B *av[])
{
	ER	er;
	INT	fd;

	if (ac < 2) {
		er = fs_sync();
		if (er < E_OK) {
			P("fs_sync ERR [%#x]\n", er);
		}
	} else {
		fd = fs_open(av[1], O_RDWR);
		if (fd < 0) {
			P("fs_open(%s) ERR [%#x]\n", av[1], fd);
		} else {
			if (ac >= 3 && *av[2] == 'd') {
				er = fs_fdatasync(fd);
				if (er < E_OK) {
					P("fs_fdatasync ERR [%#x]\n", er);
				}
			} else {
				er = fs_fsync(fd);
				if (er < E_OK) {
					P("fs_sync ERR [%#x]\n", er);
				}
			}
		}
	}
}

/*
	chmod command
*/
LOCAL	void	cmd_chmod(INT ac, B *av[])
{
	ER	er;
	INT	md;

	if (ac < 3) return;

	md = strtol(av[2], NULL, 0);

	er = fs_chmod(av[1], md);
	if (er < E_OK) {
		P("fs_chmod(%s, %#x/0%o) ERR [%#x]\n", av[1], md, md, er);
	}
}

/*
	load command
*/
LOCAL	void	cmd_load(INT ac, B *av[])
{
	struct pm	prog;
	pm_entry_t*	entry;
	ER		er;

	if (ac < 2) return;

	prog.pmtype = PM_FILE;
	prog.pmhdr  = av[1];

	er = pm_load(&prog, TA_RNG3, &entry);
	if (er < E_OK) {
		P("pm_load ERR [%#x]\n", er);
		return;
	}
	P("[%d] PROG (entry at %d)\n", er, (int)entry);
}

/*
	lodspg command
*/
LOCAL	void	cmd_loadspg(INT ac, B *av[])
{
	struct pm	prog;
	ER		er;

	if (ac < 2) return;

	prog.pmtype = PM_FILE;
	prog.pmhdr  = av[1];

	er = pm_loadspg(&prog, ac - 1, (UB**)(av + 1));
	if (er < E_OK) {
		P("pm_loadspg ERR [%#x]\n", er);
		return;
	}
	P("[%d] SYSPRG\n", er);
}

/*
	unload command
*/
LOCAL	void	cmd_unload(INT ac, B *av[])
{
	ER	er;
	INT	progid;

	if (ac < 2) return;

	progid = strtol(av[1], NULL, 0);

	er = pm_unload(progid);
	if (er < E_OK) {
		P("pm_unload(%d) ERR [%#x]\n", progid, er);
	}
}

/*
	call command
*/
LOCAL	void	cmd_call(INT ac, B *av[])
{
	FP	fnc;
	W	p1, p2, p3;

	if (ac < 2) return;

	fnc = (FP)strtol(av[1], NULL, 0);
	p1 = (ac >= 3) ? strtol(av[2], NULL, 0) : 0;
	p2 = (ac >= 4) ? strtol(av[3], NULL, 0) : 0;
	p3 = (ac >= 5) ? strtol(av[4], NULL, 0) : 0;

	(*fnc)(p1, p2, p3);
}

/*
	setup parameters
*/
LOCAL	INT	setup_param(B *bp, B **av)
{
	INT	ac;

	for (ac = 0; ac < N_ARGS; ac++) {
		while (*((UB*)bp) <= ' ' && *bp != '\0') bp++;
		if (*bp == '\0') break;
		av[ac] = bp;
		while (*((UB*)bp) > ' ') bp++;
		if (*bp != '\0') {
			*bp++ = '\0';
		}
	}
	av[ac] = NULL;
	return ac;
}

/*
	execute command
*/
EXPORT	INT	exec_cmd(B *cmd)
{
	INT	ac;
	B	*av[N_ARGS];

	ac = setup_param(cmd, av);
	if (ac < 1) return 0;

	if (strcmp(av[0], "date") == 0) {
		cmd_date(ac, av);
	} else if (strcmp(av[0], "attach") == 0) {
		cmd_attach(ac, av);
	} else if (strcmp(av[0], "detach") == 0) {
		cmd_detach(ac, av);
	} else if (strcmp(av[0], "mkdir") == 0) {
		cmd_mkdir(ac, av);
	} else if (strcmp(av[0], "rmdir") == 0) {
		cmd_rmdir(ac, av);
	} else if (strcmp(av[0], "pwd") == 0) {
		cmd_pwd(ac, av);
	} else if (strcmp(av[0], "cd") == 0) {
		cmd_cd(ac, av);
	} else if (strcmp(av[0], "rm") == 0) {
		cmd_rm(ac, av);
	} else if (strcmp(av[0], "mv") == 0) {
		cmd_mv(ac, av);
	} else if (strcmp(av[0], "ls") == 0) {
		cmd_ls(ac, av);
	} else if (strcmp(av[0], "tp") == 0 || strcmp(av[0], "tpx") == 0) {
		cmd_tp(ac, av);
	} else if (strcmp(av[0], "cp") == 0) {
		cmd_cp(ac, av);
	} else if (strcmp(av[0], "trunc") == 0) {
		cmd_trunc(ac, av);
	} else if (strcmp(av[0], "df") == 0) {
		cmd_df(ac, av);
	} else if (strcmp(av[0], "sync") == 0) {
		cmd_sync(ac, av);
	} else if (strcmp(av[0], "chmod") == 0) {
		cmd_chmod(ac, av);
	} else if (strcmp(av[0], "ref") == 0) {
		cmd_ref(ac, av);
	} else if (strcmp(av[0], "load") == 0) {
		cmd_load(ac, av);
	} else if (strcmp(av[0], "loadspg") == 0) {
		cmd_loadspg(ac, av);
	} else if (strcmp(av[0], "unload") == 0) {
		cmd_unload(ac, av);
	} else if (strcmp(av[0], "call") == 0) {
		cmd_call(ac, av);
	} else if (strncmp(av[0], "?", 1) == 0) {
		P("date     [y m d [h m s]]\n");
		P("attach   devnm connm\n");
		P("detach   connm\n");
		P("cd       dir\n");
		P("pwd      \n");
		P("ls       [-t][-l][dir]\n");
		P("mkdir    dir [mode]\n");
		P("rmdir    dir\n");
		P("rm       path\n");
		P("mv       o-path n-path\n");
		P("trunc    path len\n");
		P("df       path\n");
		P("sync     [path [d]]\n");
		P("chmod    path mode\n");
		P("tp       path [ofs len]\n");
		P("tpx      path [ofs len]\n");
		P("cp       s-path d-path/dir [wofs [wlen]]\n");
		P("ref      [item]\n");
		P("call     addr [p1 p2 p3]\n");
		P("load     path\n");
		P("loadspg  path [arg ...]\n");
		P("unload   progid\n");
#ifdef	NET_SAMPLE
		P("net      execute network sample\n");

	} else if (strcmp(av[0], "net") == 0) {
IMPORT	void	net_test(void);
		net_test();
#endif
	} else {
		return 0;
	}
	return 1;
}

