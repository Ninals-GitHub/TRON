/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2013/03/08.
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
 *	pminfo.c
 *
 *       T2EX: program load functions
 *       program module information
 */

#include <basic.h>
#include <errno.h>
#include <sys/debug.h>
#include "pminfo.h"

EXPORT	ProgInfo*	pmInfoTable = NULL;
EXPORT	INT		pmInfoCount;
LOCAL	QUEUE		pmFreeQue;

/*
 * Initialize control block of program module information
 */
EXPORT ER pmInitInfo( INT n )
{
	ER	er;
	INT	i;

	if ( n < 0 ) {
		er = EX_INVAL;
		goto err_ret;
	}

	/* Allocate control block for program module information */
	pmInfoTable = (ProgInfo*)Kcalloc(n, sizeof(ProgInfo));

	if ( pmInfoTable == NULL ) {
		er = EX_NOMEM;
		goto err_ret;
	}

	/* Initialize queue and count */
	pmInfoCount = n;
	QueInit(&pmFreeQue);

	/* Initialize program module information */
	for ( i = 0; i < n; i++ ) {
		pmInfoTable[i].pmid = (i + 1);
		QueInsert(&(pmInfoTable[i].q), &pmFreeQue);
	}

	return E_OK;

err_ret:
#ifdef DEBUG
	TM_DEBUG_PRINT(("pmInitInfo ercd = %d\n", er));
#endif
	return er;
}

/*
 * Cleanup control block of program module information
 *	Entries in used queue are not unloaded automatically;
 *	thus needed to unload them before calling this function.
 */
EXPORT void pmCleanupInfo()
{
	Kfree(pmInfoTable);
	pmInfoTable = NULL;
}

/*
 * Allocate a free program module information
 */
EXPORT ProgInfo* pmAllocInfo()
{
	ProgInfo*	prog;

	/* Get the first entry in free queue */
	prog = (ProgInfo*)QueRemoveNext(&pmFreeQue);

	if ( prog ) {
		/* Mark the program as used */
		prog->used = TRUE;
	}

	return prog;
}

/*
 * Deallocate program module information
 */
EXPORT void pmFreeInfo( ProgInfo* prog )
{
	/* Append to the end of free queue */
	QueInsert(&(prog->q), &pmFreeQue);

	/* Mark the program as free */
	prog->used = FALSE;
}
