/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/07/28
 *
 *----------------------------------------------------------------------
 */

/*
 *	T-Kernel SVC interface library (x86)
 *
 */

#include <machine.h>
#include <tk/sysdef.h>
#include <sys/svc/fnconsio.h>
#include <sys/svc/svc.h>

define_svc_arg4 console_get CONSIO_CONSOLE_GET_FN
define_svc_arg4 console_put CONSIO_CONSOLE_PUT_FN
define_svc_arg2 console_conf CONSIO_CONSOLE_CONF_FN
define_svc_arg3 console_in CONSIO_CONSOLE_IN_FN
define_svc_arg3 console_out CONSIO_CONSOLE_OUT_FN
define_svc_arg3 console_ctl CONSIO_CONSOLE_CTL_FN
