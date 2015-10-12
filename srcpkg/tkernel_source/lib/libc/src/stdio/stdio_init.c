/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2014/01/08.
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
 *	@(#)stdio_init.c
 *
 */

#include <basic.h>
#include <stdio.h>
#include <t2ex/util.h>

IMPORT	int __sdidinit;
IMPORT	void	__sinit();

struct glue {
	struct	glue *next;
	int	niobs;
	FILE	*iobs;
};

IMPORT	struct glue __sglue;

LOCAL	void (*cleanup)(void);

LOCAL	FastUMLock	mlock;

EXPORT	ER	libc_stdio_init( void )
{
	ER	er;

	if (mlock.id == 0) {
		er = CreateUMLock( &mlock, (UB*)"t2ek" );
		if (er < 0) goto e1;
	}

	__sinit();
	return 0;

e1:	mlock.id = 0;
	return er;
}

EXPORT	ER	libc_stdio_cleanup( void )
{
	if (cleanup) {
		cleanup();
		cleanup = NULL;
	}
	__sdidinit = 0;

	if (mlock.id) {
		DeleteUMLock( &mlock );
		mlock.id = 0;
	}

	return 0;
}

EXPORT	void	__atexit_register_cleanup( void (*f)(void) )
{
	cleanup = f;
}

LOCAL	int	fp2no( FILE *fp )
{
	int	n;
	struct glue *g;

	n = 0;
	for (g = &__sglue; g != NULL; g = g->next) {
		if (g->iobs <= fp && fp < g->iobs + g->niobs) {
			return n + (fp - g->iobs);
		}
		n += g->niobs;
	}
	return -1;
}

EXPORT	void	flockfile(FILE *fp)
{
	int	n;

	if (!__sdidinit || mlock.id == 0) goto e1;

	n = fp2no( fp );
	if (n < 0) goto e1;

	UMLock( &mlock, n );

e1:	;
}

EXPORT	void	funlockfile(FILE *fp)
{
	int	n;

	if (!__sdidinit || mlock.id == 0) goto e1;

	n = fp2no( fp );
	if (n < 0) goto e1;

	UMUnlock( &mlock, n );

e1:	;
}

#define	lock_no_findfp	31

EXPORT	void	__libc_lock_findfp( void )
{
	int	n;

	if (mlock.id == 0) goto e1;

	n = lock_no_findfp;

	UMLock( &mlock, n );

e1:	;
}

EXPORT	void	__libc_unlock_findfp( void )
{
	int	n;

	if (mlock.id == 0) goto e1;

	n = lock_no_findfp;

	UMUnlock( &mlock, n );

e1:	;
}

EXPORT	int	isatty( int fd )
{
	int	r;

	r = (0 <= fd && fd <= 2);
	return r;
}

