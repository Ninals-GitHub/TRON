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
 *	h8io.h	H8 sub micro computer IO interface definition for T-Engine/SH
 */

#ifndef	__DEVICE_H8IO_H__
#define	__DEVICE_H8IO_H__

#include <basic.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Definition for interface library automatic creation (mkiflib)
 */
/*** DEFINE_IFLIB
[INCLUDE FILE]
<device/h8io.h>

[PREFIX]
H8IO
***/

/*
 *   H8 sub micro computer IO interface
 */
/* [BEGIN SYSCALLS] */
IMPORT INT	H8Read(W reg, W len);
IMPORT ER	H8Write(W reg, W len, W dat);
IMPORT ER	H8Reset(void);
IMPORT ER	H8Mread(W reg, UB *dat, W len);
IMPORT ER	H8Mwrite(W reg, UB *dat, W len);
/* [END SYSCALLS] */

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_H8IO_H__ */
