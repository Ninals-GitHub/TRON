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
 *	consmlock.c	Console/Low-level serial I/O driver
 *
 *	Multilocking for high-speed exclusive control : system-independent
 *	* Modify "fastmlock.c (libtk)" for "console/serial" driver use
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/util.h>
#include <libstr.h>

/* ------------------------------------------------------------------------ */
/*
 *	System-dependent code
 *	void INC( INT *val )		Increment
 *	void DEC( INT *val )		Decrement
 *	BOOL BTS( UINT *val, INT no )	Bit test & Set
 *	void BR( UINT *val, INT no )	Bit reset
 *
 *	These require the exclusive behavior.
 */

Inline	void	INC( INT *val )
{
	UINT	imask;

	DI(imask);
	(*val)++;
	EI(imask);
}

Inline	void	DEC( INT *val )
{
	UINT	imask;

	DI(imask);
	(*val)--;
	EI(imask);
}

Inline	BOOL	BTS( UINT *val, INT no )
{
	UINT	imask;
	UINT	b;
	UINT	bm = 1 << no;

	DI(imask);
	b = *val & bm;
	*val |= bm;
	EI(imask);
	return b;
}

Inline	void	BR( UINT *val, INT no )
{
	UINT	imask;

	DI(imask);
	*val &= ~(1 << no);
	EI(imask);
}

/* ------------------------------------------------------------------------ */

/*
 *	Lock with specification of waiting time
 *	no	Lock number 0-31
 */
LOCAL	ER	consMLockTmo( FastMLock *lock, INT no, TMO tmo )
{
	UINT	ptn = 1 << no;
	UINT	flg;
	ER	err;

	INC(&lock->wai);
	for ( ;; ) {
		if ( !BTS(&lock->flg, no) ) {
			err = E_OK;
			break;
		}
		err = tk_wai_flg(lock->id, ptn, TWF_ORW|TWF_BITCLR, &flg, tmo);
		if ( err < E_OK ) break;
	}
	DEC(&lock->wai);

	return err;
}

/*
 *	Lock
 *	no	Lock number 0-31
 */
EXPORT	ER	consMLock( FastMLock *lock, INT no )
{
	return consMLockTmo(lock, no, TMO_FEVR);
}

/*
 *	Release the lock
 *	no	Lock number 0-31
 */
EXPORT	ER	consMUnlock( FastMLock *lock, INT no )
{
	UINT	ptn = 1 << no;
	ER	err;

	BR(&lock->flg, no);
	err = ( lock->wai == 0 )? E_OK: tk_set_flg(lock->id, ptn);

	return err;
}

/*
 *	Create the multilocking
 */
EXPORT	ER	consCreateMLock( FastMLock *lock, UB *name )
{
	T_CFLG	cflg;
	ER	err;

	if ( name == NULL ) {
		cflg.exinf = NULL;
	} else {
		strncpy((B*)&cflg.exinf, name, sizeof(cflg.exinf));
	}
	cflg.flgatr  = TA_TFIFO | TA_WMUL;
	cflg.iflgptn = 0;

	lock->id = err = tk_cre_flg(&cflg);
	if ( err < E_OK ) return err;

	lock->wai = 0;
	lock->flg = 0;

	return lock->id;
}

/*
 *	Delete the multilocking
 */
EXPORT	ER	consDeleteMLock( FastMLock *lock )
{
	ER	err;

	if ( lock->id <= 0 ) return E_PAR;

	err = tk_del_flg(lock->id);
	if ( err < E_OK ) return err;

	lock->id = 0;

	return E_OK;
}
