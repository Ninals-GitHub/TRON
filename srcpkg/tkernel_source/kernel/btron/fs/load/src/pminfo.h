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
 *	pminfo.h
 *
 *       T2EX: program load functions
 *       program module information
 */

#ifndef _T2EX_LOAD_PMINFO_
#define _T2EX_LOAD_PMINFO_

#include <basic.h>
#include <sys/queue.h>
#include <tk/tkernel.h>
#include <t2ex/load.h>

/*
 * Program load manager lock
 */
IMPORT	FastLock	pmLock;

#define	LockPM()	Lock(&pmLock);
#define	UnlockPM()	Unlock(&pmLock);

/*
 * Program module information
 */
typedef struct {
	QUEUE	q;
	ID	pmid;		/* program ID */
	BOOL	used;
	UINT	attr;
	void*	loadadr;	/* load address (top) */
	UINT	loadsz;		/* load program size (including BSS) */
	FP	entry;		/* program entry address */
	FP	modentry;	/* user program entry address (NULL in case of system program) */
} ProgInfo;

IMPORT	ProgInfo*	pmInfoTable;
IMPORT	INT		pmInfoCount;

Inline	ProgInfo*	pmGetInfo( ID id )
{
	if ( id <= 0 || id > pmInfoCount )
		return NULL;
	return &(pmInfoTable[id - 1]);
}

IMPORT ER pmInitInfo( INT n );
IMPORT void pmCleanupInfo();
IMPORT ProgInfo* pmAllocInfo();
IMPORT void pmFreeInfo( ProgInfo* prog );

#endif /* _T2EX_LOAD_PMINFO_ */
