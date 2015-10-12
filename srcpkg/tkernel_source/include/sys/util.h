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
 *	@(#)util.h (sys)
 *
 *	Manager utilities
 */

#ifndef __SYS_UTIL_H__
#define __SYS_UTIL_H__

#include <basic.h>
#include <tk/typedef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __pinfo__
typedef struct pinfo	PINFO;	/* Defined in sys/pinfo.h */
#define __pinfo__
#endif

/*
 * SYSCONF definition
 */
#define L_SYSCONF_VAL		16	/* Maximum number of elements */
#define L_SYSCONF_STR		256	/* Maximum string length */

/* Device related */
IMPORT ID	GetDevEvtMbf(void);

/* Memory allocation */
IMPORT void*	Smalloc(size_t size);
IMPORT void*	Scalloc(size_t nmemb, size_t size);
IMPORT void*	Srealloc(void *ptr, size_t size);
IMPORT void	Sfree(void *ptr);
IMPORT void	Smalloctest(int mode);
IMPORT BOOL	Smalloccheck(void *ptr);
IMPORT void	Kmalloctest(int mode);
IMPORT BOOL	Kmalloccheck(void *ptr);
IMPORT void	Vmalloctest(int mode);
IMPORT BOOL	Vmalloccheck(void *ptr);

/* Address check */
IMPORT ER	CheckSpaceR( void *address, W len );
IMPORT ER	CheckSpaceRW( void *address, W len );
IMPORT ER	CheckSpaceRE( void *address, W len );
IMPORT ER	CheckStrSpaceR( TC *str, W max );
IMPORT ER	CheckStrSpaceRW( TC *str, W max );
IMPORT ER	CheckBStrSpaceR( UB *str, W max );
IMPORT ER	CheckBStrSpaceRW( UB *str, W max );

/* Error code conversion */
IMPORT BOOL	_isDebugMode( void );
IMPORT BOOL	_isFsrcvMode( void );

/* Other */
IMPORT void _InitLibtk(void);
IMPORT void KnlInit(void);

#ifdef __cplusplus
}
#endif
#endif /*
 * Library (libtk) initialization
 *	The library is normally initialized automatically,
 *	so these functions do not need to be called explicitly.
 */
