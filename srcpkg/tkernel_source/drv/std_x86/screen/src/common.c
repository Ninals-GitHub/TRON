/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by T-Engine Forum at 2012/10/24.
 *
 *----------------------------------------------------------------------
 */

/*
 *       common.c        screen driver (common part)
 *
 *       all the screen configuration is done by the individual driver tailed to the specific video chip.
 *       information regarding the display mode is enumerated as constant definitions
 *				==> device/teng/videomode.h
 *       display mode can not be changed, hence.
 */
#include "screen.h"

#include <sys/segment.h>
#include <device/videomode.h>
#include <tcode.h>

EXPORT	VideoInf	Vinf;		/* current video information              */

IMPORT	FUNCP	VideoFunc[];		/* video chip dependent processing functions */

LOCAL	CONST	UB	*OEMName = "T-Engine Video Device";

/* definition of color map */
EXPORT	UW	Cmap[256] = {
	0x10ffffff, 0x1000009f, 0x1000ef00, 0x1000efef,
	0x10ff0000, 0x10ef00ff, 0x10dfdf00, 0x107f7f9f,
	0x10dfdfdf, 0x107f9fff, 0x10bfffaf, 0x10cfffff,
	0x10ff6f6f, 0x10ef8fff, 0x10efff9f, 0x10010101,
	0x10ffffcc, 0x10ffff66, 0x10ffff33, 0x10ffff00,
	0x10ffccff, 0x10ffcccc, 0x10ffcc99, 0x10ffcc66,
	0x10ffcc33, 0x10ffcc00, 0x10ff99cc, 0x10ff9999,
	0x10ff9966, 0x10ff9933, 0x10ff9900, 0x10ff66ff,
	0x10ff66cc, 0x10ff6699, 0x10ff6633, 0x10ff6600,
	0x10ff33ff, 0x10ff33cc, 0x10ff3399, 0x10ff3366,
	0x10ff3333, 0x10ff3300, 0x10ff00cc, 0x10ff0099,
	0x10ff0066, 0x10ff0033, 0x10ccff99, 0x10ccff66,
	0x10ccff33, 0x10ccff00, 0x10ccccff, 0x10cccc99,
	0x10cccc66, 0x10cccc33, 0x10cc99ff, 0x10cc99cc,
	0x10cc9999, 0x10cc9966, 0x10cc9933, 0x10cc9900,
	0x10cc66ff, 0x10cc66cc, 0x10cc6699, 0x10cc6666,
	0x10cc6633, 0x10cc6600, 0x10cc33ff, 0x10cc33cc,
	0x10cc3399, 0x10cc3366, 0x10cc3333, 0x10cc3300,
	0x10cc00ff, 0x10cc00cc, 0x10cc0099, 0x10cc0066,
	0x10cc0033, 0x10cc0000, 0x1099ffff, 0x1099ffcc,
	0x1099ff99, 0x1099ff66, 0x1099ff33, 0x1099ff00,
	0x1099ccff, 0x1099cccc, 0x1099cc99, 0x1099cc66,
	0x1099cc33, 0x1099cc00, 0x109999ff, 0x109999cc,
	0x10999999, 0x10999966, 0x10999933, 0x10999900,
	0x109966ff, 0x109966cc, 0x10996699, 0x10996666,
	0x10996633, 0x10996600, 0x109933ff, 0x109933cc,
	0x10993399, 0x10993366, 0x10993333, 0x10993300,
	0x109900ff, 0x109900cc, 0x10990099, 0x10990066,
	0x10990033, 0x10990000, 0x1066ffff, 0x1066ffcc,
	0x1066ff99, 0x1066ff66, 0x1066ff33, 0x1066ff00,
	0x1066ccff, 0x1066cccc, 0x1066cc99, 0x1066cc66,
	0x1066cc33, 0x1066cc00, 0x106699cc, 0x10669999,
	0x10669966, 0x10669933, 0x10669900, 0x106666ff,
	0x106666cc, 0x10666666, 0x10666633, 0x10666600,
	0x106633ff, 0x106633cc, 0x10663399, 0x10663366,
	0x10663333, 0x10663300, 0x106600ff, 0x106600cc,
	0x10660099, 0x10660066, 0x10660033, 0x10660000,
	0x1033ffff, 0x1033ffcc, 0x1033ff99, 0x1033ff66,
	0x1033ff33, 0x1033ff00, 0x1033ccff, 0x1033cccc,
	0x1033cc99, 0x1033cc66, 0x1033cc33, 0x1033cc00,
	0x103399ff, 0x103399cc, 0x10339999, 0x10339966,
	0x10339933, 0x10339900, 0x103366ff, 0x103366cc,
	0x10336699, 0x10336666, 0x10336633, 0x10336600,
	0x103333ff, 0x103333cc, 0x10333399, 0x10333366,
	0x10333333, 0x10333300, 0x103300ff, 0x103300cc,
	0x10330099, 0x10330066, 0x10330033, 0x10330000,
	0x1000ffcc, 0x1000ff99, 0x1000ff66, 0x1000ff33,
	0x1000ff00, 0x1000ccff, 0x1000cccc, 0x1000cc99,
	0x1000cc66, 0x1000cc33, 0x1000cc00, 0x100099ff,
	0x100099cc, 0x10009999, 0x10009966, 0x10009933,
	0x10009900, 0x100066ff, 0x100066cc, 0x10006699,
	0x10006666, 0x10006633, 0x10006600, 0x100033ff,
	0x100033cc, 0x10003399, 0x10003366, 0x10003333,
	0x10003300, 0x100000ff, 0x100000cc, 0x10000066,
	0x10000033, 0x10ee0000, 0x10dd0000, 0x10bb0000,
	0x10aa0000, 0x10880000, 0x10770000, 0x10550000,
	0x10440000, 0x10220000, 0x10110000, 0x1000dd00,
	0x1000bb00, 0x1000aa00, 0x10008800, 0x10007700,
	0x10005500, 0x10004400, 0x10002200, 0x10001100,
	0x100000ee, 0x100000dd, 0x100000bb, 0x100000aa,
	0x10000088, 0x10000077, 0x10000055, 0x10000044,
	0x10000022, 0x10eeeeee, 0x10cccccc, 0x10bbbbbb,
	0x10aaaaaa, 0x10888888, 0x10777777, 0x10555555,
	0x10444444, 0x10222222, 0x10111111, 0x10000000,
};

/*
        obtain number of memory blocks
*/
LOCAL	W	getMblkSz(W size)
{
	T_RSMB	msts;

	if (tk_ref_smb(&msts) < E_OK) return -1;
	return (size - 1) / msts.blksz + 1;
}
/*
        obtain large amount of memory
        ( we do not use Kmalloc() , so that we use less memory)
                * we use this to obtain the area for virtual VRAM
*/
EXPORT	ER	getMemory(W size, void **ptr)
{
	W	nblk;
LOCAL	UINT	attr[] = {TA_RNG0, TA_RNG1, TA_RNG2, TA_RNG3};

	if ((nblk = getMblkSz(size)) > 0) {
		if (tk_get_smb(ptr, nblk, attr[Vinf.vramrng]) >= E_OK)
			return E_OK;
	}
	return E_NOMEM;
}
/*
        obtain contiguous physical memory
*/
EXPORT	void*	getPhyMemory(W size, void **phyaddr)
{
	ER	err;
	UINT	attr;
	void*	la;

        /* user process can access this only when we do not use virtual VRAM */
	attr = allowUserVRAM ? MM_USER : MM_SYSTEM;

	err = MapMemory(NULL, size,
			attr | MM_READ | MM_WRITE | MM_CDIS, &la);

	return (err < E_OK ||
		CnvPhysicalAddr(la, size, phyaddr) < size) ? NULL : la;
}
/*
        mapping framebuffer to logical address space
*/
EXPORT	ER	mapFrameBuf(void *paddr, W len, void **laddr)
{
	UINT	attr;

        /* user process can access this only when we do not use virtual VRAM */
	attr = allowUserVRAM ? MM_USER : MM_SYSTEM;

	return MapMemory(paddr, len,
			attr | MM_READ | MM_WRITE | MM_CDIS, laddr);
}
/*
        initialization
*/
EXPORT	ER	initSCREEN(void)
{
	W	i, n;
	W	v[L_DEVCONF_VAL];

        /* initialize video information */
	memset(&Vinf, 0, sizeof(VideoInf));

        /* PCI address of effective VIDEO board: PCI is not used */
	Vinf.pciaddr    = -1;

        /* video attribute : extract VIDEOATTR */
	if (GetDevConf("VIDEOATTR", v) > 0 && v[0] > 0) {
		Vinf.attr |= v[0] << 12;
	}

        /* default display mode */
	n = 0;

        /* requested display mode : VIDEOMODE */
	v[2] = v[3] = 0;
	if (GetDevConf("VIDEOMODE", v) > 0 && v[0] > 0) {
		if (VALID_VIDEO_MODE(v[0] - 1)) n = v[0] - 1;
	}

	Vinf.reqmode = n;

        /* whether screen is rotated : VIDEOROTATE */
	Vinf.rotate = 0;	/* do not rotate */

        /* panel type (LCD) : LCDPANELTYPE */
	if (GetDevConf("LCDPANELTYPE", v) > 0 && v[0] > 0) {
		Vinf.paneltype = v[0];
	}

        /* VRAM protection level */
	if (GetDevConf("VIDEOPROT", v) > 0) {
		Vinf.vramrng = v[0] & 3;
	}

        /* configuration of various video information */
	Vinf.banksize   = 0;
	Vinf.bankshift  = 0;
	Vinf.cmap       = Cmap;
	strncpy(Vinf.oemname, OEMName, L_OEMNAME);

	/*
         * for the following parameters, the individual driver for a specific video chip sets them
	 * Vinf.framebuf_addr, Vinf.framebuf_total, Vinf.framebuf_rowb,
	 * Vinf.attr, Vinf.fn_setcmap, Vinf.vramsz, Vinf.f_addr,
	 * Vinf.fn_susres, Vinf.modemap
	 */

        /* initialize according the needs of video board and chip */
	for (i = 0; VideoFunc[i] != NULL &&
	     (n = (*VideoFunc[i])()) == 0; i++);

	if (n < 0) return E_NOEXS;	/* error */

        /* configuration of display mode information (at this stage, curmode is set) */
	Vinf.width      = Vinf.fb_width   = VideoHsize(Vinf.curmode);
	Vinf.height     = Vinf.fb_height  = VideoVsize(Vinf.curmode);
	Vinf.pixbits    = VideoPixBits(Vinf.curmode);
	Vinf.rowbytes   = Vinf.framebuf_rowb;
	Vinf.vramsz     = Vinf.framebuf_rowb * Vinf.fb_height;
	if (Vinf.framebuf_total > 0 &&
		Vinf.vramsz > Vinf.framebuf_total) return E_OBJ;

        /* obtain the effective size of screen */
	Vinf.act_width	= (v[2] >= 160 && v[2] < Vinf.width) ?
						v[2] : Vinf.width;
	Vinf.act_height = (v[3] >= 160 && v[3] < Vinf.height) ?
						v[3] : Vinf.height;

        /* obtain vertical scan frequency : VIDEOVFREQ */
	if (GetDevConf("VIDEOVFREQ", v) > 0 && v[0] > 0) {
		if ((Vinf.vfreq = v[0]) < MIN_VFREQ) Vinf.vfreq = MIN_VFREQ;
		else if (Vinf.vfreq > MAX_VFREQ) Vinf.vfreq = MAX_VFREQ;
	}

        /* configure actual video mode */
	(*Vinf.fn_setmode)(1);

        /* set effective VRAM address */
	Vinf.baseaddr = Vinf.f_addr;

        /* set color map */
	Vinf.cmapent = VideoCmapEnt(Vinf.curmode);
	if (Vinf.cmapent > 0) {
                /* entry #0 (white) is yet to be set so that the screen can be totally dark */
		(*Vinf.fn_setcmap)(Vinf.cmap + 1, 1, Vinf.cmapent - 1);

		if (Vinf.v_addr == NULL) {	/* screen clear (black = 0xFF) */
			memset(Vinf.baseaddr, 0xFF, Vinf.vramsz);
		} else {
                        /* update virtual VRAM screen(virtual VRAM has already been cleared) */
			(*Vinf.fn_updscr)(0, 0, Vinf.width, Vinf.height);
		}
                /* set entry #0 (white) */
		(*Vinf.fn_setcmap)(Vinf.cmap, 0, 1);
	}

	return E_OK;
}
/*
        epilog processing
*/
EXPORT	ER	finishSCREEN(void)
{
        /* there is nothing special to do */
	return E_OK;
}
/*
        suspend processing
*/
EXPORT	ER	suspendSCREEN(void)
{
	if (Vinf.attr & NEED_SUSRESPROC) (*Vinf.fn_susres)(TRUE);
	return E_OK;
}
/*
        resume processing
*/
EXPORT	ER	resumeSCREEN(void)
{
	if (Vinf.attr & NEED_SUSRESPROC) (*Vinf.fn_susres)(FALSE);

	return E_OK;
}
/*
        obtain device specification (mode setting)
*/
EXPORT	ER	getSCRXSPEC(DEV_SPEC *spec, W mode)
{
	ER	er;

	if (mode >= MAX_VIDEO_MODE || !(Vinf.modemap & (1 << mode))) {
		er = E_NOSPT;
		goto fin0;
	}

	spec->planes = 1;
	spec->pixbits = VideoPixBits(mode);
	spec->hpixels = VideoHsize(mode);
	spec->vpixels = VideoVsize(mode);
	spec->hres = 0;
	spec->vres = 0;
	if (VideoCmapEnt(mode) > 0) {
		spec->attr = DA_COLOR_RGB | DA_HAVECMAP | DA_HAVEBMP;
		spec->color[0] = spec->color[1] = spec->color[2] = 8;
	} else {
		spec->attr = DA_COLOR_RGB | DA_HAVEBMP;
		spec->color[0] = VideoRedInf(mode);
		spec->color[1] = VideoGreenInf(mode);
		spec->color[2] = VideoBlueInf(mode);
	}
	er = E_OK;

 fin0:
	return er;
}
/*
        obtain device specfication
*/
EXPORT	ER	getSCRSPEC(DEV_SPEC *spec)
{
	spec->planes = 1;
	spec->pixbits = Vinf.pixbits;
	spec->hpixels = Vinf.act_width;
	spec->vpixels = Vinf.act_height;
	spec->hres = 0;
	spec->vres = 0;
	if (Vinf.cmapent > 0) {
		spec->attr = DA_COLOR_RGB | DA_HAVECMAP | DA_HAVEBMP;
		spec->color[0] = spec->color[1] = spec->color[2] = 8;
	} else {
		spec->attr = DA_COLOR_RGB | DA_HAVEBMP;
		spec->color[0] = VideoRedInf(Vinf.curmode);
		spec->color[1] = VideoGreenInf(Vinf.curmode);
		spec->color[2] = VideoBlueInf(Vinf.curmode);
	}
	return E_OK;
}
/*
        set display mode string
*/
EXPORT	W	setModeStr(W mode, TC *str, W pos, W x, W y, W bpp, TC *desc)
{
#define	MODESTR_SIZE	16	// the number specified by VIDEOMODE needs to fit in this area
#define	TK_MULT		0x215f
#define	putTC(x, y, z)	(x)[(y)] = (z)

	W	n;

        // "mxxxxXyyyy:cccc  ...  " (TC)
        //       m: video mode number
        //       x: X width (1-4 columns)
        //       y: Y height (1-4 rows)
        //       c: number of colors
        //       ' ' (space) : padding

        /* if str = NULL, the length of the resulting string is returned */
	if (str == NULL) goto fin0;

        /* initialize pointers, etc. */
	str = &str[pos];
	n = 0;

	/* VIDEOMODE */
	putTC(str, n++, mode + 1);

        /* X size */
	if (x >= 1000)	putTC(str, n++, TK_0 + (x / 1000) % 10);
	if (x >=  100)	putTC(str, n++, TK_0 + (x /  100) % 10);
	if (x >=   10)	putTC(str, n++, TK_0 + (x /   10) % 10);
			putTC(str, n++, TK_0 +  x         % 10);

	putTC(str, n++, TK_MULT);	// (multiplication symbol in TC)

        /* Y size */
	if (y >= 1000)	putTC(str, n++, TK_0 + (y / 1000) % 10);
	if (y >=  100)	putTC(str, n++, TK_0 + (y /  100) % 10);
	if (y >=   10)	putTC(str, n++, TK_0 + (y /   10) % 10);
			putTC(str, n++, TK_0 +  y         % 10);

	putTC(str, n++, TK_COLN);	// : (TC)

        /* number of colors */
	if (bpp == 4) {
		putTC(str, n++, TK_1);
		putTC(str, n++, TK_6);
		putTC(str, n++, TK_C);
	} else if (bpp == 8) {
		putTC(str, n++, TK_2);
		putTC(str, n++, TK_5);
		putTC(str, n++, TK_6);
		putTC(str, n++, TK_C);
	} else if (bpp == 16) {
		putTC(str, n++, TK_6);
		putTC(str, n++, TK_4);
		putTC(str, n++, TK_K);
		putTC(str, n++, TK_C);
	} else if (bpp == 24 || bpp == 32) {
		putTC(str, n++, TK_1);
		putTC(str, n++, TK_6);
		putTC(str, n++, TK_M);
		putTC(str, n++, TK_C);
	}

        /* additional information */
	if (desc != NULL) {
		for (; n < MODESTR_SIZE && *desc != TC_NULL; n++, desc++) {
			putTC(str, n, *desc);
		}
	}

        /* padding to make the string length constant */
	for (; n < MODESTR_SIZE; n++) putTC(str, n, TK_KSP);
 fin0:
	return pos + MODESTR_SIZE;
}
/*
        obtain the list of supported display modes
*/
EXPORT	INT	getSCRLIST(TC *str)
{
	W	pos, m;

	pos = 0;

        /* standard mode (videomode.h) */
	for(m = 0; m < MAX_VIDEO_MODE; m++) {
		if(Vinf.modemap & (1 << m)) {
			pos = setModeStr(m, str, pos,
					 VideoHsize(m), VideoVsize(m),
					 VideoPixBits(m) & 0xff, NULL);
		}
	}

	if (str != NULL) str[pos] = TNULL;
	return (pos + 1) * sizeof(TC);
}
/*
        get / set display mode
*/
EXPORT	ER	getsetSCRNO(W *scnum, BOOL suspend, BOOL set)
{
	if (set) return E_NOSPT;	/* mode is not supported */
	*scnum = Vinf.curmode + 1;
	return E_OK;
}
/*
        get / set color map
*/
EXPORT	INT	getsetSCRCOLOR(COLOR *cmap, BOOL set)
{
	if (Vinf.cmapent <= 0) return E_OBJ;	/* colormap is not used */

	if (cmap != NULL) {
		if (set) {
			memcpy(Vinf.cmap, cmap, Vinf.cmapent * sizeof(COLOR));
			(*Vinf.fn_setcmap)(Vinf.cmap, 0, Vinf.cmapent);
		} else {
			memcpy(cmap, Vinf.cmap, Vinf.cmapent * sizeof(COLOR));
		}
	}
	return Vinf.cmapent * sizeof(COLOR);
}
/*
        device-specific image area
*/
EXPORT	ER	getSCRBMP(BMP *bmp)
{
	bmp->planes = 1;
	bmp->pixbits = Vinf.pixbits;
	bmp->rowbytes = Vinf.rowbytes;
	bmp->bounds.c.left = 0;
	bmp->bounds.c.top = 0;
	bmp->bounds.c.right = Vinf.act_width;
	bmp->bounds.c.bottom = Vinf.act_height;
	bmp->baseaddr[0] = Vinf.baseaddr;
	return E_OK;
}
/*
        get / set screen brightness
*/
EXPORT	ER	getsetSCRBRIGHT(W *brightness, BOOL set)
{
	if (Vinf.fn_bright) return (*Vinf.fn_bright)(brightness, set);
	return E_NOSPT;		/* not supported */
}
/*
        obtain the screen update function
*/
EXPORT	ER	getSCRUPDFN(FP *updfn)
{
	*updfn = (FP)Vinf.fn_updscr;
	return E_OK;
}
/*
        update the virtual VRAM screen
*/
EXPORT        ER      setSCRUPDRECT(RECT *rp)
{
	if (! Vinf.fn_updscr) return E_NOSPT;	/* not supported */
	(*Vinf.fn_updscr)(rp->c.left, rp->c.top, rp->c.right - rp->c.left,
			rp->c.bottom - rp->c.top);
	return E_OK;
}
/*
        get / set monitor vertical scan frequency
*/
EXPORT	ER	getsetSCRVFREQ(W *vfreq, BOOL set)
{
	if ((Vinf.attr & SUPPORT_VFREQ) == 0) return E_NOSPT; /* not supported */

	if (set) {
		if ((Vinf.vfreq = *vfreq) <= 0)  Vinf.vfreq = 0;
		else if (Vinf.vfreq < MIN_VFREQ) Vinf.vfreq = MIN_VFREQ;
		else if (Vinf.vfreq > MAX_VFREQ) Vinf.vfreq = MAX_VFREQ;
		(*Vinf.fn_setmode)(0);
	} else {
		*vfreq = Vinf.vfreq;
	}
	return E_OK;
}
/*
        get / set monitor adjustment parameters
*/
EXPORT	ER	getsetSCRADJUST(ScrAdjust *adj, BOOL set)
{
        /* not supported */
	return E_NOSPT;
}
/*
        obtain video device information
*/
EXPORT	ER	getSCRDEVINFO(ScrDevInfo *inf)
{
	memset((void*)inf, 0, sizeof(ScrDevInfo));
	memcpy(inf->name1, Vinf.oemname, L_OEMNAME);
	memcpy(inf->name3, Vinf.chipinf, L_CHIPINF);

	inf->framebuf_addr = (Vinf.attr & BPP_24) ? NULL : Vinf.framebuf_addr;
	inf->framebuf_size = Vinf.framebuf_total;
	inf->mainmem_size  = (Vinf.v_addr != NULL) ? Vinf.vramsz : 0;

	return E_OK;
}
/*
        screen draw processing
*/
EXPORT	ER	setSCRWRITE(W kind, void *buf, W size)
{
	if (Vinf.fn_write) return (*Vinf.fn_write)(kind, buf, size);
	return E_NOSPT;		/* not supported */
}
