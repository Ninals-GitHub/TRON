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
 *	@(#)fastumlock.c (libtk)
 *
 *	User-mode high-speed exclusive control multi-lock
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <t2ex/util.h>
#include <libstr.h>

#define USE_ATOMIC_INT ( ATOMIC_INC_USER_MODE && ATOMIC_DEC_USER_MODE && ATOMIC_BITSET_USER_MODE && ATOMIC_BITCLR_USER_MODE  )

/* ------------------------------------------------------------------------ */

/*
 * Lock with wait time designation
 *	no	lock number 0 - 31
 */
EXPORT ER UMLockTmo( FastUMLock *lock, INT no, TMO tmo )
{
	UINT	ptn = (UINT)(1 << no);
	UINT	flg;
	ER	ercd;

#if USE_ATOMIC_INT
	atomic_inc(&lock->wai);
	for ( ;; ) {
		if ( !(atomic_bitset(&lock->flg, ptn) & ptn) ) {
			ercd = E_OK;
			break;
		}

		ercd = tk_wai_flg(lock->id, ptn, TWF_ORW|TWF_BITCLR, &flg, tmo);
		if ( ercd < E_OK ) {
			break;
		}
	}
	atomic_dec(&lock->wai);
#else
	ercd = tk_wai_flg(lock->id, ptn, TWF_ORW|TWF_BITCLR, &flg, tmo);
#endif

	return ercd;
}
EXPORT ER UMLockTmo_u( FastUMLock *lock, INT no, TMO_U tmo )
{
	UINT	ptn = (UINT)(1 << no);
	UINT	flg;
	ER	ercd;

#if USE_ATOMIC_INT
	atomic_inc(&lock->wai);
	for ( ;; ) {
		if ( !(atomic_bitset(&lock->flg, ptn) & ptn) ) {
			ercd = E_OK;
			break;
		}

		ercd = tk_wai_flg_u(lock->id, ptn, TWF_ORW|TWF_BITCLR, &flg, tmo);
		if ( ercd < E_OK ) {
			break;
		}
	}
	atomic_dec(&lock->wai);
#else
	ercd = tk_wai_flg_u(lock->id, ptn, TWF_ORW|TWF_BITCLR, &flg, tmo);
#endif

	return ercd;
}

/*
 * Lock
 *	no	Lock number 0 - 31
 */
EXPORT ER UMLock( FastUMLock *lock, INT no )
{
	return UMLockTmo(lock, no, TMO_FEVR);
}

/*
 * Lock release
 *	no	Lock number 0 - 31
 */
EXPORT ER UMUnlock( FastUMLock *lock, INT no )
{
	UINT	ptn = (UINT)(1 << no);
	ER	ercd;

#if USE_ATOMIC_INT
	atomic_bitclr(&lock->flg, ~ptn);
	ercd = ( lock->wai == 0 )? E_OK: tk_set_flg(lock->id, ptn);
#else
	ercd = tk_set_flg(lock->id, ptn);
#endif

	return ercd;
}

/*
 * Create multi-lock
 */
EXPORT ER CreateUMLock( FastUMLock *lock, CONST UB *name )
{
	T_CFLG	cflg;
	ER	ercd;

	if ( name == NULL ) {
		cflg.exinf = NULL;
	} else {
		strncpy((char*)&cflg.exinf, (char*)name, sizeof(cflg.exinf));
	}
	cflg.flgatr  = TA_TPRI | TA_WMUL | TA_NODISWAI;
#if USE_ATOMIC_INT
	cflg.iflgptn = 0;
#else
	cflg.iflgptn = ~0;
#endif

	lock->id = ercd = tk_cre_flg(&cflg);
	if ( ercd < E_OK ) {
		return ercd;
	}

	lock->wai = 0;
	lock->flg = 0;

	return E_OK;
}

/*
 * Delete multi-lock
 */
EXPORT ER DeleteUMLock( FastUMLock *lock )
{
	ER	ercd;

	if ( lock->id <= 0 ) {
		return E_PAR;
	}

	ercd = tk_del_flg(lock->id);
	if ( ercd < E_OK ) {
		return ercd;
	}

	lock->id = 0;

	return E_OK;
}
