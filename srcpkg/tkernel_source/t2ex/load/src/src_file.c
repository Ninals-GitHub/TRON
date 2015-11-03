/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
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

/*
 *	src_file.c
 *
 *       T2EX: program load functions
 *       file program source
 */

#include <basic.h>
#include <t2ex/fs.h>
#include "source.h"

/*
 * Read program
 */
LOCAL ER readFileSource(struct LoadSource* ldr, long ofs, void* adr, size_t sz)
{
	size_t pos;
	ER er = 0;

	/* Seek if current offset != ofs */
	if (ofs != ldr->li.file.cur) {
		er = fs_lseek(ldr->li.file.fd, ofs, SEEK_SET);
		if ( er < E_OK ) {
			printf("readFileSource error 01:%d\n", er);
			return er;
		}
		ldr->li.file.cur = ofs;
	}

	/* Read from file */
	for ( pos = 0; pos < sz; ) {
		printf("ldr->li.file.fd:%d adr + pos:0x%08X sz - pos:0x%08X\n", ldr->li.file.fd, (B*)adr + pos, sz - pos);
			
		er = fs_read(ldr->li.file.fd, (B*)adr + pos, sz - pos);
		if ( er <= E_OK ) {
			printf("readFileSource error 02:%d\n", er);
			break;
		}
		pos += er;
		ldr->li.file.cur += er;
	}

	return (pos > 0)? pos: er;
}

/*
 * Quit loading
 */
LOCAL ER closeFileSource(struct LoadSource* ldr)
{
	return fs_close(ldr->li.file.fd);
}

/*
 * Start loading
 */
EXPORT ER openFileSource(struct LoadSource* ldr, const struct pm* prog)
{
	int fd = fs_open((char*)prog->pmhdr, O_RDONLY);
	if ( fd < E_OK ) {
		return fd;
	}

	ldr->type = prog->pmtype;
	ldr->read = readFileSource;
	ldr->close = closeFileSource;
	ldr->li.file.fd = fd;
	ldr->li.file.cur = 0;

	return E_OK;
}
