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
 *	@(#)tkn_time.c
 *
 */

/* network time and clock management */

#include <tk/tkernel.h>

#include <sys/param.h>
#include <sys/types.h>

#include <tk/dbgspt.h>

void
tkn_microtime(struct timeval *tv)
{
	SYSTIM  systim;
	UINT    nsoff;
	int64_t msec, sec, usec;

	td_get_tim(&systim, &nsoff);
	msec = systim.hi;
	msec = (msec << 32) | systim.lo;
	sec  = msec / 1000;
	msec = msec - sec * 1000;
	usec = msec * 1000 + nsoff / 1000;

	tv->tv_sec  = sec;	/* TODO: time_t overflow. */
	tv->tv_usec = usec;
}

struct timeval startup_tv;

time_t tkn_time_second(void)
{
	SYSTIM tim;

	tk_get_tim(&tim);

	return ((((uint64_t)tim.hi) << 32) | tim.lo) / 1000;
}

time_t tkn_time_uptime(void)
{
	SYSTIM tim;

	tk_get_otm(&tim);

	return ((((uint64_t)tim.hi) << 32) | tim.lo) / 1000;
}

void tkn_tc_setclock(struct timespec *ts)
{
	SYSTIM tim;

	uint64_t hilo = ts->tv_sec * 1000 + ts->tv_nsec / 1000000;
	tim.hi = hilo >> 32;
	tim.lo = hilo & 0xffffffffU;

	tk_set_tim(&tim);
}

void tkn_getmicrotime(struct timeval *tv)
{
	tkn_microtime(tv);
}

void tkn_getmicrouptime(struct timeval *tv)
{
	SYSTIM  systim;
	UINT    nsoff;
	int64_t msec, sec, usec;

	td_get_otm(&systim, &nsoff);
	msec = systim.hi;
	msec = (msec << 32) | systim.lo;
	sec  = msec / 1000;
	msec = msec - sec * 1000;
	usec = msec * 1000 + nsoff / 1000;

	tv->tv_sec  = sec;	/* TODO: time_t overflow. */
	tv->tv_usec = usec;
}

void tkn_getnanotime(struct timespec *ts)
{
	SYSTIM tim;
	UINT ofs;

	td_get_tim(&tim, &ofs);

	ts->tv_sec = (((uint64_t)tim.hi << 32)  | tim.lo) / 1000;
	ts->tv_nsec = ofs;
}

