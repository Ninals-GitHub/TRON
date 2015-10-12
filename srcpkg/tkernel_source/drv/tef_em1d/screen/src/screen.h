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
        screen.h        screen driver : header file
 *
 */
#include <tk/tkernel.h>
#include <libstr.h>
#include <device/screen.h>
#include <device/sdrvif.h>
#include <device/devconf.h>

#ifdef	DEBUG
#define	DP(exp)		printf exp
#else
#define	DP(exp)		/* exp */
#endif

/*
        video-related information
*/
#define	L_OEMNAME	32
#define	L_CHIPINF	32
typedef	struct {
	B	oemname[L_OEMNAME+1];	/* VESA BIOS OEM name (string) */
	B	chipinf[L_CHIPINF+1];	/* chip information (string)        */
	UH	vesaver;		/* VESA BIOS version              */
	UB	reqmode;		/* request mode                        */
	UB	curmode;		/* current mode                        */
	UW	attr;			/* video attribute                        */
	UW	paneltype;		/* panel type (LCD)            */
	UW	modemap;		/* mode map                        */
	void	*framebuf_addr;		/* physical address of framebuffer       */
	W	framebuf_total;		/* total size of framebuffer        */
	W	framebuf_rowb;		/* row bytes of framebuffer       */
	W	banksize;		/* bank size (when banks are used)        */
	W	bankshift;		/* bank shift number (when banks are used)        */

	W	vfreq;			/* vertical sync frequency              */
	W	pciaddr;		/* PCI address                      */
	UH	vendorid;		/* PCI vendor ID          */
	UH	deviceid;		/* PCI device ID          */
	UW	maxvfreq;		/* maximum vertical sync frequency            */

	W	width;			/* screen width (pixel)            */
	W	height;			/* screen height (pixel)\           */
	W	pixbits;		/* number of bits in a pixel              */
	W	act_width;		/* screen width (pixel)            */
	W	act_height;		/* screen height (pixel)\           */

	void	*f_addr;		/* real VRAM logical address                */
	void	*v_addr;		/* virtual VRAM logical address       */
	void	*baseaddr;		/* effective VRAM logical address       */
	W	rowbytes;		/* row bytes of effective VRAM                */
	W	vramsz;			/* effective VRAM size          */
	W	vramrng;		/* VRAM protection level            */

	W	cmapent;		/* number of entries in color map  */
	COLOR	*cmap;			 /* color map                        */

	W	pixbyte;		/* number of bytes per one pixel     */

	W	rotate;			/* whether screen is rotated                        */
	W	fb_width;		/* framebuffer width (pixel)      */
	W	fb_height;		/* framebuffer height (pixel) */

        /* video board- / chip- dependent processing functions */

        /* mode setup processing */
	void	(*fn_setmode)(W flg);

        /* color map setup processing */
	void	(*fn_setcmap)(COLOR *cmap, W ix, W nent);

        /* VRAM bank switch processing */
	void	(*fn_banksw)(W bankno);
        /* screen update processing */
	void	(*fn_updscr)(W x, W y, W dx, W dy);

        /* set / get screen brightness */
	ER	(*fn_bright)(W *brightness, BOOL set);

        /* screen write processing */
	ER	(*fn_write)(W kind, void *buf, W size);

        /* suspend / resume processing (TRUE=suspend, FALSE=resume) */
	void	(*fn_susres)(BOOL suspend);

        /* pointer to extended work area */
	void	*extwrk;
} VideoInf;

/*
        video attribute  (attr)
*/
#define	BPP_24			0x20	/* 24 bpp mode            */
#define	DACBITS_8		0x40	/* DAC is 8 bits          */
#define	LINEAR_FRAMEBUF		0x80	/* Linear Frame Buffer is effective        */

#define	SUPPORT_VFREQ		0x0200	/* VFREQ is supported          */
#define	USE_VVRAM		0x1000	/* use virtual VRAM forcibly                */
#define	NEED_FINPROC		0x10000	/* termination processing is necessary              */
#define	NEED_SUSRESPROC		0x20000	/* suspend/resume processing is necessary  */

/*
        vertical sync frequency (refresh rate) (Hz)
*/
#define	MIN_VFREQ	40		/* minimum */
#define	MAX_VFREQ	100		/* maximum */

/*
        judge whether user process can access real VRAM access or not
                * this is valid only when virtual VRAM is not used
*/
#define	allowUserVRAM	(Vinf.vramrng == 3 && !(Vinf.attr & USE_VVRAM))

/*
        current video information
*/
IMPORT	VideoInf	Vinf;

/*
        external functions
*/
/* common.c */
IMPORT	ER	getMemory(W size, void **ptr);
IMPORT	void*	getPhyMemory(W size, void **phyaddr);
IMPORT	ER	mapFrameBuf(void *paddr, W len, void **laddr);
IMPORT	ER	initSCREEN(void);
IMPORT	ER	finishSCREEN(void);
IMPORT	ER	suspendSCREEN(void);
IMPORT	ER	resumeSCREEN(void);
IMPORT	ER	getSCRXSPEC(DEV_SPEC *spec, W mode);
IMPORT	ER	getSCRSPEC(DEV_SPEC *spec);
IMPORT	W	setModeStr(W mode, TC *str, W pos, W x, W y, W bpp, TC *desc);
IMPORT	INT	getSCRLIST(TC *str);
IMPORT	ER	getsetSCRNO(W *scnum, BOOL suspend, BOOL set);
IMPORT	INT	getsetSCRCOLOR(COLOR *cmap, BOOL set);
IMPORT	ER	getSCRBMP(BMP *bmp);
IMPORT	ER	getsetSCRBRIGHT(W *brightness, BOOL set);
IMPORT	ER	getSCRUPDFN(FP *updfn);
IMPORT	ER	getsetSCRVFREQ(W *vfreq, BOOL set);
IMPORT	ER	getsetSCRADJUST(ScrAdjust *adj, BOOL set);
IMPORT	ER	getSCRDEVINFO(ScrDevInfo *inf);
IMPORT	ER	setSCRUPDRECT(RECT *rp);
IMPORT	ER	setSCRWRITE(W kind, void *buf, W size);

/* (controller dependent) */
IMPORT	W	getSpecSCRXSPEC(DEV_SPEC *spec, W mode);
IMPORT	W	getSpecSCRLIST(TC *str, W pos);

/* Get "DEVCONF" entry */
IMPORT	W	GetDevConf(UB *name, W *val);
