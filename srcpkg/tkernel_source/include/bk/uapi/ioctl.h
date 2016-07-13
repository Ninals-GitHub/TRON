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

#ifndef	__BK_UAPI_IOCTL_H__
#define	__BK_UAPI_IOCTL_H__

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	ioctl command format
----------------------------------------------------------------------------------
*/
/* ioctl command structure:
    bits 30-31	direction
    bits 16-29	parameter length
    bits  8-15	cmd group
    bits  0-7	command
 */
#define	_IOC_NR_BITS	8
#define	_IOC_TYPE_BITS	8
#define	_IOC_SIZE_BITS	14
#define	_IOC_DIR_BITS	2

#define	_IOC_NR_MASK	((1 << _IOC_NR_BITS	) - 1)
#define	_IOC_TYPE_MASK	((1 << _IOC_TYPE_BITS	) - 1)
#define	_IOC_SIZE_MASK	((1 << _IOC_SIZE_BITS	) - 1)
#define	_IOC_DIR_MASK	((1 << _IOC_DIR_BITS	) - 1)

#define	_IOC_NR_SHIFT	(0)
#define	_IOC_TYPE_SHIFT	(_IOC_NR_SHIFT   + _IOC_NR_BITS)
#define	_IOC_SIZE_SHIFT	(_IOC_TYPE_SHIFT + _IOC_TYPE_BITS)
#define	_IOC_DIR_SHIFT	(_IOC_SIZE_SHIFT + _IOC_SIZE_BITS)

#define	_IOC(dir, type, nr, size)	(((dir) << _IOC_DIR_SHIFT)	|	\
					((type) << _IOC_TYPE_SHIFT)	|	\
					((nr) << _IOC_NR_SHIFT)		|	\
					((size) << _IOC_SIZE_SHIFT))


#define	_IOC_NONE	(0x0)
#define	_IOC_WRITE	(0x1)
#define	_IOC_READ	(0x2)

#define	_IO(type, nr)		_IOC(_IOC_NONE, type, nr, 0)
#define	_IOR(type, nr, size)	_IOC(_IOC_READ, type, nr, sizeof(size))
#define	_IOW(type, nr, size)	_IOC(_IOC_WRITE, type, nr, sizeof(size))
#define	_IOWR(type, nr, size)	_IOC(_IOC_WRITE | _IOC_READ, type, nr, sizeof(size))

#define	_IOC_MASK(req, shift, mask)	(((req) >> shift) & mask)
#define	_IOC_NR(req)		_IOC_MASK(req, _IOC_NR_SHIFT, _IOC_NR_MASK)
#define	_IOC_TYPE(req)		_IOC_MASK(req, _IOC_TYPE_SHIFT, _IOC_TYPE_MASK)
#define	_IOC_SIZE(req)		_IOC_MASK(req, _IOC_SIZE_SHIFT, _IOC_SIZE_MASK)
#define	_IOC_DIR(req)		_IOC_MASK(req, _IOC_NR_DIR, _IOC_DIR_MASK)


/*
==================================================================================

	Management 

==================================================================================
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BK_UAPI_IOCTL_H__
