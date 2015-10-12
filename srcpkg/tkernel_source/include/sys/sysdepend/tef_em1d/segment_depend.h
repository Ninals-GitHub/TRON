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
 *	@(#)segment_depend.h (sys/EM1-D512)
 *
 *	Segment management system dependent definitions
 */

#ifndef __SYS_SEGMENT_DEPEND_H__
#define __SYS_SEGMENT_DEPEND_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Access level definitions */
typedef enum {
	MapUser		= 0x02,
	MapSystem	= 0x00,
	MapRead		= 0x00,
	MapWrite	= 0x30,
	MapExecute	= 0x04
} MapModeLevel;

/*
 * MapMemory() attr
 */
#define MM_USER		0x020U	/* User */
#define MM_SYSTEM	0x000U	/* System */
#define MM_READ		0x000U	/* Read */
#define MM_WRITE	0x300U	/* Write */
#define MM_EXECUTE	0x040U	/* Execute */
#define MM_CDIS		0x00cU	/* Cache disabled */

#ifdef __cplusplus
}
#endif
#endif /* __SYS_SEGMENT_DEPEND_H__ */
