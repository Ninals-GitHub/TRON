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
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

#include <bk/kernel.h>
#include <bk/fs/vfs.h>
#include <bk/drivers/major.h>
#include <bk/uapi/ioctl.h>
#include <bk/uapi/ioctl_tty.h>
#include <bk/uapi/termios.h>

#include <device/std_x86/vga.h>
#include <device/std_x86/kbd.h>

#include "tty.h"
#include "csi.h"

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int tty_open(struct vnode *vnode, struct file *filp);
LOCAL ssize_t tty_read(struct file *filp, char *buf, size_t len, loff_t *ppos);
LOCAL ssize_t
tty_write(struct file *filp, const char *buf, size_t len, loff_t *ppos);
LOCAL void update_pos_cur(struct tty *tty);

/*
----------------------------------------------------------------------------------
	ioctl operations
----------------------------------------------------------------------------------
*/
LOCAL long tty_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
LOCAL int tty_tcgetattr(struct termios *termios);
LOCAL int tty_tcsetattr(struct termios *termios);
LOCAL int tty_tiocgwinsz(struct winsize *ws);
LOCAL int tty_tiocswinsz(struct winsize *ws);

/*
==================================================================================

	DEFINE 

==================================================================================
*/

/*
==================================================================================

	Management 

==================================================================================
*/

/*
----------------------------------------------------------------------------------
	tty termios
----------------------------------------------------------------------------------
*/
LOCAL struct termios2 tty_termios = {
	//.c_iflags = 066402,		// IUTF8 IMAXBEL IXANY IXON ICRNL BRKINT in ubuntu
	.c_iflag = 026402,		// IMAXBEL IXANY IXON ICRNL BRKINT
	.c_oflag = 00005,		// ONLCR OPOST
	.c_cflag = 02277,		// HUPCL CREAD CS8 B38400
	.c_lflag = 0105073,		// IEXTEN ECHOKE ECHOCTL ECHOK ECHOE
					// ECHO ICANON ISIG
					// [CCS]	[ASCII]
	.c_cc = { 0x03,		// 0:VINTR	EXT <end of text>
			0x1C,		// 1:VQUIT	FS  <file separator>
			0x7F,		// 2:VERASE	DEL
			0x15,		// 3:VKILL	NAK
			0x04,		// 4:VEOF	EOT <end of transmission>
			0x00,		// 5:VTIME	ENQ <enquery>
			0x01,		// 6:VMIN	SOH <start of heading>
			0xFF,		// 7:VSWTC
			0x11,		// 8:VSTART	DC1 <device control 1>
			0x13,		// 9:VSTOP	CR
			0x1A,		// 10:VSUSP	SUB/EOF <substitute/EOF>
			0xFF,		// 11:VEOL
			0x12,		// 12:VREPRINT	DC2
			0x0F,		// 13:VDISCARD	SI <shift in>
			0x17,		// 14:VWERASE	ETB <end of tx block>
			0x16,		// 15:VLNEXT	SYN <synchronous idle>
			0xFF,		// 16:VEOL2
			0x00,
			0x00, },
};

/*
----------------------------------------------------------------------------------
	tty window size
----------------------------------------------------------------------------------
*/
LOCAL struct winsize tty_winsize;

/*
----------------------------------------------------------------------------------
	tty management
----------------------------------------------------------------------------------
*/
#if 0
LOCAL uint32_t x_max;
LOCAL uint32_t y_max;
#endif

LOCAL struct tty tty = {
	.termios	= &tty_termios,
	.winsize	= &tty_winsize,
	.text_color	= DEFAULT_COLOR << COLOR_SHIFT,
};

#if 0
LOCAL uint32_t pos;
LOCAL uint32_t line_start;

LOCAL uint16_t text_color = DEFAULT_COLOR << COLOR_SHIFT;

LOCAL uint16_t fb[X_MAX * Y_MAX];
#endif

/*
----------------------------------------------------------------------------------
	tty file operations
----------------------------------------------------------------------------------
*/
LOCAL struct file_operations tty_fops = {
	.open		= tty_open,
	.read		= tty_read,
	.write		= tty_write,
	.unlocked_ioctl	= tty_ioctl,
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_tty
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize a tty driver
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int _INIT_ init_tty(void)
{
	int err;
	
	err = register_char_device(TTYAUX_MAJOR, "tty", &tty_fops);
	
	if (UNLIKELY(err < 0)) {
		return(err);
	}
	
	tty.winsize->ws_row = Y_MAX;
	tty.winsize->ws_col = X_MAX;
	
	tty.pos = 0;
	tty.line_start = 0;
	
	memset((void*)tty.fb, 0x00, sizeof(tty.fb));
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:tty_notify_arrow
 Input		:int arrow
 		 < the type of pressed arrow button >
 Output		:void
 Return		:void
 Description	:notify the event which arrow up is pressed
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void tty_notify_arrow(int arrow)
{
	switch (arrow) {
	case	TTY_ARROW_UP:
		tty.arrow_up = 1;
		break;
	case	TTY_ARROW_DOWN:
		tty.arrow_down = 1;
		break;
	case	TTY_ARROW_RIGHT:
		tty.arrow_right = 1;
		break;
	case	TTY_ARROW_LEFT:
		tty.arrow_left = 1;
		break;
	default:
		break;
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/

/*
----------------------------------------------------------------------------------
	tty file operations
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:tty_open
 Input		:struct vnode *vnode
 		 < vnode object of opening a file >
 		 struct file *filp
 		 < open file object >
 Output		:void
 Return		:int
 		 < result >
 Description	:opne a tty character device file
==================================================================================
*/
LOCAL int tty_open(struct vnode *vnode, struct file *filp)
{
	return(0);
}

/*
==================================================================================
 Funtion	:tty_read
 Input		:struct file *filp
 		 < open file object >
 		 char *buf
 		 < user buffer to read >
 		 size_t len
 		 < lenght to read >
 		 loff_t *ppos
 		 < file offset >
 Output		:char *buf
 		 < user buffer to read >
 		 loff_t *ppos
 		 < file offset >
 Return		:ssize_t
 		 < read result >
 Description	:read from a tty character device
==================================================================================
*/
LOCAL ssize_t tty_read(struct file *filp, char *buf, size_t len, loff_t *ppos)
{
	ssize_t read_len;
	
	if (UNLIKELY(tty.report)) {
		return(read_csi_report(buf, len, &tty));
	}
	
	if (UNLIKELY(tty.arrow_up || tty.arrow_down ||
				tty.arrow_right || tty.arrow_left)) {
		return(read_csi_cursor(filp, buf, len, &tty));
	}
	
	read_len = kbd_in(buf, len);

	if (UNLIKELY(tty.arrow_up || tty.arrow_down ||
				tty.arrow_right || tty.arrow_left)) {
		return(read_csi_cursor(filp, buf, len, &tty));
	}
	
	return(read_len);
}

/*
==================================================================================
 Funtion	:tty_write
 Input		:struct file *filp
 		 < open file object >
 		 const char *buf
 		 < user buffer to write >
 		 size_t len
 		 < write length >
 		 loff_t *ppos
 		 < file offset >
 Output		:loff_t *ppos
 		 < file offset >
 Return		:ssize_t
 		 < actual write size >
 Description	:write to a tty character device
==================================================================================
*/
LOCAL ssize_t
tty_write(struct file *filp, const char *buf, size_t len, loff_t *ppos)
{
	size_t i;
	int line_feed;
	static int page = 0;
	uint32_t y_max = tty.winsize->ws_row;
	uint32_t x_max = tty.winsize->ws_col;
	struct csi_state state = {
		.state		= CSI_NONE,
		.n		= 0,
		.m		= 0,
	};
	int csi_com;
	
	for (i = 0;i < len;i++) {
		csi_com = analyze_csi(buf + i, &state, &tty);
		
		if (UNLIKELY(csi_com)) {
			if (UNLIKELY(csi_com == CSI_ED)) {
			}
			continue;
		}
		
		if (*(buf + i) == '\n') {
			line_feed = x_max - tty.pos % x_max;
			memset(tty.fb + tty.pos, 0x00, line_feed * sizeof(uint16_t));
			tty.pos += line_feed;
			if (page) tty.line_start++;
			if (y_max <= tty.line_start) tty.line_start = 0;
			if (x_max * y_max <= tty.pos) {
				page = 1;
				tty.pos = 0;
				tty.line_start = 1;
			}
			
			memset(tty.fb + tty.pos, 0x00, x_max * sizeof(uint16_t));
			continue;
		}
		
		if (UNLIKELY(*(buf + i) == BACKSPACE)) {
			if (tty.pos) {
				tty.pos--;
			}
			*(tty.fb + tty.pos) = (uint16_t)(tty.text_color | ' ');
		} else if (UNLIKELY((*(buf + i) < ' ') || ('~' < *(buf + i)))) {
			continue;
		} else {
			*(tty.fb + tty.pos++) = (uint16_t)(tty.text_color | *(buf + i));
		}
		
		if (tty.pos && !(tty.pos % x_max)) {
			if (page) tty.line_start++;
			if (y_max <= tty.line_start) tty.line_start = 0;
			if (x_max * y_max <= tty.pos) {
				page = 1;
				tty.pos = 0;
				tty.line_start = 1;
			}
			memset(tty.fb + tty.pos, 0x00, x_max * sizeof(uint16_t));
		}
	}
	if (tty.line_start) {
		memcpy((void*)((uint16_t*)VRAM + (y_max - tty.line_start) * x_max),
			tty.fb, tty.line_start * x_max * sizeof(uint16_t));
		memcpy((void*)VRAM, tty.fb + tty.line_start * x_max,
			(y_max - tty.line_start) * x_max * sizeof(uint16_t));
		
	} else {
		memcpy((void*)VRAM , tty.fb, x_max * y_max * sizeof(uint16_t));
	}
	
	update_pos_cur(&tty);

	return(len);
}

/*
----------------------------------------------------------------------------------
	ioctl operations
----------------------------------------------------------------------------------
*/
/*
==================================================================================
 Funtion	:tty_ioctl
 Input		:struct file *filp
 		 < open file >
 		 unsigned int cmd
 		 < request command >
 		 unsigned long arg
 		 < command arguments >
 Output		:unsigned long arg
 		 < command arguments if in function >
 Return		:long
 		 < result >
 Description	:io control of tty
==================================================================================
*/
LOCAL long tty_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	if (UNLIKELY(_IOC_TYPE(cmd) != 'T')) {
		return(-ENOTTY);
	}
	
	switch (cmd) {
	case	TCGETS:
		err = tty_tcgetattr((struct termios*)arg);
		break;
	case	TCSETS:
		err = tty_tcsetattr((struct termios*)arg);
		break;
	case	TIOCGWINSZ:
		err = tty_tiocgwinsz((struct winsize*)arg);
		break;
	case	TIOCSWINSZ:
		err = tty_tiocswinsz((struct winsize*)arg);
		break;
	default:
		printf("tty_ioctl:currently not implmented req=0x%08X\n", cmd);
		for(;;);
		break;
	}
	
	return(err);
}

/*
==================================================================================
 Funtion	:tty_tcgetattr
 Input		:struct termios *termios
 		 < terminal io settings >
 Output		:struct termios *termios
 		 < terminal io settings >
 Return		:int
 		 < result >
 Description	:get the current serial port settings
==================================================================================
*/
LOCAL int tty_tcgetattr(struct termios *termios)
{
	int i;
	
	if (UNLIKELY(vm_check_accessW((void*)termios, sizeof(struct termios)))) {
		return(-EINVAL);
	}
	
	termios->c_iflag = tty.termios->c_iflag;
	termios->c_oflag = tty.termios->c_oflag;
	termios->c_cflag = tty.termios->c_cflag;
	termios->c_lflag = tty.termios->c_lflag;
	termios->c_line = tty.termios->c_line;
	
	for (i = 0;i < NCCS;i++) {
		termios->c_cc[i] = tty.termios->c_cc[i];
	}
	
	return(0);
}

/*
==================================================================================
 Funtion	:tty_tcsetattr
 Input		:struct termios *termios
 		 < terminal io settings >
 Output		:void
 Return		:int
 		 < result >
 Description	:set the current serial port settings
==================================================================================
*/
LOCAL int tty_tcsetattr(struct termios *termios)
{
	int i;
	
	if (UNLIKELY(vm_check_accessR((void*)termios, sizeof(struct termios)))) {
		return(-EINVAL);
	}
	
	tty.termios->c_iflag = termios->c_iflag;
	tty.termios->c_oflag = termios->c_oflag;
	tty.termios->c_cflag = termios->c_cflag;
	tty.termios->c_lflag = termios->c_lflag;
	tty.termios->c_line = termios->c_line;
	
	//printf("TCSETS:\n");
	//printf("c_iflag:0x%08o c_oflag:0x%08o ", termios->c_iflag, termios->c_oflag);
	//printf("c_cflag:0x%08o c_loflag:0x%08o ", termios->c_cflag, termios->c_lflag);
	//printf("c_lflag:0x%08o c_line:0x%08o\n", termios->c_lflag, termios->c_line);
	//printf("c_cc[]:\n");
	for (i = 0;i < NCCS;i++) {
		tty.termios->c_cc[i] = termios->c_cc[i];
		//printf("%02X ", termios->c_cc[i]);
	}
	//printf("\n");
	
	return(0);
}

/*
==================================================================================
 Funtion	:tty_tiocgwinsz
 Input		:struct winsize *ws
 		 < window size >
 Output		:struct winsize *ws
 		 < window size >
 Return		:int
 		 < result >
 Description	:get window size
==================================================================================
*/
LOCAL int tty_tiocgwinsz(struct winsize *ws)
{
	if (UNLIKELY(vm_check_accessW((void*)ws, sizeof(struct winsize)))) {
		return(-EINVAL);
	}
	
	ws->ws_row = tty.winsize->ws_row;
	ws->ws_col = tty.winsize->ws_col;
	ws->ws_xpixel = 0;
	ws->ws_ypixel = 0;
	
	return(0);
}

/*
==================================================================================
 Funtion	:tty_tiocswinsz
 Input		:struct winsize *ws
 		 < window size >
 Output		:void
 Return		:int
 		 < result >
 Description	:set window size
==================================================================================
*/
LOCAL int tty_tiocswinsz(struct winsize *ws)
{
	if (UNLIKELY(vm_check_accessR((void*)ws, sizeof(struct winsize)))) {
		return(-EINVAL);
	}
	
	tty.winsize->ws_row = ws->ws_row;
	tty.winsize->ws_col = ws->ws_col;
	tty.winsize->ws_xpixel = ws->ws_xpixel;
	tty.winsize->ws_ypixel = ws->ws_ypixel;
	
	return(0);
}

/*
==================================================================================
 Funtion	:update_pos_cur
 Input		:struct tty *tty
 		 < tty management >
 Output		:void
 Return		:void
 Description	:update current cursor position
==================================================================================
*/
LOCAL void update_pos_cur(struct tty *tty)
{
	tty->pos_cur = tty->pos + 1;
	
	if (UNLIKELY((tty->winsize->ws_col * tty->winsize->ws_row) <=
							tty->pos_cur)) {
		tty->pos_cur = 0;
	}
}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
