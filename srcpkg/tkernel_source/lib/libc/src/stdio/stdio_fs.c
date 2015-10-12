/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by T-Engine Forum at 2014/07/08.
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
 *	@(#)stdio_fs.c
 *
 */

#include <basic.h>
#include <t2ex/fs.h>

struct __libc_stat {
	unsigned int	st_mode;
	unsigned int	st_blksize;
	off64_t	st_size;
};

#ifndef	_T2EX
EXPORT	int	__libc_open( const char *path, int oflags, mode_t mode )
{
	return fs_open( path, oflags, mode );
}
#endif

EXPORT	int	__libc_close( int fd )
{
	return fs_close( fd );
}

EXPORT	int	__libc_read( int fd, void *buf, size_t count )
{
	return fs_read( fd, buf, count );
}

EXPORT	int	__libc_write( int fd, const void *buf, size_t count )
{
	return fs_write( fd, buf, count );
}

EXPORT	off64_t	__libc_lseek( int fd, off64_t offset64, int whence )
{
	return fs_lseek64( fd, offset64, whence );
}

#ifndef	_T2EX
EXPORT	ER	__libc_fcntl(int fd, int cmd, int arg)
{
	return fs_fcntl( fd, cmd, arg );
}
#endif

EXPORT	int	__libc_fstat( int fd, struct __libc_stat *buf  )
{
	int	r;
	struct stat64	s;

	r = fs_fstat64( fd, &s );
	if (r < 0) return -1;

	buf->st_mode = s.st_mode;
	buf->st_size = s.st_size;
	buf->st_blksize = s.st_blksize;

	return 0;
}

