/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */

#include <tk/typedef.h>
#include <tk/tkernel.h>
#include <stdint.h>
#include <string.h>

#include <device/std_x86/vga.h>


/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL void clear_screen(void);
void vga_update_timer(void *exinf);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	VRAM		((uint16_t*)0xC00B8000)

#define	COLOR_SHIFT	8
#define	BACK_SHIFT	4

#define	DEFAULT_COLOR	( (VGA_BLACK << BACK_SHIFT) | (VGA_WHITE & 0xFF) )

#define	X_MAX		80
#define	Y_MAX		25

#define	VGA_UPDATE	0x00000001

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL uint32_t x_max;
LOCAL uint32_t y_max;

LOCAL uint32_t pos;
LOCAL uint32_t line_start;

LOCAL uint16_t text_color = DEFAULT_COLOR << COLOR_SHIFT;

LOCAL uint16_t fb[X_MAX * Y_MAX];

#if 0
LOCAL uint32_t update_flag = 0;

LOCAL ID vga_cycid;
LOCAL CONST T_CCYC vga_pk_ccyc = {
	.exinf		= NULL;
	.cycatr		= TA_HLNG | TA_DSNAME,
	.cychdr		= (FP*)vga_update_timer,
	.cyctim		= 100,
	.cycphs		= 0,
	.dsname		= "vga_tim"
};


LOCAL ID vga_flgid;


LOCAL CONST T_CFLG vga_pk_cflg = {
	.exinf		= NULL,
	.figattr	= (TA_PRI | TA_WSGL | TA_DSNAME ),
	.iflgptn	= 0,
	.dsname		= "vga_flg",
};

LOCAL CONST T_CTSK vga_pk_ctsk = {
	.exinf		= NULL,
	.tskattr	= TA_HLNG | TA_SSTKSZ | TA_DSNAME | TA_RNG0,
	.task		= vga_main,
	.itskpri	= 0,
	.sstksz		= 8192,
	.stkptr		= NULL,
	.uatb		= NULL,
	.lsid		= 0,
	.resid		= 0,
	.dsname		= "vga_tsk"
};
#endif
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:initVga
 Input		:void
 Output		:void
 Return		:void
 Description	:initialize vga text mode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void initVga(void)
{
	x_max = X_MAX;
	y_max = Y_MAX;
	
	pos = 0;
	line_start = 0;
	
	memset((void*)fb, 0x00, sizeof(fb));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:startup_vga
 Input		:void
 Output		:void
 Return		:ER
		 < error code >
 Description	:create vga task and start up him
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER startup_vga(void)
{
#if 0
	vga_cycid = tk_cre_cyc(&vga_pk_ccyc);
	
	if (vga_cycid < 0) {
		return((ER)vga_cycid);
	}
	
	vga_flgid = tk_cre_flg(&vga_pk_cflg);
	
	if (vga_flgid < 0) {
		return((ER)vga_flgid);
	}
#endif
	return(E_OK);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vga_out
 Input		:B *buf
		 < buffer to output >
		 UW len
		 < output length >
 Output		:void
 Return		:ER
		 < actual output length or error code >
 Description	:output to vga fb
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT ER vga_out(B *buf, UW len)
{
	int i;
	int line_feed;
	static int page = 0;
	
	for (i = 0;i < len;i++) {
		if (*(buf + i) == '\n') {
			line_feed = x_max - pos % x_max;
			memset(fb + pos, 0x00, line_feed * sizeof(uint16_t));
			pos += line_feed;
			if (page) line_start++;
			if (y_max <= line_start) line_start = 0;
			if (x_max * y_max <= pos) {
				page = 1;
				pos = 0;
				line_start = 1;
			}
			
			memset(fb + pos, 0x00, x_max * sizeof(uint16_t));
			continue;
		}
		
		*(fb + pos++) = (uint16_t)(text_color | *(buf + i));
		
		if (pos && !(pos % x_max)) {
			if (page) line_start++;
			if (y_max <= line_start) line_start = 0;
			if (x_max * y_max <= pos) {
				page = 1;
				pos = 0;
				line_start = 1;
			}
			memset(fb + pos, 0x00, x_max * sizeof(uint16_t));
		}
	}
	if (line_start) {
		
		memcpy((void*)((uint16_t*)VRAM + (y_max - line_start) * x_max),
			fb, line_start * x_max * sizeof(uint16_t));
		memcpy((void*)VRAM, fb + line_start * x_max,
			(y_max - line_start) * x_max * sizeof(uint16_t));
		
	} else {
		memcpy((void*)VRAM , fb, x_max * y_max * sizeof(uint16_t));
	}

	return(len);
}


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:clear_screen
 Input		:void
 Output		:void
 Return		:void
 Description	:clear screen
==================================================================================
*/
LOCAL void clear_screen(void)
{
	memset((void*)VRAM, 0x00, sizeof(fb));
}

/*
==================================================================================
 Funtion	:vga_update_timer
 Input		:void *exinf
		 < extended information >
 Output		:void
 Return		:void
 Description	:timer handler for vga task
==================================================================================
*/
#if 0
void vga_update_timer(void *exinf)
{
	if (update_flag) {
		int i;
		//tk_set_flg(vga_flgid, VGA_UPDATE);
		
		if (line_start) {
			memcpy((void*)((uint16_t*)VRAM + line_start * x_max),
				fb + line_start * x_max, (y_max - line_start) * x_max);
			memcpy((void*)VRAM, fb, line_start * x_max);
		} else {
			memcpy((void*)VRAM , fb, x_max * y_max);
		}
	}
}
#endif
/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
