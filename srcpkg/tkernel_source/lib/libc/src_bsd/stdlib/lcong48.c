/*	$OpenBSD: lcong48.c,v 1.3 2005/08/08 08:05:36 espie Exp $ */
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





void
lcong48_r(unsigned short p[7], struct rand48_data *buffer)
{
	buffer->seed[0] = p[0];
	buffer->seed[1] = p[1];
	buffer->seed[2] = p[2];
	buffer->mult[0] = p[3];
	buffer->mult[1] = p[4];
	buffer->mult[2] = p[5];
	buffer->add = p[6];
}
