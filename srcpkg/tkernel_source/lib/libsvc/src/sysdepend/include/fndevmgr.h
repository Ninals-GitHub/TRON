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

#define DEVICE_TK_OPN_DEV_FN	(0x00010200 | DEVICE_SVC)
#define DEVICE_TK_CLS_DEV_FN	(0x00020200 | DEVICE_SVC)
#define DEVICE_TK_REA_DEV_FN	(0x00030500 | DEVICE_SVC)
#define DEVICE_TK_SREA_DEV_FN	(0x00040500 | DEVICE_SVC)
#define DEVICE_TK_WRI_DEV_FN	(0x00050500 | DEVICE_SVC)
#define DEVICE_TK_SWRI_DEV_FN	(0x00060500 | DEVICE_SVC)
#define DEVICE_TK_WAI_DEV_FN	(0x00070500 | DEVICE_SVC)
#define DEVICE_TK_SUS_DEV_FN	(0x00080100 | DEVICE_SVC)
#define DEVICE_TK_GET_DEV_FN	(0x00090200 | DEVICE_SVC)
#define DEVICE_TK_REF_DEV_FN	(0x000a0200 | DEVICE_SVC)
#define DEVICE_TK_OREF_DEV_FN	(0x000b0200 | DEVICE_SVC)
#define DEVICE_TK_LST_DEV_FN	(0x000c0300 | DEVICE_SVC)
#define DEVICE_TK_EVT_DEV_FN	(0x000d0300 | DEVICE_SVC)
#define DEVICE_TK_REA_DEV_DU_FN	(0x000e0500 | DEVICE_SVC)
#define DEVICE_TK_SREA_DEV_D_FN	(0x000f0500 | DEVICE_SVC)
#define DEVICE_TK_WRI_DEV_DU_FN	(0x00100500 | DEVICE_SVC)
#define DEVICE_TK_SWRI_DEV_D_FN	(0x00110500 | DEVICE_SVC)
#define DEVICE_TK_WAI_DEV_U_FN	(0x00120500 | DEVICE_SVC)
#define DEVICE_TK_DEF_DEV_FN	(0x01000300 | DEVICE_SVC)
#define DEVICE_TK_REF_IDV_FN	(0x01010100 | DEVICE_SVC)

