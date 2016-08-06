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

#include <bk/drivers/acpica/acpi.h>
#include <bk/drivers/acpica/acpixf.h>
#include <bk/drivers/acpica/actypes.h>
#include <bk/kernel.h>
#include <bk/uapi/errno.h>
#include <cpu.h>
#include <limits.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL int init_acpica(void);
LOCAL ACPI_STATUS
display_one_device(ACPI_HANDLE *handle, UINT32 level, void *context);
LOCAL void display_system_devices(void);

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	ACPI_MAX_TABLES		128

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
 Funtion	:init_acpi
 Input		:void
 Output		:void
 Return		:int
 		 < resutl >
 Description	:initialize acpi driver
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_acpi(void)
{
	int err;
	
	err = init_acpica();
	
	//display_system_devices();
	
	return(err);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:acpi_power_off
 Input		:void
 Output		:void
 Return		:void
 Description	:power off the system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void acpi_power_off(void)
{
	AcpiEnterSleepStatePrep(ACPI_STATE_S5);
	disableInt();
	AcpiEnterSleepState(ACPI_STATE_S5);
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
 Funtion	:init_acpica
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize acpica
==================================================================================
*/
LOCAL int init_acpica(void)
{
	ACPI_STATUS err;
	
	/* -------------------------------------------------------------------- */
	/* initialize the acpica subsystem					*/
	/* -------------------------------------------------------------------- */
	err = AcpiInitializeSubsystem();
	
	if (UNLIKELY(ACPI_FAILURE(err))) {
		printf("cannot initialize acpica subsystem %d\n", err);
		return(-EINVAL);
	}
	
	/* -------------------------------------------------------------------- */
	/* initialize the acpica table manager and get all acpi tables		*/
	/* -------------------------------------------------------------------- */
	err = AcpiInitializeTables(NULL, ACPI_MAX_TABLES, FALSE);
	
	if (UNLIKELY(ACPI_FAILURE(err))) {
		printf("cannot initialize acpica tables\n");
		return(-EINVAL);
	}
	
	/* -------------------------------------------------------------------- */
	/* create the acpi namespace from acpi tables				*/
	/* -------------------------------------------------------------------- */
	err = AcpiLoadTables();
	
	if (UNLIKELY(ACPI_FAILURE(err))) {
		printf("cannot load acpica tables\n");
		return(-EINVAL);
	}
	
	/* -------------------------------------------------------------------- */
	/* initialize the acpi hardware						*/
	/* -------------------------------------------------------------------- */
	err = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	
	if (UNLIKELY(ACPI_FAILURE(err))) {
		printf("cannot enable acpica subsystem\n");
		return(-EINVAL);
	}
	
	/* -------------------------------------------------------------------- */
	/* complete the acpi namespace object initialization			*/
	/* -------------------------------------------------------------------- */
	err = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	
	if (UNLIKELY(ACPI_FAILURE(err))) {
		printf("cannot initialize acpica objects\n");
	}
	printf("finished acpica initialization!!!\n");
	
	return(0);
}
#if 0
/*
==================================================================================
 Funtion	:display_one_device
 Input		:ACPI_HANDLE *handle
 		 < acpi handle >
 		 UINT32 level
 		 < walk level >
 		 void *context
 		 < display context >
 		 void **output
 		 < display output >
 Output		:void *output
 		 < display output >
 Return		:ACPI_STATUS
 		 < result >
 Description	:display device information
==================================================================================
*/
LOCAL ACPI_STATUS
display_one_device(ACPI_HANDLE *handle, UINT32 level, void *context, void *output)
{
	ACPI_STATUS status;
	ACPI_DEVICE_INFO *info;
	ACPI_BUFFER path;
	char buffer[256];
	
	path.Length = sizeof(buffer);
	path.Pointer = buffer;
	
	/* -------------------------------------------------------------------- */
	/* get the full path of the device and output it			*/
	/* -------------------------------------------------------------------- */
	status = AcpiHandleToPathname(handle, &path);
	
	if (LIKELY(ACPI_SUCCESS(status))) {
		printf("%s\n", path.Pointer);
	}
	
	/* -------------------------------------------------------------------- */
	/* get the device info for the device and output it			*/
	/* -------------------------------------------------------------------- */
	status = AcpiGetDeviceInfo(handle, &info);
	
	if (LIKELY(ACPI_SUCCESS(status))) {
		printf("    HID:%.8X, ADR:%.8X, Status:%x\n",
			info.HardwareId, info.Address, info.CurrentStatus);
	}
	
	return(NULL);
}

/*
==================================================================================
 Funtion	:display_system_devices
 Input		:void
 Output		:void
 Return		:void
 Description	:display devices on the system
==================================================================================
*/
LOCAL void display_system_devices(void)
{
	ACPI_HANDLE sys_bus_handle;
	
	AcpiGetHandle(0, ACPI_NS_ROOT_PATH, &sys_bus_handle);
	
	printf("display of all devices in the namespace:\n");
	
	AcpiWalkNamespace(ACPI_TYPE_DEVICE, sys_bus_handle, INT_MAX,
					display_one_device, display_one_device,
					NULL, NULL);
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
