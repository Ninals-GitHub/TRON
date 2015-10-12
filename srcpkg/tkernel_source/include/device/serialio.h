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
 *	serialio.h	Low-level serial I/O driver definition
 */

#ifndef	__DEVICE_SERIALIO_H__
#define	__DEVICE_SERIALIO_H__

#include <basic.h>
#include <tk/typedef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	"serial_ctl()" request code
 */
typedef enum {
	RS_ABORT	= 0,
	RS_SUSPEND	= -200,
	RS_RESUME	= -201,
	RS_RCVBUFSZ	= -202,
	RS_LINECTL	= -203,
	RS_EXTFUNC	= -9999		/* Unsupported */
} SerialControlNo;

/*  Control line ON/OFF parameter */
#define	RSCTL_DTR	0x00000001	/* "DTR" signal */
#define	RSCTL_RTS	0x00000002	/*  "RTS" signal */
#define	RSCTL_SET	0x00000000	/* Set the all signals */
#define	RSCTL_ON	0xc0000000	/* Turn ON the specified signal */
#define	RSCTL_OFF	0x80000000	/* Turn OFF the specified signal */

/*
 * Definition for interface library automatic creation (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<device/serialio.h>

[PREFIX]
SERIAL
***/

/* [BEGIN SYSCALLS] */
IMPORT ER serial_in(W port, B* buf, W len, W *alen, W tmout);
IMPORT ER serial_out(W port, B* buf, W len, W *alen, W tmout);
IMPORT ER serial_ctl(W port, W kind, UW *arg);
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_SERIALIO_H__ */
