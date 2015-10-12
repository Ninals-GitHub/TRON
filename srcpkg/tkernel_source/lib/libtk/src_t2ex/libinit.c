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
 *	@(#)libinit.c (libtk)
 *
 *	libker library initialization
 *
 *	_InitLibtk() is always linked as it is called from the
 *	startup part.
 *	Note that adding too many processing can make the program
 *	quite large.
 */

#include "libtk.h"
#include <sys/util.h>
#include <sys/memalloc.h>
#include <t2ex/util.h>

EXPORT MACB		_Kmacb;		/* Kmalloc management information */
EXPORT MACB		_Vmacb;		/* Vmalloc management information */
IMPORT MACB		_Smacb;		/* Smalloc management information */
#ifdef T2EX_MM
IMPORT FastULock	_SmacbLock;	/* Smalloc lock */
#endif

#ifdef T2EX
EXPORT	INT	t2ex_call_limit;
#endif
LOCAL	BOOL	libtk_init_done = FALSE;

/*
 * Library initialization
 */
EXPORT void _InitLibtk( void )
{
	INT	rng;

	if ( libtk_init_done ) {
		return;  /* Initialized */
	}
	/* Kernel utility initialization */
	KnlInit();

#ifdef T2EX
	/* T2EX API protected level */
	if ( tk_get_cfn("TSVCLimit", &t2ex_call_limit, 1) < 1 ) {
		t2ex_call_limit = 2;
	}

	/* Kmalloc/Vmalloc memory protection level */
	if ( tk_get_cfn("TKmallocLvl", &rng, 1) < 1 ) {
		rng = t2ex_call_limit;
	}
#else
	/* Lowest protection level where system calls can be issued */
	if ( tk_get_cfn("TSVCLimit", &rng, 1) < 1 ) {
		rng = 2;
	}
#endif
	rng <<= 8;

	/* Create exclusive control lock for library sharing */
	_init_liblock();
#ifdef T2EX_MM
	CreateULock(&_SmacbLock, "ultk");
#endif

	/* malloc initialization */
	_tkm_init((UINT)rng, &_Kmacb);			/* Kmalloc init */
	_tkm_init((UINT)rng|TA_NORESIDENT, &_Vmacb);	/* Vmalloc init */
	_tkm_init(TA_RNG3|TA_NORESIDENT, &_Smacb);	/* Smalloc init */

	libtk_init_done = TRUE;  /* Initialization complete */
}

/*
 * Library finalization
 */
EXPORT void _FinishLibtk( void )
{
	if ( !libtk_init_done ) {
		return;
	}

	DeleteULock(&_SmacbLock);
	_delete_liblock();
}
