/*	$OpenBSD: fdopen.c,v 1.6 2008/04/21 12:28:35 otto Exp $ */
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"

FILE *
fdopen_eno(int fd, const char *mode, int *eno)
{
	FILE *fp;
	int flags, oflags, fdflags, tmp, f;

	/* _file is only a short */
	if (fd > SHRT_MAX) {
		/*errno*/f = EMFILE;
		goto e1;
	}

	if ((flags = __sflags(mode, &oflags)) == 0)
		{ f = EINVAL; goto e1; }

	/* Make sure the mode the user wants is a subset of the actual mode. */
	if ((fdflags = __libc_fcntl(fd, F_GETFL, 0)) < 0)
		{ f = fdflags & 0xffff; goto e1; }
	tmp = fdflags & O_ACCMODE;
	if (tmp != O_RDWR && (tmp != (oflags & O_ACCMODE))) {
		/*errno*/f = EINVAL;
		goto e1;
	}

	if ((fp = __sfp()) == NULL)
		{ f = EMFILE; goto e1; }
	fp->_flags = flags;
	/*
	 * If opened for appending, but underlying descriptor does not have
	 * O_APPEND bit set, assert __SAPP so that __swrite() will lseek to
	 * end before each write.
	 */
	if ((oflags & O_APPEND) && !(fdflags & O_APPEND))
		fp->_flags |= __SAPP;
	fp->_file = fd;
	fp->_cookie = fp;
	fp->_read = __sread;
	fp->_write = __swrite;
	fp->_seek = __sseek;
	fp->_close = __sclose;
	return (fp); e1: if (eno) *eno = f; return NULL;
}
FILE *fdopen(int fd, const char *mode){ return fdopen_eno(fd, mode, NULL); }
