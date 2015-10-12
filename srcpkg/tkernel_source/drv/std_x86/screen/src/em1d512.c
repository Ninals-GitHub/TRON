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
        em1d512.c       screen driver
        video board / chip dependent processing: EM1-D512 LCD controller
 *
 */
#include "screen.h"
#include <sys/segment.h>
#include <device/videomode.h>
#include <device/em1d512_iic.h>

#define	ASMUBase	0xc0110000
#define	PLL2CTRL0	(_UW *)(ASMUBase + 0x008c)
#define	PLL2CTRL1	(_UW *)(ASMUBase + 0x0090)
#define	DIVLCDLCLK	(_UW *)(ASMUBase + 0x0130)
#define	PLL_STATUS	(_UW *)(ASMUBase + 0x0520)

#define	LCDBase		0x40270000
#define	CONTROL		(_UW *)(LCDBase + 0x0000)
#define	QOS		(_UW *)(LCDBase + 0x0004)
#define	DATAREQ		(_UW *)(LCDBase + 0x0008)
#define	LCDOUT		(_UW *)(LCDBase + 0x0010)
#define	BUSSEL		(_UW *)(LCDBase + 0x0014)
#define	STATUS		(_UW *)(LCDBase + 0x0018)
#define	BACKCOLOR	(_UW *)(LCDBase + 0x001c)
#define	AREAADR		(_UW *)(LCDBase + 0x0020)
#define	HOFFSET		(_UW *)(LCDBase + 0x0024)
#define	IFORMAT		(_UW *)(LCDBase + 0x0028)
#define	RESIZE		(_UW *)(LCDBase + 0x002c)
#define	HTOTAL		(_UW *)(LCDBase + 0x0030)
#define	HAREA		(_UW *)(LCDBase + 0x0034)
#define	HEDGE1		(_UW *)(LCDBase + 0x0038)
#define	HEDGE2		(_UW *)(LCDBase + 0x003c)
#define	VTOTAL		(_UW *)(LCDBase + 0x0040)
#define	VAREA		(_UW *)(LCDBase + 0x0044)
#define	VEDGE1		(_UW *)(LCDBase + 0x0048)
#define	VEDGE2		(_UW *)(LCDBase + 0x004c)
#define	INTSTATUS	(_UW *)(LCDBase + 0x0060)
#define	INTRAWSTATUS	(_UW *)(LCDBase + 0x0064)
#define	INTENSET	(_UW *)(LCDBase + 0x0068)
#define	INTENCLR	(_UW *)(LCDBase + 0x006c)
#define	INTFFCLR	(_UW *)(LCDBase + 0x0070)
#define	FRAMECNT	(_UW *)(LCDBase + 0x0074)

#define	GIOBase(x)	(0xc0050000 + 0x00000040 * (x))
#define	GIO_L		0x00
#define	GIO_H		0x01
#define	GIO_HH		0x02
#define	GIO_HHH		0x08
#define	GIO_OL(x)	(_UW *)(GIOBase(x) + 0x0008)
#define	GIO_OH(x)	(_UW *)(GIOBase(x) + 0x000c)

/* definition of LCD panel (including controller-dependent part) */
typedef	struct {
	W	paneltype;	// panel type (if negative, this applies to all the panels)
	UW	pll2ctrl0;	// to configure PLL2 clock frequency
	UW	divlcdlclk;	// LCD_LCLK division

	UH	hde;
	UH	hrs;
	UH	hre;
	UH	htot;
	UH	vde;
	UH	vrs;
	UH	vre;
	UH	vtot;
	B	hsync;
	B	vsync;
} LCDdefs;


#define	SUPPORT_MODEMAP	00020000000	// supported mode map
#define	DEFAULT_REQMODE	DMeWVGAx16

/* LCD parameter */
LOCAL	const LCDdefs	LCDparm[] = {
	{
		// 800x480@@60Hz, 31.5kHz hsync (pixclk=33.3MHz)
		-1, 0x61, 0x51,		// 401.408MHz / 12
		800,  840,  968, 1056,   480,  491,  493,  525,  -1, -1
	},
};

/* send command to DA9052 */
LOCAL	W	WriteDA9052(W reg, W dat)
{
#define	RETRY	4

	W	er, i;
	UB	cmd[2];

	for (i = RETRY; i > 0; i--) {
		cmd[0] = reg << 1;
		cmd[1] = dat;
		er = em1d512_spixfer(0, cmd, cmd, sizeof(cmd));
		if (er >= E_OK) break;
	}

	return er;
}

/* power control of LCD panel */
LOCAL	void	LCDpower(BOOL power)
{
	LOCAL	const UB	on_cmd[] = {
		54, 0x5a,	// VLDO5: 2.5V/enable	*max 3.3V*
		70, 0x27,	// BOOST(2MHz, LED1/2 enable, controller enable)
		71, 0x4f,	// LED_CONT(LED1/2 current sink/ramp enable)
		73, 0xe1,	// LED1_CONF(12034microA)
		74, 0xe1,	// LED2_CONF(12034microA)	
		76, 0xbf,	// LED1_CONT(PWM/100%)
		77, 0xbf,	// LED2_CONT(PWM/100%)
		 0, 0x00,	// (terminate)
	};
	LOCAL	const UB	off_cmd[] = {
		77, 0x00,	// LED2_CONT(default, off)
		76, 0x00,	// LED1_CONT(default, off)
		74, 0x00,	// LED2_CONF(default, 50microA)
		73, 0x00,	// LED1_CONF(default, 50microA)
		71, 0x40,	// LED_CONT(default)
		70, 0x20,	// BOOST(default)
		54, 0x1a,	// VLDO5: 2.5V/disable
		 0, 0x00,	// (terminate)
	};
	const UB	*cmd;

	if (!power) {
		*GIO_OH(GIO_HH) = 0x00040000;	// DISP='0'
		tk_dly_tsk(10);
	}

	cmd = power ? on_cmd : off_cmd;
	while (*cmd) {
		WriteDA9052(*cmd, *(cmd + 1));
		cmd += 2;
	}
	tk_dly_tsk(10);

	if (power) {
		*GIO_OH(GIO_HH) = 0x00040004;	// DISP='1'
		tk_dly_tsk(10);
	}

	return;
}

/* set color map (not supported) */
LOCAL	void	em1_setcmap(COLOR *cmap, W index, W entries)
{
	/* no support, do nothing */
	return;
}

/* search LCD parameter */
LOCAL	const LCDdefs	*find_lcdpar(void)
{
	W	i;
	const LCDdefs	*lcd;

	for (i = 0; i < sizeof(LCDparm) / sizeof(LCDdefs); i++) {
		lcd = &LCDparm[i];

                /* paneltype mismatch */
		if (lcd->paneltype >= 0 &&
		    lcd->paneltype != Vinf.paneltype) continue;

                /* resolution mismatch */
		if (lcd->hde != VideoHsize(Vinf.reqmode) ||
		    lcd->vde != VideoVsize(Vinf.reqmode)) continue;

                /* return parameters */
		return lcd;
	}

        /* not found */
	return NULL;
}

/* set up PLL */
LOCAL	void	pll_setup(UW pll2ctrl0, UW divlcdlclk)
{
	UW	imask;
	W	i;

	DI(imask);

        /* halt PLL2 */
	*PLL2CTRL1 = 0xff;
	for (i = 0; i < 1000000 &&  (*PLL_STATUS & 0x0100); i++) WaitUsec(1);

        /* set up PLL2 clock frequency and LCD_LCLK divisor */
	*PLL2CTRL0 = pll2ctrl0;
	*DIVLCDLCLK = divlcdlclk;

        /* start PLL2 */
	*PLL2CTRL1 = 0;
	for (i = 0; i < 1000000 && !(*PLL_STATUS & 0x0100); i++) WaitUsec(1);

	EI(imask);
	return;
}

/* set display mode */
LOCAL	void	em1_setmode(W flg)
{
	const LCDdefs	*lcd;

        /* search LCD parameter (if not found, do nothing) */
	lcd = find_lcdpar();
	if (lcd == NULL) goto fin0;

        /* display off */
	*LCDOUT = 0;
	LCDpower(FALSE);

        /* termination processing (if blanking only is desired) */
	if (flg < 0) goto fin0;

        /* set up PLL */
	pll_setup(lcd->pll2ctrl0, lcd->divlcdlclk);

        /* set up parameters */
	*BUSSEL = 1;		// MEMC-LCDC mode
	*QOS = 0;
	*CONTROL= ((lcd->hsync < 0) ? 4 : 0) |
		((lcd->vsync < 0) ? 2 : 0);
	*BACKCOLOR = 0;		// background is black (#000000)
	*AREAADR = (UW)Vinf.framebuf_addr;
	*HOFFSET = Vinf.framebuf_rowb;
	*IFORMAT = 1;		// RGB565
	*HAREA = lcd->hde;
	*HEDGE1 = lcd->hrs - lcd->hde;
	*HEDGE2 = lcd->hre - lcd->hde;
	*HTOTAL = lcd->htot;
	*VAREA = lcd->vde;
	*VEDGE1 = lcd->vrs - lcd->vde;
	*VEDGE2 = lcd->vre - lcd->vde;
	*VTOTAL = lcd->vtot;
	*INTENCLR = ~0;		// all interrupts are disabled

        /* clear VRAM content */
	if (flg) memset(Vinf.f_addr, 0, Vinf.framebuf_total);

        /* display on */
	LCDpower(TRUE);
	*LCDOUT = 1;

fin0:
	return;
}

/* suspend / resume processing */
LOCAL	void	em1_suspend(BOOL suspend)
{
        /* in the case of suspend, clear Video-RAM content */
	if (suspend) memset(Vinf.f_addr, 0, Vinf.framebuf_total);

        /* screen display on/off */
	if (suspend) {
		*LCDOUT = 0;
		LCDpower(FALSE);
	} else {
		LCDpower(TRUE);
		*LCDOUT = 1;
	}

	return;
}

/*
        initialization processing

        return == 0:    not the target
                > 0:    target (normal)
                < 0:    target (error)

  1. Looking at Vinf.vendorid, Vinf.deviceid, Vinf.oemname[], decide whether device is the target of
     the driver, and if so, set details in Vinf.chipinf[].

        PCI Config register can be accessed via Vinf.pciaddr.
        Vinf.oemname - bankshift shall contain information using VESA BIOS.


  2. If there are additional functions to augment VESA BIOS, do the following.

        only set up really supported mode map in modemap.
        if curmode != reqmode, it means VESA BIOS is not supported,
          we do the following Even if curmode == reqmode, when we do not use VESA BIOS,
          do similar processing as below.
            set reqmode to curmode
            Configure values according to reqmode in the following places.
                each bit of attr            BPP_24, DACBITS_8, and LINEAR_FRAMEBUF
                                is set. (usually LINEAR_FRAMEBUF is ON )
                framebuf_addr   set physical address of framebuffer.
                framebuf_total  set the total byte counts of the framebuffer.
                framebuf_rowb   set the horizontal byte count of the framebuffer.
        set mode set function to fn_setmode.

        * mode set function is executed later to really to set the mode.
           at this stage of processing, VGA mode (640 x 480 16 colors)
           is in effect.

  3. Depending on the available functions, set the function pointers.

	fn_setmode
                set display mode for real.

		void (*fn_setmode)(W flg)

		flg = 0:
                        set display mode as specified by Vinf.curmode, and Vinf.vfreq, then
                        clear VRAM.
                        by calling setVesaMode(), we can use VESA BIOS to set
                        display mode.
		flg = 1:
                        switch to the vertical scan frequency specified in Vinf.vfreq.
                        do not clear VRAM

                Vinf.vfreq == 0 means using default or not configuring.
                Set the really used vertical frequency to Vinf.vfreq
                If unknown, or unsupported, set this to 0.

	fn_setcmap
                If the standard VGA color pallette is not used,
                set this to the pointer to the function that sets up the VGA color pallette.

		void (*fn_setcmap)(COLOR *cmap, W ix, W nent)

                Select the nent entries starting at the ix-th entry from cmap, and
                and set them into the really used color map. dac8bits == 0 means that DAC is 6 bits, !=0 means
                it has 8 bits resolution.

  4. Check whether the board is correctly configured, and if necessary,
     configure PC Config registers, etc.

*/
EXPORT	W	EM1LCDCInit(void)
{
	W	fbsz, rowb, result = E_NOMEM;

        /* check modemap: if an invalid one is specified, select a useable mode. */
	if (!(SUPPORT_MODEMAP & (1 << Vinf.reqmode))) {
		Vinf.reqmode = DEFAULT_REQMODE;
	}

        /* set memory size to be allocated - 16 bpp assumed. */
	rowb = VideoHsize(Vinf.reqmode) * 2;
	fbsz = rowb * VideoVsize(Vinf.reqmode);

	Vinf.fb_width = VideoHsize(Vinf.reqmode);
	Vinf.fb_height = VideoVsize(Vinf.reqmode);

        /* create framebuffer and mapping */
        /* upper layer software may use slightly additional data, and so reserve such extra. */
	Vinf.f_addr = getPhyMemory(fbsz + 16, &Vinf.framebuf_addr);
	if (Vinf.f_addr == NULL) goto fin0;

        /* set Vinf */
	strncpy(Vinf.chipinf, "EMMA-1 Mobile LCD Controller", L_CHIPINF);
	Vinf.framebuf_total	 = fbsz;
	Vinf.framebuf_rowb	 = rowb;
	Vinf.curmode		 = Vinf.reqmode;
	Vinf.attr		|= (LINEAR_FRAMEBUF | DACBITS_8 |
				    NEED_FINPROC | NEED_SUSRESPROC);
	Vinf.attr		&= ~BPP_24;
	Vinf.fn_setcmap		 = em1_setcmap;
	Vinf.fn_setmode		 = em1_setmode;
	Vinf.fn_susres		 = em1_suspend;
	Vinf.modemap		 = SUPPORT_MODEMAP;

	DP(("EM1LCDCInit(): Vinf.framebuf_addr %#x - %#x (%#x - %#x)\n",
	    Vinf.framebuf_addr, Vinf.framebuf_addr + Vinf.framebuf_total - 1,
	    Vinf.f_addr, Vinf.f_addr + Vinf.framebuf_total - 1));

	result = 1;
fin0:
	return result;
}
