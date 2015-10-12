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
 *	@(#)fastulock.c (libtk)
 *
 *	User-mode high-speed exclusive control lock
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <t2ex/util.h>
#include <sys/atomic.h>
#include <libstr.h>

#define USE_ATOMIC_INT ( ATOMIC_INC_USER_MODE && ATOMIC_DEC_USER_MODE )

/* ------------------------------------------------------------------------ */

/*
 * Lock
 */
EXPORT void ULock( FastULock *lock )
{
#if USE_ATOMIC_INT
	if ( (INT)atomic_inc((INT*)&lock->cnt) > 0 ) {
		tk_wai_sem(lock->id, 1, TMO_FEVR);
	}
#else
	tk_wai_sem(lock->id, 1, TMO_FEVR);
#endif
}

/*
 * Lock release
 */
EXPORT void UUnlock( FastULock *lock )
{
#if USE_ATOMIC_INT
	if ( (INT)atomic_dec((INT*)&lock->cnt) >= 0 ) {
		tk_sig_sem(lock->id, 1);
	}
#else
	tk_sig_sem(lock->id, 1);
#endif
}

/*
 * Create high-speed lock
 */
EXPORT ER CreateULock( FastULock *lock, CONST UB *name )
{
	T_CSEM	csem;
	ER	ercd;

	if ( name == NULL ) {
		csem.exinf = NULL;
	} else {
		strncpy((char*)&csem.exinf, (char*)name, sizeof(csem.exinf));
	}
	csem.sematr  = TA_TPRI | TA_NODISWAI;
#if USE_ATOMIC_INT
	csem.isemcnt = 0;
#else
	csem.isemcnt = 1;
#endif
	csem.maxsem  = 1;

	ercd = tk_cre_sem(&csem);
	if ( ercd < E_OK ) {
		return ercd;
	}

	lock->id = ercd;
	lock->cnt = (UINT)-1;

	return E_OK;
}

/*
 * Delete high-speed lock
 */
EXPORT void DeleteULock( FastULock *lock )
{
	if ( lock->id > 0 ) {
		tk_del_sem(lock->id);
	}
	lock->id = 0;
}
