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
        mode_string.h   character string for video mode
 *
 */

#include <tcode.h>

#define	TK_MULT	0x215f

LOCAL	TC	cNM_1[] = {TK_COLN, TK_2, TK_5, TK_6, TK_C, TC_NULL};
LOCAL	TC	cNM_2[] = {TK_COLN, TK_6, TK_4, TK_K, TK_C, TC_NULL};
LOCAL	TC	cNM_3[] = {TK_COLN, TK_1, TK_6, TK_M, TK_C, TC_NULL};

LOCAL	TC	sNM_1[] = {TK_2, TK_4, TK_0, TK_MULT,
			   TK_3, TK_2, TK_0, TC_NULL};
LOCAL	TC	sNM_2[] = {TK_6, TK_4, TK_0, TK_MULT,
			   TK_4, TK_8, TK_0, TC_NULL};
LOCAL	TC	sNM_3[] = {TK_8, TK_0, TK_0, TK_MULT,
			   TK_6, TK_0, TK_0, TC_NULL};
LOCAL	TC	sNM_4[] = {TK_1, TK_0, TK_2, TK_4, TK_MULT,
			   TK_7, TK_6, TK_8, TC_NULL};
LOCAL	TC	sNM_5[] = {TK_1, TK_1, TK_5, TK_2, TK_MULT,
			   TK_8, TK_6, TK_4, TC_NULL};
LOCAL	TC	sNM_6[] = {TK_1, TK_2, TK_8, TK_0, TK_MULT,
			   TK_1, TK_0, TK_2, TK_4, TC_NULL};
LOCAL	TC	sNM_7[] = {TK_4, TK_8, TK_0, TK_MULT,
			   TK_6, TK_4, TK_0, TC_NULL};
	
LOCAL	TC	*cLIST[] =
		{cNM_1, cNM_2, cNM_3};
LOCAL	TC	*sLIST[] =
		{sNM_1, sNM_2, sNM_3, sNM_4, sNM_5, sNM_6, sNM_7};
	
LOCAL	UB	OEMName[] = "T-Engine Video Device";
