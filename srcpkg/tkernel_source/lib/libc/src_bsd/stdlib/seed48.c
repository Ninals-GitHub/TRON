/*	$OpenBSD: seed48.c,v 1.3 2005/08/08 08:05:37 espie Exp $ */
/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */

#include "rand48.h"





unsigned short *
seed48_r(unsigned short xseed[3], struct rand48_data *buffer, unsigned short sseed[3])
{


	sseed[0] = buffer->seed[0];
	sseed[1] = buffer->seed[1];
	sseed[2] = buffer->seed[2];
	buffer->seed[0] = xseed[0];
	buffer->seed[1] = xseed[1];
	buffer->seed[2] = xseed[2];
	buffer->mult[0] = RAND48_MULT_0;
	buffer->mult[1] = RAND48_MULT_1;
	buffer->mult[2] = RAND48_MULT_2;
	buffer->add = RAND48_ADD;
	return sseed;
}
