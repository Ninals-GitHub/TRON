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
#include <sys/segment.h>
#include <sys/str_align.h>
#include <sys/svc/fnsegment.h>

typedef struct {
	CONST void *laddr;	_align64
	INT len;	_align64
} SEG_LOCKSPACE_PARA;

typedef struct {
	CONST void *laddr;	_align64
	INT len;	_align64
} SEG_UNLOCKSPACE_PARA;

typedef struct {
	CONST void *laddr;	_align64
	INT len;	_align64
	void **paddr;	_align64
} SEG_CNVPHYSICALADDR_PARA;

typedef struct {
	CONST void *laddr;	_align64
	INT len;	_align64
	UINT mode;	_align64
	UINT env;	_align64
} SEG_CHKSPACE_PARA;

typedef struct {
	CONST TC *str;	_align64
	INT max;	_align64
	UINT mode;	_align64
	UINT env;	_align64
} SEG_CHKSPACETSTR_PARA;

typedef struct {
	CONST UB *str;	_align64
	INT max;	_align64
	UINT mode;	_align64
	UINT env;	_align64
} SEG_CHKSPACEBSTR_PARA;

typedef struct {
	CONST void *laddr;	_align64
	INT len;	_align64
	UINT mode;	_align64
	UINT env;	_align64
	INT lsid;	_align64
} SEG_CHKSPACELEN_PARA;

typedef struct {
	void *laddr;	_align64
	void *buf;	_align64
	INT len;	_align64
	INT lsid;	_align64
} SEG_READMEMSPACE_PARA;

typedef struct {
	void *laddr;	_align64
	void *buf;	_align64
	INT len;	_align64
	INT lsid;	_align64
} SEG_WRITEMEMSPACE_PARA;

typedef struct {
	void *laddr;	_align64
	INT len;	_align64
	_pad_b(24)
	UB data;	_align64
	_pad_l(24)
	INT lsid;	_align64
} SEG_SETMEMSPACEB_PARA;

typedef struct {
	void *laddr;	_align64
	INT len;	_align64
	UINT mode;	_align64
} SEG_FLUSHMEMCACHE_PARA;

typedef struct {
	void *addr;	_align64
	INT len;	_align64
	UINT mode;	_align64
} SEG_SETCACHEMODE_PARA;

typedef struct {
	void *addr;	_align64
	INT len;	_align64
	UINT mode;	_align64
} SEG_CONTROLCACHE_PARA;

typedef struct {
	CONST void *addr;	_align64
	INT len;	_align64
	T_SPINFO *pk_spinfo;	_align64
} SEG_GETSPACEINFO_PARA;

typedef struct {
	CONST void *addr;	_align64
	INT len;	_align64
	UINT mode;	_align64
} SEG_SETMEMORYACCESS_PARA;

typedef struct {
	CONST void *paddr;	_align64
	INT len;	_align64
	UINT attr;	_align64
	void **laddr;	_align64
} SEG_MAPMEMORY_PARA;

typedef struct {
	CONST void *laddr;	_align64
} SEG_UNMAPMEMORY_PARA;

typedef struct {
	void *laddr;	_align64
	INT npage;	_align64
	INT lsid;	_align64
	UINT pte;	_align64
} SEG_MAKESPACE_PARA;

typedef struct {
	void *laddr;	_align64
	INT npage;	_align64
	INT lsid;	_align64
} SEG_UNMAKESPACE_PARA;

typedef struct {
	void *laddr;	_align64
	INT npage;	_align64
	INT lsid;	_align64
	UINT pte;	_align64
} SEG_CHANGESPACE_PARA;

