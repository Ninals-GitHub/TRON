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
 *	@(#)tkn_tun.c
 *
 */

#include <tk/tkernel.h>

#include <sys/_queue.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/systm.h>
#include <sys/conf.h>
//#include <stdlib.h>
long int strtol(const char *nptr, char **endptr, int base);
#if 0
#include <string.h>
#endif
#include <sys/errno.h>
#ifndef T2EX
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif

#include "../tkn.h"

#ifndef NTUN
#define	NTUN	0
#endif

#if NTUN > 0

/* Device major number for TUN. */
#define DEV_TUN		40
extern const struct cdevsw tun_cdevsw;
static const struct cdevsw *d_ops = &tun_cdevsw;
#define getdev(fp)	((dev_t)(fp)->f_data)

static int
devo_read(struct file *fp, off_t *offset, struct uio *uio, kauth_cred_t cred,
    int flags)
{
	(void)offset;
	(void)cred;
	(void)flags;

	return d_ops->d_read(getdev(fp), uio, 0);
}

static int
devo_write(struct file *fp, off_t *offset, struct uio *uio, kauth_cred_t cred,
    int flags)
{
	(void)offset;
	(void)cred;
	(void)flags;

	return d_ops->d_write(getdev(fp), uio, 0);
}

static int
devo_open(dev_t dev, int oflag)
{
	return d_ops->d_open(dev, oflag, 0, NULL);
}

static int
devo_close(struct file *fp)
{
	return d_ops->d_close(getdev(fp), 0, 0, NULL);
}

static int
devo_ioctl(struct file *fp, u_long cmd, void *data)
{
	return d_ops->d_ioctl(getdev(fp), cmd, data, 0, NULL);
}

static int
devo_poll(struct file *fp, int events)
{
	return d_ops->d_poll(getdev(fp), events, NULL);
}

static struct fileops devops = {
	devo_read, devo_write, devo_ioctl,
	(void *)nostop, devo_poll, (void *)nosize,
	devo_close, (void *)nullkqfilter,
	NULL, NULL, NULL
};

int
tkn_tun_open(const char *path, int oflag, struct file *fp)
{
	int error;
	const char tun[] = "/dev/tun";
	const int len = sizeof(tun) - 1;

	if (strncmp(path, tun, len) != 0)
		goto err_invalid_name;

	char *end = NULL;
	int no = strtol(path + len, &end, 10);

	if (end && *end == '\0' && 0 <= no) {
		dev_t dev = makedev(DEV_TUN, no);
		error = devo_open(dev, oflag);
		if (error)
			goto err_open_device;

		memset(fp, 0, sizeof(struct file));

		fp->f_flag = FFLAGS(oflag & O_ACCMODE);
		fp->f_type = DTYPE_MISC;
		fp->f_ops = &devops;
		fp->f_data = (void *)dev;

		return 0;
	}

err_invalid_name:
	return ENXIO;

err_open_device:
	return (error);
}

#else /* NTUN */

int
tkn_tun_open(const char *path, int oflag, struct file *fp)
{
	return ENXIO;
}

#endif /* NTUN */
