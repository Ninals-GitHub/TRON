/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2014/07/28.
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
 *	@(#)appl_main.c
 *
 */
#include <basic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

#include <cpu/x86/cpu_insn.h>

#ifdef	USE_T2EX_FS
#define	P			printf
#define	Gets(buf, bufsz)	fgets(buf, bufsz, stdin)
#else
#define	P			tm_printf
#define	Gets(buf, bufsz)	tm_getline(buf)
#endif

/* Command functions */
IMPORT	INT	exec_cmd(B *cmd);
IMPORT	void	init_calendar_date(void);

/*
 *	Application main entry
 */
EXPORT	void	appl_main( void )
{
	B	buf[256];
	INT	fin, n;

	/* initialize calendar date */
	init_calendar_date();

	/* command processing */
	for (fin = 0; fin == 0; ) {
		P("T2EX >> ");
		Gets(buf, sizeof(buf));
		for (n = strlen(buf); --n >= 0 && buf[n] <= ' '; ) 
			buf[n] = '\0';
		P("%s\n", buf);
		if (strncmp(buf, "quit", 1) == 0) {
			fin = 1;

		/* t-monitor */
#if 0
		} else if (strncmp(buf, "#", 1) == 0) {
			tm_command(&buf[1]);

		/* misc. command */
#endif
		} else {
			if (exec_cmd(buf) == 0) {
				P("q[uit]      quit\n");
				//P("# [cmd]     exec t-monitor command\n");
				P("?           command help\n");
				P("<command>   misc. command\n");
			}
		}
	}
	
	vd_printf("fin\n");
}

