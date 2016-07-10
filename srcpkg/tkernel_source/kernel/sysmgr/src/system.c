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
 *	system.c (T-Kernel/SM)
 *	System Information Management Function
 *	System Memory Management Function
 */

#include "sysmgr.h"
#include "syslog.h"
#include <sys/rominfo.h>
#include <sys/imalloc.h>
#include <sys/commarea.h>
#include <sys/svc/ifsysmgr.h>
#include <sys/sysinfo.h>

#define SYSCONF		( SCInfo.sysconf )
#define DEVCONF		( SCInfo.devconf )

/* ------------------------------------------------------------------------ */
/*
 *	System information management function
 */

/*
 * Search 'name' information
 *	If it is not found, return NULL.
 */
LOCAL CONST UB* search_conf( CONST UB *cp, CONST UB *name )
{
	size_t		len = strlen((char*)name);
	CONST UB	*p;

	while ( *cp != '\0' ) {
		if ( *cp == name[0] ) {
			for ( p = cp; *p > ' ' && *p != '#'; ++p ) {
				;
			}
			if ( (size_t)(p - cp) == len && memcmp(cp, name, len) == 0 ) {
				return cp; /* Found */
			}
		}

		/* Next */
		while ( *cp != '\0' && *cp++ != '\n' ) {
			;
		}
	}

	return NULL;
}

/*
 * Skip to the next word
 *	If it is the end, return NULL.
 */
LOCAL CONST UB* skip_next( CONST UB *p )
{
	UB	c;

	/* Skip reading current word */
	while ( (c = *p) > ' ' && c != '#' ) {
		++p;
	}

	/* Skip reading word separators */
	while ( (c = *p) != '\0' ) {
		if ( c == '\n' || c == '#' ) {
			break;
		}
		if ( c > ' ' ) {
			return p;
		}
		p++;
	}

	return NULL;
}

/*
 * Get numeric value information
 */
LOCAL INT getcfn( CONST UB *conf, CONST UB *name, INT *val, INT max )
{
	CONST UB	*p;
	long		v, n;

	p = search_conf(conf, name);
	if ( p == NULL ) {
		return E_NOEXS; /* Not found */
	}

	n = 0;
	while ( (p = skip_next(p)) != NULL ) {
		v = strtol((char*)p, (char**)&p, 0);
		if ( max-- > 0 ) {
			*val++ = v;
		}
		n++;
	}

	return n;
}

/*
 * Get string information
 */
LOCAL INT getcfs( CONST UB *conf, CONST UB *name, UB *bp, INT max )
{
	CONST UB	*p, *sp;
	UB		c;

	p = search_conf(conf, name);
	if ( p == NULL ) {
		return E_NOEXS; /* Not found */
	}

	sp = skip_next(p);
	if ( sp == NULL ) {
		return 0; /* Data is not defined */
	}

	for ( p = sp; (c = *p) != '\0'; ++p ) {
		if ( c == '\n' || c == '#' ) {
			break;
		}

		if ( max-- > 0 ) {
			*bp++ = c;
		}
	}
	if ( max > 0 ) {
		*bp = '\0';
	}

	return p - sp;
}

/*
 * Get system information (numeric values)
 *	Called directly from T-Kernel/OS.
 *	Do not call system call and extension SVC inside because
 *	this function is also called during system start-up.
 */
EXPORT INT _tk_get_cfn( CONST UB *name, INT *val, INT max )
{
	INT	n;

	n = getcfn(SYSCONF, name, val, max);
	if ( n < 0 ) {
		n = getcfn(DEVCONF, name, val, max);
	}
	if ( n < 0 ) {
		goto err_ret;
	}

	return n;

err_ret:
	BMS_DEBUG_PRINT(("_tk_get_cfn ercd = %d\n", n));
	return n;
}

/*
 * Get system information (numeric values)
 */
LOCAL INT __tk_get_cfn( CONST UB *name, INT *val, INT max )
{
	ER	ercd;

	ercd = ChkSpaceBstrR(name, 0);
	if ( ercd < E_OK ) {
		//vd_printf("ChkSpaceBstrR error:%d\n", ercd);
		goto err_ret;
	}
	ercd = ChkSpaceRW(val, (INT)sizeof(INT) * max);
	if ( ercd < E_OK ) {
		//vd_printf("ChkSpaceRW error:%d\n", ercd);
		goto err_ret;
	}

	ercd = _tk_get_cfn(name, val, max);
	if ( ercd < E_OK ) {
		//vd_printf("_tk_get_cfn error:%d\n", ercd);
		goto err_ret;
	}

	return ercd;

err_ret:
	DEBUG_PRINT(("__tk_get_cfn ercd = %d\n", ercd));
	return ercd;
}

/*
 * Get system information (strings)
 *	Called directly from T-Kernel/OS.
 *	Do not call system call and extension SVC inside because
 *	this function is also called during system start-up.
 */
EXPORT INT _tk_get_cfs( CONST UB *name, UB *buf, INT max )
{
	INT	n;

	n = getcfs(SYSCONF, name, buf, max);
	if ( n < 0 ) {
		n = getcfs(DEVCONF, name, buf, max);
	}
	if ( n < 0 ) {
		goto err_ret;
	}

	return n;

err_ret:
	BMS_DEBUG_PRINT(("_tk_get_cfs ercd = %d\n", n));
	return n;
}

/*
 * Get system information (strings)
 */
LOCAL INT __tk_get_cfs( CONST UB *name, UB *buf, INT max )
{
	ER	ercd;

	ercd = ChkSpaceBstrR(name, 0);
	if ( ercd < E_OK ) {
		goto err_ret;
	}
	ercd = ChkSpaceRW(buf, max);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	ercd = _tk_get_cfs(name, buf, max);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return ercd;

err_ret:
	DEBUG_PRINT(("__tk_get_cfs ercd = %d\n", ercd));
	return ercd;
}

/* ------------------------------------------------------------------------ */
/*
 *	System memory management function
 *
 *	Only the extension SVC entry part is processed here, while the
 *	actual memory management functions are provided externally for
 *	virtual memory system and real memory system, respectively.
 *	Memory management functions are provided in the upper level for
 *	virtual memory system, while it is provided in T-Kernel/SM for
 *	real memory system.
 */

#define VALID_MEMATR	(TA_RNG3 | TA_NORESIDENT | TA_NOCACHE)

/*
 * Get system memory
 */
LOCAL ER _tk_get_smb( void **addr, INT nblk, UINT attr )
{
	ER	ercd;

	if ( (nblk <= 0) || ((attr & ~VALID_MEMATR) != 0) ) {
		ercd = E_PAR;
		goto err_ret;
	}
	ercd = ChkSpaceRW(addr, sizeof(void*));
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	*addr = GetSysMemBlk(nblk, attr);
	if ( *addr == NULL ) {
		ercd = E_NOMEM;
		goto err_ret;
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("_tk_get_smb ercd = %d\n", ercd));
	return ercd;
}

/*
 * Free system memory
 */
LOCAL ER _tk_rel_smb( void *addr )
{
	ER	ercd;

	ercd = RelSysMemBlk(addr);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("_tk_rel_smb ercd = %d\n", ercd));
	return ercd;
}

/*
 * Get system memory information
 */
LOCAL ER _tk_ref_smb( T_RSMB *pk_rsmb )
{
	ER	ercd;

	ercd = ChkSpaceRW(pk_rsmb, sizeof(T_RSMB));
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	ercd = RefSysMemInfo(pk_rsmb);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("_tk_ref_smb ercd = %d\n", ercd));
	return ercd;
}

/* ------------------------------------------------------------------------ */
/*
 *	Kernel shared data
 */

LOCAL	CommArea	_CommArea;	/* Kernel shared data area */
IMPORT	CommArea	*__CommArea;	/* libtk */

/*
 * Get Kernel shared data
 */
LOCAL ER __GetKernelCommonArea( CommArea **area )
{
	*area = &_CommArea;

	return E_OK;
}

/*
 * Kernel shared data initial setting
 */
LOCAL void initKernelCommonArea( void )
{
	INT	n, val;

	__CommArea = &_CommArea;

	n = _tk_get_cfn(SCTAG_TEV_FFLOCK, &val, 1);
	_CommArea.tev_fflock = ( n > 0 )? val: 0;
}

/* ------------------------------------------------------------------------ */

/*
 * Extension SVC entry
 */
LOCAL INT sysmgr_svcentry( void *pk_para, FN fncd )
{
	ER	ercd;

	switch ( fncd ) {
		/* Ignore protection level */
	  case SYSTEM_TK_GET_CFN_FN:
	  case SYSTEM_TK_GET_CFS_FN:
	  case SYSTEM__SYSLOG_SEND_FN:
		break;

	  default:
		/* Test call protection level */
		ercd = ChkCallPLevel();
		if ( ercd < E_OK ) {
			goto err_ret;
		}
	}

	switch ( fncd ) {
	  case SYSTEM_TK_GET_CFN_FN:
		{ SYSTEM_TK_GET_CFN_PARA *p = pk_para;
		return __tk_get_cfn(p->name, p->val, p->max); }
	  case SYSTEM_TK_GET_CFS_FN:
		{ SYSTEM_TK_GET_CFS_PARA *p = pk_para;
		return __tk_get_cfs(p->name, p->buf, p->max); }
	  case SYSTEM_TK_GET_SMB_FN:
		{ SYSTEM_TK_GET_SMB_PARA *p = pk_para;
		return _tk_get_smb(p->addr, p->nblk, p->attr); }
	  case SYSTEM_TK_REL_SMB_FN:
		{ SYSTEM_TK_REL_SMB_PARA *p = pk_para;
		return _tk_rel_smb(p->addr); }
	  case SYSTEM_TK_REF_SMB_FN:
		{ SYSTEM_TK_REF_SMB_PARA *p = pk_para;
		return _tk_ref_smb(p->pk_rsmb); }

	  case SYSTEM__GETKERNELCOMMONAREA_FN:
		{ SYSTEM__GETKERNELCOMMONAREA_PARA *p = pk_para;
		return __GetKernelCommonArea(p->area); }
	  case SYSTEM__SYSLOG_SEND_FN:
		{ SYSTEM__SYSLOG_SEND_PARA *p = pk_para;
		return __syslog_send(p->string, p->len); }
	  default:
		ercd = E_RSFN;
	}

err_ret:
	DEBUG_PRINT(("sysmgr_svcentry ercd = %d\n", ercd));
	return ercd;
}

/*
 * Initialization of system management
 */
EXPORT ER initialize_sysmgr( void )
{
	T_DSSY	dssy;
	ER	ercd;

	/* Kernel shared data area setting */
	initKernelCommonArea();

	/* syslog initialization sequence */
	ercd = initialize_syslog();
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	/* subsystem registration */
	dssy.ssyatr    = TA_NULL;
	dssy.ssypri    = SYSTEM_PRI;
	dssy.svchdr    = (FP)&sysmgr_svcentry;
	dssy.breakfn   = NULL;
	dssy.startupfn = NULL;
	dssy.cleanupfn = NULL;
	dssy.eventfn   = NULL;
	dssy.resblksz  = 0;
	ercd = tk_def_ssy(SYSTEM_SVC, &dssy);
	if ( ercd < E_OK ) {
		goto err_ret;
	}

	return E_OK;

err_ret:
	DEBUG_PRINT(("initialize_sysmgr ercd = %d\n", ercd));
	return ercd;
}

/*
 * Finalization sequence of system management
 */
EXPORT ER finish_sysmgr( void )
{
	ER	ercd;

	/* Unregister subsystem */
	ercd = tk_def_ssy(SYSTEM_SVC, NULL);
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("1. finish_sysmgr -> tk_def_ssy ercd = %d\n", ercd));
	}
#endif

	/* syslog finalization sequence */
	ercd = finish_syslog();
#ifdef DEBUG
	if ( ercd < E_OK ) {
		DEBUG_PRINT(("2. finish_sysmgr -> finish_syslog ercd = %d\n", ercd));
	}
#endif

	return ercd;
}
