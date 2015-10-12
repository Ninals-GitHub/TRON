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
 *	@(#)commarea.h (sys)
 *
 *	Kernel common data
 */

#ifndef __SYS_COMMAREA_H__
#define __SYS_COMMAREA_H__

#include <basic.h>
#include <sys/queue.h>
#include <tk/util.h>
#include <tk/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __commarea__
#define __commarea__
typedef struct CommArea	CommArea;
#endif

/*
 * Kernel common data structure
 */
struct CommArea {
	FastLock	*PinfoLock;
	ID		SysResID;
	FUNCP		GetPinfo;
	FUNCP		GetPidToPinfo;
	FUNCP		GetLSID;
	FUNCP		GetUATB;
#if TA_GP
	void		*gp;
#endif
        INT             tev_fflock;
};

/*
 * Kernel common data reference address
 */
IMPORT CommArea	*__CommArea;

#ifdef __cplusplus
}
#endif
#endif /* __SYS_COMMAREA_H__ */
