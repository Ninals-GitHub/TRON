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
 *	float_dummy.c
 *
 *       kernel does not use floating point numbers, and in order to reduce object code size,
 *       we avoid linking libraries for floating point number operations.
 *       We prepare dummies for functions that will use floating point libraries, so that floating point libraries
 *       will not be linked.
 *
 *       * if programs linked with kernel need to use floating point number operations,
 *          not linking this file will cause the real floating point libraries
 *          to be linked in.
 */

/*
 * dummy function to for printing floating point numbers used by printf
 */
int _dtefg( double x, int prec, int fmt,
		int (*putch)(int c, void*), void *out )
{
	return 0;
}
