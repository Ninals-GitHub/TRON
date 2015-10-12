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
 *	progload.c
 *
 *       T2EX: program load functions
 *       program load operations
 */

#include <basic.h>
#include <errno.h>
#include <sys/debug.h>
#include <tk/tkernel.h>
#include <t2ex/load.h>
#include "service.h"
#include "pminfo.h"
#include "source.h"
#include "elf.h"

IMPORT ER elf_load( ProgInfo *pg, LoadSource *ldr, UINT attr, Elf32_Ehdr *hdr );
IMPORT ER pmCallStartup( INT ac, UB *av[], FP entry );

/*
 * Open program load source
 */
LOCAL ER pmOpenSource( LoadSource* source, const struct pm* prog )
{
	switch (prog->pmtype) {
#if USE_FILELOADER
	case PM_FILE:
		return openFileSource(source, prog);
#endif
#if USE_MEMLOADER
	case PM_PTR:
		return openMemSource(source, prog);
#endif
	default:
		return EX_INVAL;
	}
}


/*
 * Object file header
 */
typedef struct {
	Elf32_Ehdr	elf;
} ObjectHeader;


/*
 * Load program module
 */
LOCAL ID pmLoadProg( const struct pm* prog, UINT attr, FP* entry )
{
	LoadSource	source;
	ProgInfo	*pginfo;
	ObjectHeader	hdr;
	ER		er;

	/* Allocate control block*/
	pginfo = pmAllocInfo();
	if ( pginfo == NULL ) { er = E_LIMIT; goto err_ret0; }
	pginfo->attr = attr;

	/* Open program load source */
	er = pmOpenSource(&source, prog);
	if ( er < E_OK ) goto err_ret1;

	/* Read object header */
	er = source.read(&source, 0, &hdr, sizeof(ObjectHeader));
	if ( er < E_OK ) goto err_ret2;
	if ( er < sizeof(ObjectHeader) ) { er = EX_NOEXEC; goto err_ret2; }

	/* Load object */
	if ( hdr.elf.e_ident[EI_MAG0] == ELFMAG0
	     && hdr.elf.e_ident[EI_MAG1] == ELFMAG1
	     && hdr.elf.e_ident[EI_MAG2] == ELFMAG2
	     && hdr.elf.e_ident[EI_MAG3] == ELFMAG3 ) {
		/* ELF format */
		er = elf_load(pginfo, &source, attr, &(hdr.elf));
	}
	else {
		/* Unsupported format */
		er = EX_NOEXEC;
		goto err_ret2;
	}
	if ( er < E_OK ) goto err_ret2;

	source.close(&source);

	/* Return entry point and program ID */
	*entry = pginfo->entry;
	er = pginfo->pmid;

	return er;

err_ret2:
	source.close(&source);
err_ret1:
	pmFreeInfo(pginfo);
err_ret0:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_pm_load ercd = %d\n", er));
#endif
	return er;
}


/*
 * Unload program
 */
Inline ER pmUnloadProg( ProgInfo* pginfo )
{
	ER	er;

	er = tk_rel_smb(pginfo->loadadr);
	pmFreeInfo(pginfo);

	return er;
}


/*
 * Load program module
 */
EXPORT ID _pm_load( const struct pm* prog, UINT attr, pm_entry_t** entry )
{
	ER	er;
	ID	pmid;
	FP	realEntry;
	ProgInfo*	pginfo;

	/* Check parameters */
	er = ChkSpaceR(prog, sizeof(*prog));
	if ( er < E_OK ) goto err_ret0;

	if ( entry ) {
		er = ChkSpaceR(entry, sizeof(*entry));
		if ( er < E_OK ) goto err_ret0;
	}

	LockPM();

	/* Load program module */
	er = pmLoadProg(prog, attr, &realEntry);
	if ( er < E_OK ) goto err_ret1;
	pmid = er;
	pginfo = pmGetInfo(pmid);

	/* Execute startup */
	er = pmCallStartup(0, (UB**)&(pginfo->modentry), realEntry);
	if ( er < E_OK ) goto err_ret2;
	if ( !pginfo->modentry ) { er = EX_INVAL; goto err_ret2; }

	/* Return module entry point */
	if ( entry ) {
		*entry = (pm_entry_t*)pginfo->modentry;
	}

	UnlockPM();

	return pmid;

err_ret2:
	pmUnloadProg(pginfo);
err_ret1:
	UnlockPM();
err_ret0:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_pm_load ercd = %d\n", er));
#endif
	return er;
}


/*
 * Load system program module
 */
EXPORT ID _pm_loadspg( const struct pm* prog, INT ac, UB* av[] )
{
	ER	er;
	ID	pmid;
	int	i;
	FP	entry = NULL;
	
	/* Check parameters */
	er = ChkSpaceR(prog, sizeof(*prog));
	if ( er < E_OK ) goto err_ret0;

	if ( ac < 0 ) goto err_ret0;

	er = ChkSpaceR(av, ac * sizeof(UB*));
	if ( er < E_OK ) goto err_ret0;

	for ( i = 0; i < ac; i++ ) {
		er = ChkSpaceR(av[i], 0);
		if ( er < E_OK ) goto err_ret0;
	}

	LockPM();

	/* Load program module */
	er = pmLoadProg(prog, TA_RNG0, &entry);
	if ( er < E_OK ) goto err_ret1;
	pmid = er;

	/* Execute startup */
	er = pmCallStartup(ac, av, entry);
	if ( er < E_OK ) goto err_ret2;

	UnlockPM();

	return pmid;

err_ret2:
	pmUnloadProg(pmGetInfo(pmid));
err_ret1:
	UnlockPM();
err_ret0:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_pm_loadspg ercd = %d\n", er));
#endif
	return er;
}


/*
 * Unload program module
 */
EXPORT ER _pm_unload( ID progid )
{
	ProgInfo*	pginfo;
	ER		er;

	LockPM();

	/* Get control block */
	pginfo = pmGetInfo(progid);
	if ( !pginfo || !pginfo->used ) {
		er = E_ID;
		goto err_ret;
	}

	/* If system program, call cleanup */
	er = pmCallStartup(-1, NULL, pginfo->entry);

	/* Unload program module */
	pmUnloadProg(pginfo);

	UnlockPM();

	return er;

err_ret:
	UnlockPM();
#ifdef DEBUG
	TM_DEBUG_PRINT(("_pm_unload ercd = %d\n", er));	
#endif
	return er;
}


/*
 * Get program module information
 */
EXPORT ER _pm_status( ID progid, struct pm_stat* status )
{
	ER		er;
	ProgInfo*	pginfo;

	/* Check parameters */
	er = ChkSpaceR(status, sizeof(*status));
	if ( er < E_OK ) goto err_ret0;

	LockPM();

	/* Get program module information */
	pginfo = pmGetInfo(progid);
	if ( !pginfo || !pginfo->used ) {
		er = E_ID;
		goto err_ret1;
	}

	/* Set information to status */
	status->sysprg = ( pginfo->modentry )? FALSE: TRUE;
	status->attr = pginfo->attr;
	status->entry = ( pginfo->modentry )? pginfo->modentry: pginfo->entry;
	status->start = pginfo->loadadr;
	status->end = (void*)((B*)pginfo->loadadr + pginfo->loadsz);

	UnlockPM();

	return E_OK;

err_ret1:
	UnlockPM();
err_ret0:
#ifdef DEBUG
	TM_DEBUG_PRINT(("_pm_status ercd = %d\n", er));
#endif
	return er;
}
