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
 *	@(#)fastmlock.c (libtk)
 *
 *	High-speed exclusive control multi-lock
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/util.h>
#include <libstr.h>

/* ------------------------------------------------------------------------ */
/*
 *	void INC( INT *val )		increment
 *	void DEC( INT *val )		decrement
 *	BOOL BTS( UINT *val, INT no )	bit test and set
 *	void BR( UINT *val, INT no )	bit reset
 *
 *	The above must be operated exclusively.
 */

Inline void INC( INT *val )
{
	UINT	imask;

	DI(imask);
	(*val)++;
	EI(imask);
}

Inline void DEC( INT *val )
{
	UINT	imask;

	DI(imask);
	(*val)--;
	EI(imask);
}

Inline BOOL BTS( UINT *val, INT no )
{
	UINT	imask;
	UINT	b;
	UINT	bm = (UINT)(1 << no);

	DI(imask);
	b = *val & bm;
	*val |= bm;
	EI(imask);
	return (BOOL)b;
}

Inline void BR( UINT *val, INT no )
{
	UINT	imask;

	DI(imask);
	*val &= ~(UINT)(1 << no);
	EI(imask);
}

/* ------------------------------------------------------------------------ */

/*
 * Lock with wait time designation
 *	no	lock number 0 - 31
 */
EXPORT ER MLockTmo( FastMLock *lock, INT no, TMO tmo )
{
	UINT	ptn = (UINT)(1 << no);
	UINT	flg;
	ER	ercd;

	INC(&lock->wai);
	for ( ;; ) {
		if ( !BTS(&lock->flg, no) ) {
			ercd = E_OK;
			break;
		}

		ercd = tk_wai_flg(lock->id, ptn, TWF_ORW|TWF_BITCLR, &flg, tmo);
		if ( ercd < E_OK ) {
			break;
		}
	}
	DEC(&lock->wai);

	return ercd;
}
EXPORT ER MLockTmo_u( FastMLock *lock, INT no, TMO_U tmo )
{
	UINT	ptn = (UINT)(1 << no);
	UINT	flg;
	ER	ercd;

	INC(&lock->wai);
	for ( ;; ) {
		if ( !BTS(&lock->flg, no) ) {
			ercd = E_OK;
			break;
		}

		ercd = tk_wai_flg_u(lock->id, ptn, TWF_ORW|TWF_BITCLR, &flg, tmo);
		if ( ercd < E_OK ) {
			break;
		}
	}
	DEC(&lock->wai);

	return ercd;
}

/*
 * Lock
 *	no	Lock number 0 - 31
 */
EXPORT ER MLock( FastMLock *lock, INT no )
{
	return MLockTmo(lock, no, TMO_FEVR);
}

/*
 * Lock release
 *	no	Lock number 0 - 31
 */
EXPORT ER MUnlock( FastMLock *lock, INT no )
{
	UINT	ptn = (UINT)(1 << no);
	ER	ercd;

	BR(&lock->flg, no);
	ercd = ( lock->wai == 0 )? E_OK: tk_set_flg(lock->id, ptn);

	return ercd;
}

/*
 * Create multi-lock
 */
EXPORT ER CreateMLock( FastMLock *lock, CONST UB *name )
{
	T_CFLG	cflg;
	ER	ercd;

	if ( name == NULL ) {
		cflg.exinf = NULL;
	} else {
		strncpy((char*)&cflg.exinf, (char*)name, sizeof(cflg.exinf));
	}
	cflg.flgatr  = TA_TPRI | TA_WMUL | TA_NODISWAI;
	cflg.iflgptn = 0;

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
EXPORT ER DeleteMLock( FastMLock *lock )
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
