/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *
 *----------------------------------------------------------------------
 */


/*
 *	clkdrv.h	Clock driver : common definition
 */

#define	DEBUG_MODULE	"(clkdrv)"

#include <tk/tkernel.h>
#include <device/sdrvif.h>
#include <device/clk.h>
#include <sys/debug.h>

#define	InvalidID	(0)		/* Invalid ID */
#define	InvalidDevID	(-1)		/* Invalid device ID */

/*
 *	Clock driver entry
 */
IMPORT	ER	ClockDrv(INT ac, UB *av[]);

/*
 *	System types (hardware-dependent) routine
 */

/*
 *	Hardware initialization configuration
 */
IMPORT	ER	cdInitHardware(void);
IMPORT	ER	cdFinishHardware(void);

/*
 *	Set/Get the current time
 */
IMPORT	ER	cdSetDateTime(void *date_tim);
IMPORT	ER	cdGetDateTime(void *date_tim);

/*
 *	Set/Get the automatic power-supply ON time
 */
IMPORT	ER	cdSetAutoPwOn(void *date_tim);
IMPORT	ER	cdGetAutoPwOn(void *date_tim);

/*
 *	Access of nonvolatile register
 */
IMPORT	INT	cdSetRegister(void *buf, INT size);
IMPORT	INT	cdGetRegister(void *buf, INT size);
