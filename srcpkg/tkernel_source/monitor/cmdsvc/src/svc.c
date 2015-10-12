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
 *	svc.c
 *
 *       service call
 */

#include "cmdsvc.h"
#include <sys/rominfo.h>

/*
 * submodule extended SVC function
 *       fno     function code
 *       p1-p3 parameter(s)
 *       er      returns error code
 *       return value      if the function is handled, returns TRUE.
 *                         if not, returns FALSE
 */
typedef BOOL (*ESVCFUNC)( W fno, W p1, W p2, W p3, W *er );

/*
 * use weak attribute to call only the linked submodule.
 * `weak' is a gcc-specific function and so other methods (using "#if" for example) need to be employed with other compilers.
 *
 */
#ifdef __GNUC__
#define	WEAK	__attribute__ ((weak))
#else
#define	WEAK
#endif

/*
 * submodule
 */
IMPORT BOOL memDiskSVC( W fno, W p1, W p2, W p3, W *er )	WEAK;
IMPORT BOOL pciSVC( W fno, W p1, W p2, W p3, W *er )		WEAK;
IMPORT BOOL usbSVC( W fno, W p1, W p2, W p3, W *er )		WEAK;
IMPORT BOOL sysExtSVC( W fno, W p1, W p2, W p3, W *er )		WEAK;

/*
 * calling submodule extended SVC function
 */
LOCAL BOOL callSubModuleSVC( ESVCFUNC func, W fno, W p1, W p2, W p3, W *er )
{
	if ( func == NULL ) return FALSE;

	return (*func)(fno, p1, p2, p3, er);
}

#define	CALL_ESVC(func)		callSubModuleSVC(func, fno, p1, p2, p3, &er)

/*
 * extended service call processing
 */
LOCAL W procExtSVC( W fno, W p1, W p2, W p3 )
{
	W	er;

	switch ( fno ) {
	  case TMEF_PORTBPS:	/* debug port speed (bps) */
		er = ConPortBps;
		break;

	  case TMEF_DIPSW:	/* DIPSW status */
		er = getDipSw();
		break;

	  case TMEF_WROM:	/* Flash ROM write */
		er = writeFrom(p1, p2, p3, 0);
		break;

	  default:
		CALL_ESVC(sysExtSVC) ||
		CALL_ESVC(usbSVC) ||
		CALL_ESVC(pciSVC) ||
		CALL_ESVC(memDiskSVC) ||
		(er = E_PAR);
	}

	return er;
}

/* ------------------------------------------------------------------------ */

/*
 * service call
 */
EXPORT W procSVC( W fno, W p1, W p2, W p3, W p4 )
{
	W	er = E_OK;
	W	n;

	switch ( fno ) {
	  case TM_MONITOR:	/* void tm_monitor( void ) */
		procCommand(NULL, 0);
		break;

	  case TM_GETCHAR:	/* INT	tm_getchar( INT wait ) */
		er = getChar(p1);
		break;

	  case TM_PUTCHAR:	/* INT	tm_putchar( INT c ) */
		er = putChar(p1);
		break;

	  case TM_GETLINE:	/* INT	tm_getline( UB *buff ) */
		er = getString(wrkBuf);
		if ( er < 0 ) break;
		n = er + 1;
		if ( writeMem(p1, wrkBuf, n, 1) != n ) er = E_MACV;
		break;

	  case TM_PUTSTRING:	/* INT	tm_putstring( const UB *buff ) */
		n = readMemStr(p1, wrkBuf, WRKBUF_SZ);
		if ( n < 0 ) { er = E_MACV; break; }
		er = putString(wrkBuf);
		break;

	  case TM_COMMAND:	/* INT	tm_command( const UB *buff ) */
		n = readMemStr(p1, lineBuf, L_LINE);
		if ( n < 0 ) { er = E_MACV; break; }
		procCommand(( n == 0 )? NULL: lineBuf, 1);
		break;

	  case TM_READDISK:
	  case TM_WRITEDISK:
	  case TM_INFODISK:
		/* INT tm_readdisk( const UB *dev, INT sec, INT nsec, void *addr )
		 * INT tm_writedisk( const UB *dev, INT sec, INT nsec, void *addr )
		 * INT tm_infodisk( const UB *dev, INT *blksz, INT *nblks )
		 */
		n = readMemStr(p1, lineBuf, L_LINE);
		if ( n < 0 ) { er = E_MACV; break; }

		if ( fno == TM_INFODISK ) {
			er = infoDisk(lineBuf, (UW*)p2, (UW*)p3);
		} else {
			n = ( fno == TM_READDISK )? 0: 1;
			er = rwDisk(lineBuf, p2, p3, (void*)p4, n);
		}
		break;

	  case TM_EXIT:		/* void tm_exit( INT mode ) */
		sysExit(p1); /* do not return */
		break;

	  case TM_EXTSVC:
		/* INT	tm_extsvc( INT fno, INT par1, INT par2, INT par3 ) */
		er = procExtSVC(p1, p2, p3, p4);
		break;

	  default:
		er = E_PAR;
	}

	return er;
}
