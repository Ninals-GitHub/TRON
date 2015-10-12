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
 *	tcode.h
 *
 *	TRON character code
 */

#ifndef	__TCODE_H__
#define	__TCODE_H__

/*
 * Control code/special code
 */
#define	TC_NULL		0x0000U		/* Invalid code */
#define TC_TAB		0x0009U		/* Tab */
#define TC_NL		0x000aU		/* Paragraph break */
#define	TC_NC		0x000bU		/* Column break */
#define	TC_FF		0x000cU		/* Page break */
#define TC_CR		0x000dU		/* Line break */
#define	TC_SP		0x0020U		/* Separator */
#define	TC_BLANK	0x00a0U		/* Blank */
#define	TC_LANG		0xfe00U		/* Language specification */
#define	TC_SPEC		0xff00U		/* Special code */
#define	TC_ESC		0xff80U		/* Escape */

/*
 * Typical characters used in system scripts such as symbols and alphanumerics.
 */
#define	TK_KSP		0x2121U		/*	*/
#define	TK_EXCL		0x212aU		/* !	*/
#define	TK_RQOT		0x212eU		/* `	*/
#define	TK_DQOT		0x2149U		/* " 	*/
#define	TK_SHRP		0x2174U		/* #	*/
#define	TK_DLLR		0x2170U		/* $	*/
#define	TK_PCNT		0x2173U		/* %	*/
#define	TK_AND		0x2175U		/* &	*/
#define	TK_SQOT		0x2147U		/* '	*/
#define	TK_LPAR		0x214aU		/* (	*/
#define	TK_RPAR		0x214bU		/* )	*/
#define	TK_ASTR		0x2176U		/* *	*/
#define	TK_PLUS		0x215cU		/* +	*/
#define	TK_CMMA		0x2124U		/* ,	*/
#define	TK_MINS		0x215dU		/* -	*/
#define	TK_PROD		0x2125U		/* .	*/
#define	TK_SLSH		0x213fU		/* /	*/
#define	TK_COLN		0x2127U		/* :	*/
#define	TK_SCLN		0x2128U		/* ;	*/
#define	TK_LSTN		0x2163U		/* <	*/
#define	TK_EQAL		0x2161U		/* =	*/
#define	TK_GTTN		0x2164U		/* >	*/
#define	TK_QSTN		0x2129U		/* ?	*/
#define	TK_ATMK		0x2177U		/* @	*/
#define	TK_LABR		0x214eU		/* [	*/
#define	TK_BSLH		0x216fU		/* \	*/
#define	TK_RABR		0x214fU		/* ]	*/
#define	TK_EXOR		0x2130U		/* ^	*/
#define	TK_USCR		0x2132U		/* _	*/
#define	TK_LCBR		0x2150U		/* {	*/
#define	TK_OR		0x2143U		/* |	*/
#define	TK_RCBR		0x2151U		/* }	*/
#define	TK_TILD		0x2131U		/* ~	*/

#define	TK_AL_A		0x2341U		/* 'A'	*/
#define	TK_AL_X		0x2358U		/* 'X'	*/
#define	TK_AL_Z		0x235aU		/* 'Z'	*/

#define	TK_AL_a		0x2361U		/* 'a'	*/
#define	TK_AL_x		0x2378U		/* 'x'	*/
#define	TK_AL_z		0x237aU		/* 'z'	*/

#define	TK_NU_0		0x2330U		/* '0'	*/
#define	TK_NU_9		0x2339U		/* '9'	*/

#define	TK_KK_S		0x2521U		/* Start of katakana */
#define	TK_KK_E 	0x2576U		/* End of katakana */
#define	TK_HK_S 	0x2421U		/* Start of hiragana */
#define	TK_HK_E 	0x2473U		/* End of hiragana */

#define	TK_DKON		0x212bU		/* Voiced sound symbol		*/
#define	TK_HNDK		0x212cU		/* Semi-voiced sound symbol	*/

#define	TK_0		(TK_NU_0+0)
#define	TK_1		(TK_NU_0+1)
#define	TK_2		(TK_NU_0+2)
#define	TK_3		(TK_NU_0+3)
#define	TK_4		(TK_NU_0+4)
#define	TK_5		(TK_NU_0+5)
#define	TK_6		(TK_NU_0+6)
#define	TK_7		(TK_NU_0+7)
#define	TK_8		(TK_NU_0+8)
#define	TK_9		(TK_NU_0+9)

#define	TK_A		(TK_AL_A+0)
#define	TK_B		(TK_AL_A+1)
#define	TK_C		(TK_AL_A+2)
#define	TK_D		(TK_AL_A+3)
#define	TK_E		(TK_AL_A+4)
#define	TK_F		(TK_AL_A+5)
#define	TK_G		(TK_AL_A+6)
#define	TK_H		(TK_AL_A+7)
#define	TK_I		(TK_AL_A+8)
#define	TK_J		(TK_AL_A+9)
#define	TK_K		(TK_AL_A+10)
#define	TK_L		(TK_AL_A+11)
#define	TK_M		(TK_AL_A+12)
#define	TK_N		(TK_AL_A+13)
#define	TK_O		(TK_AL_A+14)
#define	TK_P		(TK_AL_A+15)
#define	TK_Q		(TK_AL_A+16)
#define	TK_R		(TK_AL_A+17)
#define	TK_S		(TK_AL_A+18)
#define	TK_T		(TK_AL_A+19)
#define	TK_U		(TK_AL_A+20)
#define	TK_V		(TK_AL_A+21)
#define	TK_W		(TK_AL_A+22)
#define	TK_X		(TK_AL_A+23)
#define	TK_Y		(TK_AL_A+24)
#define	TK_Z		(TK_AL_A+25)

#define	TK_a		(TK_AL_a+0)
#define	TK_b		(TK_AL_a+1)
#define	TK_c		(TK_AL_a+2)
#define	TK_d		(TK_AL_a+3)
#define	TK_e		(TK_AL_a+4)
#define	TK_f		(TK_AL_a+5)
#define	TK_g		(TK_AL_a+6)
#define	TK_h		(TK_AL_a+7)
#define	TK_i		(TK_AL_a+8)
#define	TK_j		(TK_AL_a+9)
#define	TK_k		(TK_AL_a+10)
#define	TK_l		(TK_AL_a+11)
#define	TK_m		(TK_AL_a+12)
#define	TK_n		(TK_AL_a+13)
#define	TK_o		(TK_AL_a+14)
#define	TK_p		(TK_AL_a+15)
#define	TK_q		(TK_AL_a+16)
#define	TK_r		(TK_AL_a+17)
#define	TK_s		(TK_AL_a+18)
#define	TK_t		(TK_AL_a+19)
#define	TK_u		(TK_AL_a+20)
#define	TK_v		(TK_AL_a+21)
#define	TK_w		(TK_AL_a+22)
#define	TK_x		(TK_AL_a+23)
#define	TK_y		(TK_AL_a+24)
#define	TK_z		(TK_AL_a+25)

#endif /* __TCODE_H__ */
