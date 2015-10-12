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
 *	@(#)memcheck.c (libtk)
 *
 *	Memory allocation library
 *
 *	memalloc.c test routine
 *	* Must be reentrant *
 */

#include "mem.h"
#include <sys/syslog.h>

/*
 * Checks for errors in memory allocation information. When mode < 0,
 * dumps the usage status. When ptr != NULL, checks to see that
 * memory allocation corresponds properly with ptr allocated blocks.
 * If so, returns True.
 */
LOCAL BOOL chkalloc( void *ptr, int mode, MACB *macb )
{
	QUEUE	*aq, *nq;
	size_t	usesz = 0, fresz = 0, sz;
	int	usebk = 0, frebk = 0, npage = 0;
	BOOL	newpg, ptr_ok;

	/* Checks each area in turn */
	newpg = TRUE;
	ptr_ok = ( ptr == NULL )? TRUE: FALSE;
	for ( aq = macb->areaque.next; aq != &macb->areaque; aq = aq->next ) {

		if ( newpg && !chkAreaFlag(aq, AREA_TOP) ) {
			goto err_found;
		}

		if ( chkAreaFlag(aq, AREA_END) ) {
			if ( newpg ) {
				goto err_found;
			}
			newpg = TRUE;
			fresz += sizeof(QUEUE);
			npage++;
			continue;
		}
		newpg = FALSE;

		nq = aq->next;
		if ( Mask(aq->next) != nq || nq <= aq || Mask(nq->prev) != aq ) {
			goto err_found;
		}
		sz = (size_t)((VB*)nq - (VB*)aq);
		if ( sz < sizeof(QUEUE)*3 ) {
			goto err_found;
		}

		if ( chkAreaFlag(aq, AREA_USE) ) {
			usesz += sz;
			++usebk;
			if ( ptr == (void*)(aq+1) ) {
				ptr_ok = TRUE;
			}
			if ( mode < -1 ) {
				syslog(LOG_NOTICE, "malloc ptr: 0x%08x [%d B]",
							aq+1, AreaSize(aq));
			}
		} else {
			fresz += sz;
			++frebk;
		}
	}
	if ( !newpg ) {
		goto err_found;
	}

	if ( !ptr_ok ) {
		syslog(LOG_ERR, "MALLOC: illegal ptr: 0x%08x", ptr);
		return FALSE;
	}

	if ( mode < 0 ) {
		syslog(LOG_NOTICE,
		"MALLOC: %d pages, used: %d [%d blks] free: %d [%d blks]",
		npage, usesz, usebk, fresz, frebk);
	}

	return TRUE;

err_found:
	syslog(LOG_ERR, "MALLOC: block corrupted at 0x%08x", aq);
	return FALSE;
}

/* ------------------------------------------------------------------------ */

/*
 * Test
 *	mode =	0	Normal mode (default)
 *		1	Debug mode
 *		-1	Usage status dump
 *		-2	Detailed usage status dump
 */
EXPORT void  _mem_malloctest( int mode, MACB *_macb )
{
	MACB	*macb = AlignMACB(_macb);

	if ( mode >= 0 ) {
		/* Change test mode */
		macb->testmode = mode;
		_mem_chkalloc = chkalloc;
	} else {
		/* Dump usage status */
		chkalloc(NULL, mode, macb);
	}
}

/*
 * Memory allocation error check
 *	If ptr != NULL, performs a memory allocation error check,
 *	including a check to see whether memory allocation corresponds
 *	properly with ptr allocated blocks.
 *	If ptr == NULL, performs the memory allocation error check
 *	only and returns TRUE if all is OK.
 */
EXPORT BOOL _mem_malloccheck( void *ptr, MACB *_macb )
{
	MACB	*macb = AlignMACB(_macb);

	return chkalloc(ptr, 0, macb);
}
