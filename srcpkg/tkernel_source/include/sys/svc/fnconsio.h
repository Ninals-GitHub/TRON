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
 *	Extended SVC function code
 *
 */

#include <sys/ssid.h>

#define CONSIO_CONSOLE_GET_FN	(0x00100400 | CONSIO_SVC)
#define CONSIO_CONSOLE_PUT_FN	(0x00110400 | CONSIO_SVC)
#define CONSIO_CONSOLE_CONF_FN	(0x00120200 | CONSIO_SVC)
#define CONSIO_CONSOLE_IN_FN	(0x00200300 | CONSIO_SVC)
#define CONSIO_CONSOLE_OUT_FN	(0x00210300 | CONSIO_SVC)
#define CONSIO_CONSOLE_CTL_FN	(0x00220300 | CONSIO_SVC)

