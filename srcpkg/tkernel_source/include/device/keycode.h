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
 *	keycode.h
 *
 *       keyboard : keycode definitions
 */

#ifndef	__DEVICE_KEYCODE_H__
#define	__DEVICE_KEYCODE_H__

/* meta key code */
#define KC_EIJI 	0x1000		/* English <-> Japanese swap */
#define KC_CAPN 	0x1001		/* Hiragana <-> Katakana swap */
#define KC_SHT_R	0x1002		/* shift right          */
#define KC_SHT_L	0x1003		/* shift left          */
#define KC_EXP		0x1004		/* expansion            */
#define KC_CMD		0x1005		/* command            */
#define KC_JPN0 	0x1006		/* Japanese Hiragana                */
#define KC_JPN1 	0x1007		/* Japanese Katanaka */
#define KC_ENG0 	0x1008		/* English                    */
#define KC_ENG1 	0x1009		/* English CAPS                */
#define	KC_KBSEL	0x100a		/* Kana <-> Roman input swap      */
#define	KC_ENGALT	0x100b		/* -> English <-->English CAPS  */
#define	KC_JPNALT	0x100c		/* -> Hiragana <-> Katakana       */

#define	KC_HAN		0x1150		/* Zenkaku <-> Hankaku switch      */
#define KC_JPN0_Z 	0x1016		/* Japanese Hiragana & Zenkaku   */
#define KC_JPN1_Z 	0x1017		/* Japanese Katakana & Zenkaku    */
#define KC_ENG0_H 	0x1018		/* English & Hankaku           */
#define KC_ENG1_H 	0x1019		/* English CAPS & Hankaku  */

/* key code for PD simulation */
#define KC_HOME		0x1245		/* Home			*/
#define KC_PGUP		0x1246		/* PageUp		*/
#define KC_PGDN		0x1247		/* PageDown		*/
#define KC_END		0x125e		/* End			*/

#define KC_CC_U 	0x0100		/* main CC UP   */
#define KC_CC_D 	0x0101		/* main CC DOWN    */
#define KC_CC_R 	0x0102		/* main CC RIGHT    */
#define KC_CC_L 	0x0103		/* main CC LEFT    */

#define KC_SC_U 	0x0104		/* sub CC key UP  */
#define KC_SC_D 	0x0105		/* sub CC key DOWN  */
#define KC_SC_R 	0x0106		/* sub CC key RIGHT  */
#define KC_SC_L 	0x0107		/* sub CC LEFT  */

#define KC_SS_U 	0x0108		/* scroll key UP    */
#define KC_SS_D 	0x0109		/* scroll key DOWN    */
#define KC_SS_R 	0x010a		/* scroll key RIGHT   */
#define KC_SS_L 	0x010b		/* scroll key LEFT   */

#define KC_PG_U 	0x010c		/* page key UP */
#define KC_PG_D 	0x010d		/* page key DOWN */
#define KC_PG_R 	0x010e		/* page key RIGHT */
#define KC_PG_L 	0x010f		/* page key LEFT */

/* BREAK key code */
#define	BREAK_CODE	0x1140		/* [command] + [delete]    */

/* INSERT key code */
#define	INS_CODE	0x114c		/* insert                     */

/* other special keys */
#define	KC_NUL		0x0000		/* NUL key          */
#define	KC_IEND		0x0004		/* input end	    */
#define	KC_BS		0x0008		/* one character back space       */
#define	KC_TAB		0x0009		/* tab                    */
#define	KC_NL		0x000a		/* change paragraph          */
#define	KC_CR		0x000d		/* newline                  */
#define	KC_CAN		0x0018		/* cancel */
#define	KC_ASSIST	0x001b		/* auxiliary support, assistance  */
#define	KC_CNV		0x001e		/* conversion */
#define	KC_RCNV		0x001f		/* reverse conversion (no conversion)   */
#define	KC_DEL		0x007f		/* deletion                  */
#define KC_CNV0		0x1151		/* no conversion (Hiragana conversion) */
#define KC_CNV1		0x1152		/* SHIFT+no conversion (Katakana conversion) */
#define KC_SPACE	0x2121		/* space            */

#define	KC_PRSCRN	0x1148		/* Print Screnn		*/

#define	KC_PF1		0x1161		/* PF1			*/
#define	KC_PF2		0x1162		/* PF2			*/
#define	KC_PF3		0x1163		/* PF3			*/
#define	KC_PF4		0x1164		/* PF4			*/
#define	KC_PF5		0x1165		/* PF5			*/
#define	KC_PF6		0x1166		/* PF6			*/
#define	KC_PF7		0x1167		/* PF7			*/
#define	KC_PF8		0x1168		/* PF8			*/
#define	KC_PF9		0x1169		/* PF9			*/
#define	KC_PF10		0x116a		/* PF10			*/
#define	KC_PF11		0x116b		/* PF11			*/
#define	KC_PF12		0x116c		/* PF12			*/

/*
 * Values from 0x1161 to 0x11ff are reserved as function keys
 * Values from 0x1300 to 0x1fff are reserved for user application (for any purpose)
 * other undefined area is reserved for system use
 */

#endif /* __DEVICE_KEYCODE_H__ */
