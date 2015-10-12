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
 *	typedef.h (extension)
 *
 *	EXTENSION basic type definitions
 */

#ifndef __EXTENSION_TYPEDEF_H__
#define	__EXTENSION_TYPEDEF_H__

#include <basic.h>
#include <tk/typedef.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef	void	VOID;
#ifndef CONST
#define	CONST	const
#endif

/*
 * System time
 *	Number of seconds starting from 1/1/1985 00:00:00 GMT
 */
typedef W	STIME;

/*
 * Timeout period
 */
typedef	W	TMOUT;

#define	T_NOWAIT	(0)		/* Not wait*/
#define	T_FOREVER	(-1)		/* Permanent wait */

/*
 * Detailed error code
 *	Use ER under normal conditions; if details are needed, use ErrCode.
 */
typedef	union {
	ER		err;		/* Error code */
	struct {
#if BIGENDIAN
		H	eclass;		/* Error class */
		UH	detail;		/* Detail error */
#else
		UH	detail;		/* Detail error */
		H	eclass;		/* Error class */
#endif
	} c;
} ErrCode;

/*
 * System call function value is one of the following:
 *	ER	: Return  error or OK only
 *	WER	: Return error or significant value
 */
typedef	ER	WER;

/*
 * Common definition
 */
#define	CLR		0x0000		/* Specify clear */
#define	NOCLR		0x0008		/* Specify no clear */

/*
 * File link
 */
#define	L_FSNM		20		/* Length of file system name (number of characters) */

typedef struct {
	TC	fs_name[L_FSNM];	/* File system name */
	UH	f_id;			/* File ID */
	UH	atr1;			/* Attribute data 1 */
	UH	atr2;			/* Attribute data 2 */
	UH	atr3;			/* Attribute data 3 */
	UH	atr4;			/* Attribute data 4 */
	UH	atr5;			/* Attribute data 5 */
} LINK;

typedef struct {
	TC	fs_name[L_FSNM];	/* File system name */
	UH	f_id;			/* File ID */
	UH	attr;			/* Avatar type/attribute */
	UH	rel;			/* Relationship index */
	UH	appl[3];		/* Application ID */
} VLINK;

#ifdef __cplusplus
}
#endif
#endif	/* __EXTENSION_TYPEDEF_H__ */
