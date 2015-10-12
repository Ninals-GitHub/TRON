/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
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
 *	@(#)fs_main.c
 *
 */


#include "fsdefs.h"

/* Default "console" file system */
IMPORT	const	fs_fimp_t	fimp_consolefs_entry;
LOCAL	const	B		console_name[] = "consolefs";
LOCAL	const	B		console_con_name[] = "console";

/* Default "FAT" file system */
IMPORT	const	fs_fimp_t	fimp_fatfs_entry;

/* vga file system */
IMPORT const fs_fimp_t fimp_vga_entry;
LOCAL const B vga_name[] = "vgafs";
LOCAL const B vga_con_name[] = "vga";

/* kbd file system */
IMPORT const fs_fimp_t fimp_kbd_entry;
LOCAL const B kbd_name[] = "kbdfs";
LOCAL const B kbd_con_name[] = "kbd";

/*
 *  Initialize Console FS and stdio
 */
LOCAL	INT	stdio_initialize(void)
{
	INT	sts, stdinfd, stdoutfd, stderrfd;

	/* Regist "console" FIMP */
	//sts = fs_regist(console_name, &fimp_consolefs_entry, NULL);
	sts = fs_regist(vga_name, &fimp_vga_entry, NULL);
	if (sts != 0) goto exit0;
	
	sts = fs_regist(kbd_name, &fimp_kbd_entry, NULL);
	if (sts != 0) goto exit0;

	/* Attach "console" FIMP */
	//sts = fs_attach(NULL, console_con_name, console_name, 0, NULL);
	sts = fs_attach(NULL, vga_con_name, vga_name, 0, NULL);
	if (sts != 0) goto exit0;
	
	sts = fs_attach(NULL, kbd_con_name, kbd_name, 0, NULL);
	if (sts != 0) goto exit0;

	/* Open stdio */
	//stdinfd  = fs_open("/console/stdin",  O_RDONLY);
	//stdoutfd = fs_open("/console/stdout", O_WRONLY);
	//stderrfd = fs_open("/console/stderr", O_WRONLY);
	stdinfd = fs_open("/kbd/stdin", O_RDONLY);
	stdoutfd = fs_open("/vga/stdout", O_WRONLY);
	stderrfd = fs_open("/vga/stderr", O_WRONLY);

	if (stdinfd != STDIN_FILENO || stdoutfd != STDOUT_FILENO ||
						stderrfd != STDERR_FILENO) {
		(void)fs_close(stdinfd);
		(void)fs_close(stdoutfd);
		(void)fs_close(stderrfd);
		sts = EX_AGAIN;
	}
	
exit0:
	return sts;
}

/*
 *  Initialize Fat FS
 */
LOCAL	INT	fatfs_initialize(void)
{
	/* Regist "FAT" FIMP */
	return fs_regist(FIMP_FAT, &fimp_fatfs_entry, 0);
}

/*
 *  File system function : main entry
 */
EXPORT	INT	fs_main(INT ac, UB *arg[])
{
	INT	sts;

	sts = 0;
	if (ac >= 0) {		/* Startup */
		if (fs_tk_is_initialized() == 0) {
			sts = fs_tk_initialize();
			if (sts != 0) goto exit0;

			sts = stdio_initialize();
			if (sts == 0) {
				sts = fatfs_initialize();
			}
			if (sts != 0) {
				(void)fs_tk_finalize();
			}
		}
	} else {		/* Finish */
		/* Assume all tasks are never executing fs_xxx() call !! */
		if (fs_tk_is_initialized() != 0) {
			fs_sync();
			sts = fs_tk_finalize();
		}
	}
	
	vd_printf("ramdisk attatch %d\n", fs_attach("mda", "ram", FIMP_FAT, 0, NULL));
exit0:
	return sts;
}

