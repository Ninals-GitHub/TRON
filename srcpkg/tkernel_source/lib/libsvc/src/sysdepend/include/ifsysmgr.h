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
 *	Extended SVC parameter packet
 *
 */

#include <basic.h>
#include <tk/sysmgr.h>
#include <sys/str_align.h>
#include <sys/svc/fnsysmgr.h>

typedef struct {
	CONST UB *name;	_align64
	INT *val;	_align64
	INT max;	_align64
} SYSTEM_TK_GET_CFN_PARA;

typedef struct {
	CONST UB *name;	_align64
	UB *buf;	_align64
	INT max;	_align64
} SYSTEM_TK_GET_CFS_PARA;

typedef struct {
	void **addr;	_align64
	INT nblk;	_align64
	UINT attr;	_align64
} SYSTEM_TK_GET_SMB_PARA;

typedef struct {
	void *addr;	_align64
} SYSTEM_TK_REL_SMB_PARA;

typedef struct {
	T_RSMB *pk_rsmb;	_align64
} SYSTEM_TK_REF_SMB_PARA;

typedef struct {
	CommArea **area;	_align64
} SYSTEM__GETKERNELCOMMONAREA_PARA;

typedef struct {
	const char *string;	_align64
	int len;	_align64
} SYSTEM__SYSLOG_SEND_PARA;

