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
 *	@(#)tkn_rwlock.c
 *
 */

#include <tk/tkernel.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/rwlock.h>
#include <sys/_queue.h>

#include <sys/tkn_intr.h>

#include <tk/util.h>
#include "tkn.h"

#define MAX_READERS	65535

LOCAL int init_semaphore( __volatile krwlock_t *rw )
{
	T_CSEM	csem;

	SetOBJNAME(csem.exinf, "Nrw");
	csem.sematr  = TA_TFIFO | TA_FIRST;
	csem.isemcnt = MAX_READERS;
	csem.maxsem = MAX_READERS;
	rw->semid = tk_cre_sem(&csem);
	rw->oldspl = -1;
	if ( rw->semid < E_OK ) {
		return ENOMEM;
	}

	rw->is_writer = 0;

	return 0;
}

/* ------------------------------------------------------------------------ */

/* reader/writer locks */

int tkn_rw_init(krwlock_t *rw)
{
	int error = 0;

	LockTKN();

	if ( rw->semid == 0 ) error = init_semaphore(rw);

	UnlockTKN();

	return error;
}

void tkn_rw_destroy(krwlock_t *rw)
{
	LockTKN();

	if ( rw->semid != 0 ) {
		tk_del_sem(rw->semid);
		if ( rw->oldspl >= 0 ) {
			splx(rw->oldspl);
		}
	}

	rw->semid = 0;

	UnlockTKN();
}

LOCAL int tkn_rw_enter_tmo(krwlock_t *rw, const krw_t op, TMO tmo)
{
	LockTKN();

	if ( rw->semid == 0 ) panic("tkn_rw_enter_tmo: not initialized.\n");

	UnlockTKN();

	int is_writer = op == RW_WRITER;
	int count = is_writer ? MAX_READERS : 1;

	rw->oldspl = splvm();

	ER ercd = tk_wai_sem(rw->semid, count, tmo);
	if ( ercd == E_OK ) {
		LockTKN();
		rw->is_writer = is_writer;
		UnlockTKN();
		return 1;
	} else {
		return 0;
	}
}

void tkn_rw_enter(krwlock_t *rw, const krw_t op)
{
	tkn_rw_enter_tmo(rw, op, TMO_FEVR);
}

int tkn_rw_tryenter(krwlock_t *rw, const krw_t op)
{
	return tkn_rw_enter_tmo(rw, op, TMO_POL);
}

void tkn_rw_exit(krwlock_t *rw)
{
	LockTKN();

	if ( rw->semid == 0 ) panic("tkn_rw_exit: not initialized.\n");

	int s = rw->oldspl;
	rw->oldspl = -1;
	tk_sig_sem(rw->semid, rw->is_writer ? MAX_READERS : 1);

	if ( s >= 0 ) {
		splx(s);
	}

	UnlockTKN();
}

