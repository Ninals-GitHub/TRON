/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/08.
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
 *	@(#)tkn_log.c
 *
 */

#include <tk/tkernel.h>
#include <tm/tmonitor.h>


#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>

#include <stdarg.h>
#include <sys/syslog.h>

#if defined(T2EX_LOG) || defined(DEBUG)
LOCAL void log_output(const char* format, va_list ap)
{
	static char buf[128];

	vsnprintf(buf, sizeof(buf), format, ap);
	buf[sizeof(buf) - 1] = '\0';

	tm_putstring((UB*)buf);
}
#endif

#ifdef T2EX_LOG
/* log level */
#ifndef TKN_LOG_LEVEL
#define TKN_LOG_LEVEL LOG_WARNING
#endif

int tkn_log_level = TKN_LOG_LEVEL;

/*
 * message logging - instead of kernel log func. 'log()'
 */
void
log(int level, const char* format, ...)
{
	va_list list;

	if (level > tkn_log_level) {
		return;
	}

	va_start(list, format);
	log_output(format, list);
	va_end(list);
}
#endif

#ifdef DEBUG
void tkn_panic(const char* format, ...)
{
	va_list list;

	tm_putstring((UB*)"panic: ");

	va_start(list, format);
	log_output(format, list);
	va_end(list);

	tm_putstring((UB*)"\n" );

	/* display register */
	tm_command((UB*)"R C");

	for (;;) tm_monitor();			/* stop the system */
}
#endif
