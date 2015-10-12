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
 *	@(#)tkminit.c (libtk)
 *
 *	Memory allocation library
 *	T-Kernel initialization sequence
 *
 *	_tkm_init is always linked as it is called from the startup
 *	sequence part.
 *	Note that adding too many processing can make the program
 *	size quite large, irrespective of whether malloc is required
 *	or not.
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <sys/memalloc.h>

/*
 * Memory allocation
 */
LOCAL void* getblk( INT nblk, UINT mematr )
{
	void*	ptr;
	ER	ercd;

	ercd = tk_get_smb(&ptr, nblk, mematr);
	if ( ercd < E_OK ) {
		return NULL;
	}

	return ptr;
}

/*
 * Memory release
 */
LOCAL void relblk( void *ptr )
{
	tk_rel_smb(ptr);
}

/*
 * MACB initialization
 */
EXPORT ER _tkm_init( UINT mematr, MACB *_macb )
{
	MACB	*macb = AlignMACB(_macb);
	T_RSMB	rsmb;
	ER	ercd;

	/* Initialize memory allocation management information */
	macb->pagesz   = 0;	/* 0 indicates not available for use */
	macb->mematr   = mematr;
	macb->testmode = 0;
	macb->getblk   = getblk;
	macb->relblk   = relblk;
	QueInit(&macb->areaque);
	QueInit(&macb->freeque);

	/* Get memory information */
	ercd = tk_ref_smb(&rsmb);
	if ( ercd < E_OK ) {
		return ercd;
	}
	macb->pagesz = (UINT)rsmb.blksz;

	return E_OK;
}
