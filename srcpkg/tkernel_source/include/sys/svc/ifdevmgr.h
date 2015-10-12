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
#include <tk/devmgr.h>
#include <sys/str_align.h>
#include <sys/svc/fndevmgr.h>

typedef struct {
	CONST UB *devnm;	_align64
	UINT omode;	_align64
} DEVICE_TK_OPN_DEV_PARA;

typedef struct {
	ID dd;	_align64
	UINT option;	_align64
} DEVICE_TK_CLS_DEV_PARA;

typedef struct {
	ID dd;	_align64
	W start;	_align64
	void *buf;	_align64
	W size;	_align64
	TMO tmout;	_align64
} DEVICE_TK_REA_DEV_PARA;

typedef struct {
	ID dd;	_align64
	W start;	_align64
	void *buf;	_align64
	W size;	_align64
	W *asize;	_align64
} DEVICE_TK_SREA_DEV_PARA;

typedef struct {
	ID dd;	_align64
	W start;	_align64
	CONST void *buf;	_align64
	W size;	_align64
	TMO tmout;	_align64
} DEVICE_TK_WRI_DEV_PARA;

typedef struct {
	ID dd;	_align64
	W start;	_align64
	CONST void *buf;	_align64
	W size;	_align64
	W *asize;	_align64
} DEVICE_TK_SWRI_DEV_PARA;

typedef struct {
	ID dd;	_align64
	ID reqid;	_align64
	W *asize;	_align64
	ER *ioer;	_align64
	TMO tmout;	_align64
} DEVICE_TK_WAI_DEV_PARA;

typedef struct {
	UINT mode;	_align64
} DEVICE_TK_SUS_DEV_PARA;

typedef struct {
	ID devid;	_align64
	UB *devnm;	_align64
} DEVICE_TK_GET_DEV_PARA;

typedef struct {
	CONST UB *devnm;	_align64
	T_RDEV *rdev;	_align64
} DEVICE_TK_REF_DEV_PARA;

typedef struct {
	ID dd;	_align64
	T_RDEV *rdev;	_align64
} DEVICE_TK_OREF_DEV_PARA;

typedef struct {
	T_LDEV *ldev;	_align64
	INT start;	_align64
	INT ndev;	_align64
} DEVICE_TK_LST_DEV_PARA;

typedef struct {
	ID devid;	_align64
	INT evttyp;	_align64
	void *evtinf;	_align64
} DEVICE_TK_EVT_DEV_PARA;

typedef struct {
	ID dd;	_align64
	D start_d;	_align64
	void *buf;	_align64
	W size;	_align64
	TMO_U tmout_u;	_align64
} DEVICE_TK_REA_DEV_DU_PARA;

typedef struct {
	ID dd;	_align64
	D start_d;	_align64
	void *buf;	_align64
	W size;	_align64
	W *asize;	_align64
} DEVICE_TK_SREA_DEV_D_PARA;

typedef struct {
	ID dd;	_align64
	D start_d;	_align64
	CONST void *buf;	_align64
	W size;	_align64
	TMO_U tmout_u;	_align64
} DEVICE_TK_WRI_DEV_DU_PARA;

typedef struct {
	ID dd;	_align64
	D start_d;	_align64
	CONST void *buf;	_align64
	W size;	_align64
	W *asize;	_align64
} DEVICE_TK_SWRI_DEV_D_PARA;

typedef struct {
	ID dd;	_align64
	ID reqid;	_align64
	W *asize;	_align64
	ER *ioer;	_align64
	TMO_U tmout_u;	_align64
} DEVICE_TK_WAI_DEV_U_PARA;

typedef struct {
	CONST UB *devnm;	_align64
	CONST T_DDEV *ddev;	_align64
	T_IDEV *idev;	_align64
} DEVICE_TK_DEF_DEV_PARA;

typedef struct {
	T_IDEV *idev;	_align64
} DEVICE_TK_REF_IDV_PARA;

