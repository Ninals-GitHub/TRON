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
 *	videomode.h
 *
 *       video mode definitions
 */

#ifndef	__DEVICE_VIDEOMODE_H__
#define	__DEVICE_VIDEOMODE_H__

#include <machine.h>

/* below are some modes that are not supported by a particular system */

#ifdef __cplusplus
extern "C" {
#endif

#define	MAX_VIDEO_SZIX	8

/*
        video mode
*/
typedef	enum {
	DMi240x8	= 0,	/* 240 x 320 256 colors (color map)     */
	DMi240x16	= 1,	/* 240 x 320 16 bits(R5-G6-B5)    */
	DMi240x32	= 2,	/* 240 x 320 32 bits (R8-G8-B8)    */

	DMe640x8	= 3,	/* 640 x 480 256 colors (color map)     */
	DMe640x16	= 4,	/* 640 x 480 16 bits (R5-G6-B5)    */
	DMe640x32	= 5,	/* 640 x 480 32 bits (R8-G8-B8)    */

	DMe800x8	= 6,	/* 800 x 600 256 colors (color map)      */
	DMe800x16	= 7,	/* 800 x 600 16 bits (R5-G6-B5)    */
	DMe800x32	= 8,	/* 800 x 600 32 bits (R8-G8-B8)    */

	DMe1024x8	= 9,	/* 1024 x 768 256 colors (colar map)   */
	DMe1024x16	= 10,	/* 1024 x 768 16 bits (R5-G6-B5)  */
	DMe1024x32	= 11,	/* 1024 x 768 32 bits(R8-G8-B8)  */

	DMe1152x8	= 12,	/* 1152 x 864 256 colors (color map)    */
	DMe1152x16	= 13,	/* 1152 x 864 16 bits (R5-G6-B5)  */
	DMe1152x32	= 14,	/* 1152 x 864 32 bits (R8-G8-B8)  */

	DMe1280x8	= 15,	/* 1280 x 1024 256 colors (color map)    */
	DMe1280x16	= 16,	/* 1280 x 1024 16 bits (R5-G6-B5) */
	DMe1280x32	= 17,	/* 1280 x 1024 32 bits (R8-G8-B8) */

	DMi480x8	= 18,	/* 480 x 640 256 colors (color map)    */
	DMi480x16	= 19,	/* 480 x 640 16 bits (R5-G6-B5)    */
	DMi480x32	= 20,	/* 480 x 640 32 bits (R8-G8-B8)    */

	DMeWVGAx16	= 22,	/* 800 x 480 16 bits (R5-G6-B5)    */
} VideoMode;

#define	MAX_VIDEO_COLIX		3
#define	MAX_VIDEO_MODE		(MAX_VIDEO_SZIX * MAX_VIDEO_COLIX)

#define	VideoSzIx(mode)		((mode) / MAX_VIDEO_COLIX)
#define	VideoColIx(mode)	((mode) % MAX_VIDEO_COLIX)
#define	VALID_VIDEO_MODE(mode)	((mode) >= 0 && (mode) < MAX_VIDEO_MODE)

/* screen horizontal size */
#define	VideoHsize(mode)	__vHsize[VideoSzIx(mode)]
/* screen vertical size */
#define	VideoVsize(mode)	__vVsize[VideoSzIx(mode)]

/* number of bits per pixel */
#define	VideoPixBits(mode)	__vPixBits[VideoColIx(mode)]

/* number of entries in color map */
#define	VideoCmapEnt(mode)	__vCmapEnt[VideoColIx(mode)]
#define	MAX_COLMAP_ENT		256

/* number of bits and the location of bits for component color in direct color method */
#define	VideoRedInf(mode)	__vRedInf[VideoColIx(mode)]
#define	VideoGreenInf(mode)	__vGreenInf[VideoColIx(mode)]
#define	VideoBlueInf(mode)	__vBlueInf[VideoColIx(mode)]

LOCAL	const	UH	__vHsize[MAX_VIDEO_SZIX] =
			{240, 640, 800, 1024, 1152, 1280, 480, 800};
LOCAL	const	UH	__vVsize[MAX_VIDEO_SZIX] =
			{320, 480, 600,  768,  864, 1024, 640, 480};
LOCAL	const	UH	__vPixBits[MAX_VIDEO_COLIX] =
			{0x0808, 0x1010, 0x2018};
LOCAL	const	UH	__vCmapEnt[MAX_VIDEO_COLIX] =
			{256, 0, 0};
LOCAL	const	UH	__vRedInf[MAX_VIDEO_COLIX] =
			{0x0000, 0x0B05, 0x1008};
LOCAL	const	UH	__vGreenInf[MAX_VIDEO_COLIX] =
			{0x0000, 0x0506, 0x0808};
LOCAL	const	UH	__vBlueInf[MAX_VIDEO_COLIX] =
			{0x0000, 0x0005, 0x0008};

#ifdef __cplusplus
}
#endif

#endif	/* __DEVICE_VIDEOMODE_H__ */
