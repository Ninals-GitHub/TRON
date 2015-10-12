/*	$OpenBSD: _rand48.c,v 1.3 2005/08/08 08:05:36 espie Exp $ */
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
__dorand48_r(unsigned short xseed[3], struct rand48_data *buffer)
{
	unsigned long accu;
	unsigned short temp[2];

	accu = (unsigned long) buffer->mult[0] * (unsigned long) xseed[0] +
	 (unsigned long) buffer->add;
	temp[0] = (unsigned short) accu;	/* lower 16 bits */
	accu >>= sizeof(unsigned short) * 8;
	accu += (unsigned long) buffer->mult[0] * (unsigned long) xseed[1] +
	 (unsigned long) buffer->mult[1] * (unsigned long) xseed[0];
	temp[1] = (unsigned short) accu;	/* middle 16 bits */
	accu >>= sizeof(unsigned short) * 8;
	accu += buffer->mult[0] * xseed[2] + buffer->mult[1] * xseed[1] + buffer->mult[2] * xseed[0];
	xseed[0] = temp[0];
	xseed[1] = temp[1];
	xseed[2] = (unsigned short) accu;
}
