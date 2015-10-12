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
 *	check.h (T-Kernel/OS)
 *	Macro for Error Check
 */

#ifndef _CHECK_
#define _CHECK_

/*
 * Check object ID range (E_ID)
 */
#if CHK_ID
#define CHECK_TSKID(tskid) {					\
	if (!in_indp() && ((tskid) == TSK_SELF)) {		\
		return E_OBJ;					\
	} else if (!CHK_TSKID(tskid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_TSKID_SELF(tskid) {				\
	if ( !( (!in_indp() && (tskid) == TSK_SELF)		\
		|| CHK_TSKID(tskid) ) ) {			\
		return E_ID;					\
	}							\
}
#define CHECK_SEMID(semid) {					\
	if (!CHK_SEMID(semid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_FLGID(flgid) {					\
	if (!CHK_FLGID(flgid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_MBXID(mbxid) {					\
	if (!CHK_MBXID(mbxid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_MBFID(mbfid) {					\
	if (!CHK_MBFID(mbfid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_PORID(porid) {					\
	if (!CHK_PORID(porid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_MTXID(pisid) {					\
	if (!CHK_MTXID(pisid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_MPLID(mplid) {					\
	if (!CHK_MPLID(mplid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_MPFID(mpfid) {					\
	if (!CHK_MPFID(mpfid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_CYCID(cycid) {					\
	if (!CHK_CYCID(cycid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_ALMID(almid) {					\
	if (!CHK_ALMID(almid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_RESID(resid) {					\
	if (!CHK_RESID(resid)) {				\
		return E_ID;					\
	}							\
}
#define CHECK_RESID_ANY(resid) {				\
	if (!(CHK_RESID(resid) || (resid) == 0)) {		\
		return E_ID;					\
	}							\
}
#define CHECK_SSYID(ssid) {					\
	if (!CHK_SSYID(ssid)) {					\
		return E_ID;					\
	}							\
}
#define CHECK_SSYID_ALL(ssid) {					\
	if (!(CHK_SSYID(ssid) || (ssid) == 0)) {		\
		return E_ID;					\
	}							\
}
#else /* CHK_ID */
#define CHECK_TSKID(tskid)
#define CHECK_TSKID_SELF(tskid)
#define CHECK_SEMID(semid)
#define CHECK_FLGID(flgid)
#define CHECK_MBXID(mbxid)
#define CHECK_MBFID(mbfid)
#define CHECK_PORID(porid)
#define CHECK_MTXID(pisid)
#define CHECK_MPLID(mplid)
#define CHECK_MPFID(mpfid)
#define CHECK_CYCID(cycid)
#define CHECK_ALMID(almid)
#define CHECK_RESID(resid)
#define CHECK_RESID_ANY(resid)
#define CHECK_SSYID(ssid)
#define CHECK_SSYID_ALL(ssid)
#endif /* CHK_ID */

/*
 * Check whether its own task is specified (E_OBJ)
 */
#if CHK_SELF
#define CHECK_NONSELF(tskid) {					\
	if (!in_indp() && (tskid) == ctxtsk->tskid) {		\
		return E_OBJ;					\
	}							\
}
#else /* CHK_SELF */
#define CHECK_NONSELF(tskid)
#endif /* CHK_SELF */

/*
 * Check task priority value (E_PAR)
 */
#if CHK_PAR
#define CHECK_PRI(pri) {					\
	if (!CHK_PRI(pri)) {					\
		return E_PAR;					\
	}							\
}
#define CHECK_PRI_INI(pri) {					\
	if ((pri) != TPRI_INI && !CHK_PRI(pri)) {		\
		return E_PAR;					\
	}							\
}
#define CHECK_PRI_RUN(pri) {					\
	if ((pri) != TPRI_RUN && !CHK_PRI(pri)) {		\
		return E_PAR;					\
	}							\
}
#else /* CHK_PAR */
#define CHECK_PRI(pri)
#define CHECK_PRI_INI(pri)
#define CHECK_PRI_RUN(pri)
#endif /* CHK_PAR */

/*
 * Check subsystem priority value (E_PAR)
 */
#if CHK_PAR
#define CHECK_SSYPRI(ssypri) {					\
	if (!CHK_SSYPRI(ssypri)) {				\
		return E_PAR;					\
	}							\
}
#else /* CHK_PAR */
#define CHECK_SSYPRI(ssypri)
#endif /* CHK_PAR */

/*
 * Check timeout specification value (E_PAR)
 */
#if CHK_PAR
#define CHECK_TMOUT(tmout) {					\
	if (!((tmout) >= TMO_FEVR)) {				\
		return E_PAR;					\
	}							\
}
#else /* CHK_PAR */
#define CHECK_TMOUT(tmout)
#endif /* CHK_PAR */

/*
 * Check other parameter errors (E_PAR)
 */
#if CHK_PAR
#define CHECK_PAR(exp) {					\
	if (!(exp)) {						\
		return E_PAR;					\
	}							\
}
#else /* CHK_PAR */
#define CHECK_PAR(exp)
#endif /* CHK_PAR */

/*
 * Check reservation attribute error (E_RSATR)
 */
#if CHK_RSATR
#define CHECK_RSATR(atr, maxatr) {				\
	if ((atr) & ~(maxatr)) {				\
	        return E_RSATR;					\
	}							\
}
#else /* CHK_RSATR */
#define CHECK_RSATR(atr, maxatr)
#endif /* CHK_RSATR */

/*
 * Check unsupported function (E_NOSPT)
 */
#if CHK_NOSPT
#define CHECK_NOSPT(exp) {					\
	if (!(exp)) {						\
		return E_NOSPT;					\
	}							\
}
#else /* CHK_NOSPT */
#define CHECK_NOSPT(exp)
#endif /* CHK_NOSPT */

/*
 * Check whether task-independent part is running (E_CTX)
 */
#if CHK_CTX
#define CHECK_INTSK() {						\
	if (in_indp()) {					\
		return E_CTX;					\
	}							\
}
#else /* CHK_CTX */
#define CHECK_INTSK()
#endif /* CHK_CTX */

/*
 * Check whether dispatch is in disabled state (E_CTX)
 */
#if CHK_CTX
#define CHECK_DISPATCH() {					\
	if (in_ddsp()) {					\
		return E_CTX;					\
	}							\
}
#define CHECK_DISPATCH_POL(tmout) {				\
	if ((tmout) != TMO_POL && in_ddsp()) {			\
		return E_CTX;					\
	}							\
}
#else /* CHK_CTX */
#define CHECK_DISPATCH()
#define CHECK_DISPATCH_POL(tmout)
#endif /* CHK_CTX */

/*
 * Check other context errors (E_CTX)
 */
#if CHK_CTX
#define CHECK_CTX(exp) {					\
	if (!(exp)) {						\
		return E_CTX;					\
	}							\
}
#else /* CHK_CTX */
#define CHECK_CTX(exp)
#endif /* CHK_CTX */

/*
 * Check non-existence co-processor error (E_NOCOP)
 */
#if CHK_NOCOP
#define CHECK_NOCOP(atr) {					\
	if ( ((atr) & (TA_COP0|TA_COP1|TA_COP2|TA_COP3)) != 0	\
	  && ((atr) & available_cop) == 0 ) {			\
		return E_NOCOP;					\
	}							\
}
#else
#define CHECK_NOCOP(atr)
#endif

#endif /* _CHECK_ */
