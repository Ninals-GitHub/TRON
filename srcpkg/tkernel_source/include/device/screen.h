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
 *	screen.h
 *
 *       screen (display) driver
 */

#ifndef __DEVICE_SCREEN_H__
#define __DEVICE_SCREEN_H__

#include <basic.h>
#include <tk/devmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SCREEN data number */
typedef	enum {
        /* common attribute */
	DN_SCRSPEC	= TDN_DISPSPEC,	/* DEV_SPEC	(R) */

        /* individual attribute : -100 - -199 are used for general purpose */
	DN_SCRLIST	= -100,		/* TC[]		(R) */
	DN_SCRNO	= -101,		/* W		(RW)*/
	DN_SCRCOLOR	= -102,		/* COLOR[]	(RW)*/
	DN_SCRBMP	= -103,		/* BMP		(R) */

	DN_SCRBRIGHT	= -200,		/* W		(RW)*/

	DN_SCRUPDFN	= -300,		/* FP		(R) */
	DN_SCRVFREQ	= -301,		/* W		(RW)*/
	DN_SCRADJUST	= -302,		/* ScrAdjust	(RW)*/
	DN_SCRDEVINFO	= -303,		/* ScrDevInfo	(R) */
	DN_SCRMEMCLK	= -304,		/* W		(RW)*/
	DN_SCRUPDRECT	= -305,		/* RECT		(RW)*/
	DN_SCRWRITE	= -306,		/* -		(W) */

	DN_SCRXSPEC0	= -500,		/* DEV_SPEC	(R) */
} ScrDataNo;

// we use the range from -755 to - 500
#define	DN_SCRXSPEC(n)	(DN_SCRXSPEC0 - ((n) & 0xff))

/*
        DN_SCRADJUST : monitor adjust parameter
*/
typedef	struct {
	UH	left;		/* number of blank pixels on the left (multiple of 8)    */
	UH	hsync;		/* number of pixels for horizontal sync (multiple of 8)    */
	UH	right;		 /* number of blank pixels on the right (multiple of 8)    */
	UH	top;		/* number of blank pixels at the top                   */
	UH	vsync;		/* number of pixels for vertical sync                     */
	UH	bottom;		/* number of blank pixels at the bottom                    */
} ScrAdjust;

/*
        DN_SCRDEVINFO : device information
*/
typedef	struct {
	UB	name1[32];	/* name-1 (ASCII)                  */
	UB	name2[32];	/* name-2 (ASCII)                  */
	UB	name3[32];	/* name-3 (ASCII)                  */
	void*	framebuf_addr;	/* physical address of framebuffer                */
	W	framebuf_size;	/* framebuffer size         */
	W	mainmem_size;	/* main memory size                     */
	UB	reserved[24];	/* reserved                                    */
} ScrDevInfo;

/*
	(btron/typedef.h)
*/
/* color */
#ifndef	__color__
#define	__color__
typedef UW	COLOR;	/* color */
#endif	/* __color__ */

/* point */
#ifndef	__pnt__
#define	__pnt__
typedef struct point {
	H	x;	/* horizontal coordinate value*/
	H	y;	/* vertical coordinate value */
} PNT;
#endif	/* __pnt__ */

/* rectangle */
#ifndef	__rect__
#define	__rect__
typedef	union rect {
	struct _rect {
		H	left;		/* X coordinate value of left edge */
		H	top;		/* Y coodinate value of the upper edge. */
		H	right;		/* X coordinate value of right edge */
		H	bottom;		/* Y coordinate value of the lower edge */
	} c;
	struct {
		PNT	lefttop;	/* upper-left corner point */
		PNT	rightbot;	/* lower-right corner point */
	} p;
} RECT;
#endif	/* __rect__ */

/*
	(btron/dp.h)
*/
/* bitmap */
#ifndef	__bmp__
#define	__bmp__
#define PLANES		1

typedef struct Bitmap {
	UW	planes;			/* number of planes		*/
	UH	pixbits;		/* number of pixel bits		*/
	UH	rowbytes;		/* row bytes of a plane		*/
	RECT	bounds;			/* coordinate specification	*/
	UB	*baseaddr[PLANES];	/* base addresses		*/
} BMP;
#endif	/* __bmp__ */

/* device information */
#ifndef	__dev_spec__
#define	__dev_spec__
typedef	struct	{
	H	attr;		/* device attribute */
	H	planes;		/* number of planes */
	H	pixbits;	/* number of bits in a pixel (boundary / effective value) */
	H	hpixels;	/* number of pixels on the horizontal row */
	H	vpixels;	/* number of pixels in the vertical direction */
	H	hres;		/* horizontal resolution */
	H	vres;		/* vertical resolution */
	H	color[4];	/* color information */
	H	resv[6];
}	DEV_SPEC;

/* device attributes */
#define	DA_CANLOCK	0x4000
#define	DA_HAVEBMP	0x2000
#define	DA_HAVECMAP	0x0008
#define	DA_COLORSYSTEM	0x0007

/* Color System */
#define	DA_COLOR_MONO	0
#define	DA_COLOR_RGB	1
#define	DA_COLOR_CMY	2
#endif	/* __dev_spec__ */

#ifdef __cplusplus
}
#endif
#endif /* __DEVICE_SCREEN_H__ */
