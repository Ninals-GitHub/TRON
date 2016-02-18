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

#include <device/std_x86/vga.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int tty_open(struct vnode *vnode, struct file *filp);
LOCAL ssize_t tty_read(struct file *filp, char *buf, size_t len, loff_t *ppos);
LOCAL ssize_t
tty_write(struct file *filp, const char *buf, size_t len, loff_t *ppos);

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

/*
==================================================================================

	Management 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	vga management
----------------------------------------------------------------------------------
*/
LOCAL uint32_t x_max;
LOCAL uint32_t y_max;

LOCAL uint32_t pos;
LOCAL uint32_t line_start;

LOCAL uint16_t text_color = DEFAULT_COLOR << COLOR_SHIFT;

LOCAL uint16_t fb[X_MAX * Y_MAX];

/*
----------------------------------------------------------------------------------
	tty file operations
----------------------------------------------------------------------------------
*/
LOCAL struct file_operations tty_fops = {
	.open	= tty_open,
	.read	= tty_read,
	.write	= tty_write,
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
	
	x_max = X_MAX;
	y_max = Y_MAX;
	
	pos = 0;
	line_start = 0;
	
	memset((void*)fb, 0x00, sizeof(fb));
	
	return(0);
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
	return(0);
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
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
