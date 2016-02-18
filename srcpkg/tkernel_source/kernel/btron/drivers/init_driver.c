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

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	block devices
----------------------------------------------------------------------------------
*/
INITCALL_DEFINE(init_ramdisk);

/*
----------------------------------------------------------------------------------
	file systems
----------------------------------------------------------------------------------
*/
INITCALL_DEFINE(init_rootfs);

/*
----------------------------------------------------------------------------------
	character devices
----------------------------------------------------------------------------------
*/
INITCALL_DEFINE(init_tty);

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
LOCAL INITCALL driver_init_func[] = {
	/* -------------------------------------------------------------------- */
	/* block devices							*/
	/* -------------------------------------------------------------------- */
	init_ramdisk,
	/* -------------------------------------------------------------------- */
	/* file systems								*/
	/* -------------------------------------------------------------------- */
	init_rootfs,
	/* -------------------------------------------------------------------- */
	/* character devices							*/
	/* -------------------------------------------------------------------- */
	init_tty,
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_drivers
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize drivers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int _INIT_ init_drivers(void)
{
	int i;
	int err;
	
	for (i = 0;i < sizeof(driver_init_func) / sizeof(INITCALL);i++) {
		err = driver_init_func[i]();
		if (err) {
			vd_printf("driver_init_func:error:%d\n", i);
			return(err);
		}
	}
	
	return(0);
}


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
