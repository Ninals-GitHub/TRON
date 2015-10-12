/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2012 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/04.
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

/*
 *	@(#)math.h
 *
 */

#ifndef _MATH_H_
#define	_MATH_H_

#include <basic.h>
#include <limits.h>

#define fpclassify(x) \
	((sizeof(x) == sizeof(double)) ? __fpclassify(x) \
	: (sizeof(x) == sizeof(float)) ? __fpclassifyf(x) \
	: __fpclassifyl(x))
#define isfinite(x) \
	((sizeof(x) == sizeof(double)) ? __isfinite(x) \
	: (sizeof(x) == sizeof(float)) ? __isfinitef(x) \
	: __isfinitel(x))
#define isinf(x) \
	((sizeof(x) == sizeof(double)) ? __isinf(x) \
	: (sizeof(x) == sizeof(float)) ? __isinff(x) \
	: __isinfl(x))
#define isnan(x) \
	((sizeof(x) == sizeof(double)) ? __isnan(x) \
	: (sizeof(x) == sizeof(float)) ? __isnanf(x) \
	: __isnanl(x))
#define isnormal(x) \
	((sizeof(x) == sizeof(double)) ? __isnormal(x) \
	: (sizeof(x) == sizeof(float)) ? __isnormalf(x) \
	: __isnormall(x))
#define signbit(x) \
	((sizeof(x) == sizeof(double)) ? __signbit(x) \
	: (sizeof(x) == sizeof(float)) ? __signbitf(x) \
	: __signbitl(x))

#define	isgreater(x, y)	__builtin_isgreater((x), (y))
#define	isgreaterequal(x, y)	__builtin_isgreaterequal((x), (y))
#define	isless(x, y)	__builtin_isless((x), (y))
#define	islessequal(x, y)	__builtin_islessequal((x), (y))
#define	islessgreater(x, y)	__builtin_islessgreater((x), (y))
#define	isunordered(x, y)	__builtin_isunordered((x), (y))

#define	FP_INFINITE	0x01
#define	FP_NAN	0x02
#define	FP_NORMAL	0x04
#define	FP_SUBNORMAL	0x08
#define	FP_ZERO	0x10
#define FP_ILOGB0	(-INT_MAX)
#define FP_ILOGBNAN	INT_MAX

#define	M_E		2.7182818284590452354	/* e */
#define	M_LOG2E		1.4426950408889634074	/* log 2e */
#define	M_LOG10E	0.43429448190325182765	/* log 10e */
#define	M_LN2		0.69314718055994530942	/* log e2 */
#define	M_LN10		2.30258509299404568402	/* log e10 */
#define	M_PI		3.14159265358979323846	/* pi */
#define	M_PI_2		1.57079632679489661923	/* pi/2 */
#define	M_PI_4		0.78539816339744830962	/* pi/4 */
#define	M_1_PI		0.31830988618379067154	/* 1/pi */
#define	M_2_PI		0.63661977236758134308	/* 2/pi */
#define	M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#define	M_SQRT2		1.41421356237309504880	/* sqrt(2) */
#define	M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */

#define	MAXFLOAT	3.40282347E+38F
#define	HUGE_VAL	__builtin_huge_val()
#define	HUGE_VALF	__builtin_huge_valf()
#define	HUGE_VALL	__builtin_huge_vall()
#define	INFINITY	__builtin_inff()
#define	NAN		__builtin_nanf("")

#ifdef __cplusplus
extern "C" {
#endif

IMPORT	double	acos(double x);
IMPORT	float	acosf(float x);
IMPORT	long double	acosl(long double x);
IMPORT	double	acosh(double x);
IMPORT	float	acoshf(float x);
IMPORT	long double	acoshl(long double x);
IMPORT	double	asin(double x);
IMPORT	float	asinf(float x);
IMPORT	long double	asinl(long double x);
IMPORT	double	asinh(double x);
IMPORT	float	asinhf(float x);
IMPORT	long double	asinhl(long double x);
IMPORT	double	atan(double x);
IMPORT	float	atanf(float x);
IMPORT	long double	atanl(long double x);
IMPORT	double	atan2(double y, double x);
IMPORT	float	atan2f(float y, float x);
IMPORT	long double	atan2l(long double y, long double x);
IMPORT	double	atanh(double x);
IMPORT	float	atanhf(float x);
IMPORT	long double	atanhl(long double x);
IMPORT	double	cbrt(double x);
IMPORT	float	cbrtf(float x);
IMPORT	long double	cbrtl(long double x);
IMPORT	double	ceil(double x);
IMPORT	float	ceilf(float x);
IMPORT	long double	ceill(long double x);
IMPORT	double	copysign(double x, double y);
IMPORT	float	copysignf(float x, float y);
IMPORT	long double	copysignl(long double x, long double y);
IMPORT	double	cos(double x);
IMPORT	float	cosf(float x);
IMPORT	long double	cosl(long double x);
IMPORT	double	cosh(double x);
IMPORT	float	coshf(float x);
IMPORT	long double	coshl(long double x);
IMPORT	double	erf(double x);
IMPORT	float	erff(float x);
IMPORT	long double	erfl(long double x);
IMPORT	double	erfc(double x);
IMPORT	float	erfcf(float x);
IMPORT	long double	erfcl(long double x);
IMPORT	double	exp(double x);
IMPORT	float	expf(float x);
IMPORT	long double	expl(long double x);
IMPORT	double	exp2(double x);
IMPORT	float	exp2f(float x);
IMPORT	long double	exp2l(long double x);
IMPORT	double	expm1(double x);
IMPORT	float	expm1f(float x);
IMPORT	long double	expm1l(long double x);
IMPORT	double	fabs(double x);
IMPORT	float	fabsf(float x);
IMPORT	long double	fabsl(long double x);
IMPORT	double	fdim(double x, double y);
IMPORT	float	fdimf(float x, float y);
IMPORT	long double	fdiml(long double x, long double y);
IMPORT	double	floor(double x);
IMPORT	float	floorf(float x);
IMPORT	long double	floorl(long double x);
IMPORT	double	fma(double x, double y, double z);
IMPORT	float	fmaf(float x, float y, float z);
IMPORT	long double	fmal(long double x, long double y, long double z);
IMPORT	double	fmax(double x, double y);
IMPORT	float	fmaxf(float x, float y);
IMPORT	long double	fmaxl(long double x, long double y);
IMPORT	double	fmin(double x, double y);
IMPORT	float	fminf(float x, float y);
IMPORT	long double	fminl(long double x, long double y);
IMPORT	double	fmod(double x, double y);
IMPORT	float	fmodf(float x, float y);
IMPORT	long double	fmodl(long double x, long double y);
IMPORT	double	frexp(double value, int *exp);
IMPORT	float	frexpf(float value, int *exp);
IMPORT	long double	frexpl(long double value, int *exp);
IMPORT	double	hypot(double x, double y);
IMPORT	float	hypotf(float x, float y);
IMPORT	long double	hypotl(long double x, long double y);
IMPORT	int	ilogb(double x);
IMPORT	int	ilogbf(float x);
IMPORT	int	ilogbl(long double x);
IMPORT	double	j0(double x);
IMPORT	double	j1(double x);
IMPORT	double	jn(int n, double x);
IMPORT	double	ldexp(double x, int exp);
IMPORT	float	ldexpf(float x, int exp);
IMPORT	long double	ldexpl(long double x, int exp);
IMPORT	long long	llrint(double x);
IMPORT	long long	llrintf(float x);
IMPORT	long long	llrintl(long double x);
IMPORT	long long	llround(double x);
IMPORT	long long	llroundf(float x);
IMPORT	long long	llroundl(long double x);
IMPORT	double	log(double x);
IMPORT	float	logf(float x);
IMPORT	long double	logl(long double x);
IMPORT	double	log10(double x);
IMPORT	float	log10f(float x);
IMPORT	long double	log10l(long double x);
IMPORT	double	log1p(double x);
IMPORT	float	log1pf(float x);
IMPORT	long double	log1pl(long double x);
IMPORT	double	log2(double x);
IMPORT	float	log2f(float x);
IMPORT	long double	log2l(long double x);
IMPORT	double	logb(double x);
IMPORT	float	logbf(float x);
IMPORT	long double	logbl(long double x);
IMPORT	long	lrint(double x);
IMPORT	long	lrintf(float x);
IMPORT	long	lrintl(long double x);
IMPORT	long	lround(double x);
IMPORT	long	lroundf(float x);
IMPORT	long	lroundl(long double x);
IMPORT	double	modf(double value, double *iptr);
IMPORT	float	modff(float value, float *iptr);
IMPORT	long double	modfl(long double value, long double *iptr);
IMPORT	double	nan(const char *tagp);
IMPORT	float	nanf(const char *tagp);
IMPORT	long double	nanl(const char *tagp);
IMPORT	double	nearbyint(double);
IMPORT	float		nearbyintf(float);
IMPORT	long double	nearbyintl(long double);
IMPORT	double	nextafter(double x, double y);
IMPORT	float	nextafterf(float x, float y);
IMPORT	long double	nextafterl(long double x, long double y);
IMPORT	double	nexttoward(double x, long double y);
IMPORT	float	nexttowardf(float x, long double y);
IMPORT	long double	nexttowardl(long double x, long double y);
IMPORT	double	pow(double x, double y);
IMPORT	float	powf(float x, float y);
IMPORT	long double	powl(long double x, long double y);
IMPORT	double	remainder(double x, double y);
IMPORT	float	remainderf(float x, float y);
IMPORT	long double	remainderl(long double x, long double y);
IMPORT	double	remquo(double x, double y, int *quo);
IMPORT	float	remquof(float x, float y, int *quo);
IMPORT	long double	remquol(long double x, long double y, int *quo);
IMPORT	double	rint(double x);
IMPORT	float	rintf(float x);
IMPORT	long double	rintl(long double x);
IMPORT	double	round(double x);
IMPORT	float	roundf(float x);
IMPORT	long double	roundl(long double x);
IMPORT	double	scalbln(double x, long n);
IMPORT	float	scalblnf(float x, long n);
IMPORT	long double	scalblnl(long double x, long n);
IMPORT	double	scalbn(double x, int n);
IMPORT	float	scalbnf(float x, int n);
IMPORT	long double	scalbnl(long double x, int n);
IMPORT	double	sin(double x);
IMPORT	float	sinf(float x);
IMPORT	long double	sinl(long double x);
IMPORT	double	sinh(double x);
IMPORT	float	sinhf(float x);
IMPORT	long double	sinhl(long double x);
IMPORT	double	sqrt(double x);
IMPORT	float	sqrtf(float x);
IMPORT	long double	sqrtl(long double x);
IMPORT	double	tan(double x);
IMPORT	float	tanf(float x);
IMPORT	long double	tanl(long double x);
IMPORT	double	tanh(double x);
IMPORT	float	tanhf(float x);
IMPORT	long double	tanhl(long double x);
IMPORT	double	tgamma(double x);
IMPORT	float	tgammaf(float x);
IMPORT	long double	tgammal(long double x);
IMPORT	double	trunc(double x);
IMPORT	float	truncf(float x);
IMPORT	long double	truncl(long double x);
IMPORT	double	y0(double x);
IMPORT	double	y1(double x);
IMPORT	double	yn(int n, double x);
IMPORT	double	lgamma_r(double x, int* signp);
IMPORT	float	lgammaf_r(float x, int* signp);
IMPORT	long double	lgammal_r(long double x, int* signp);

IMPORT	int	__fpclassify(double);
IMPORT	int	__fpclassifyf(float);
IMPORT	int	__fpclassifyl(long double);
IMPORT	int	__isfinite(double);
IMPORT	int	__isfinitef(float);
IMPORT	int	__isfinitel(long double);
IMPORT	int	__isinf(double);
IMPORT	int	__isinff(float);
IMPORT	int	__isinfl(long double);
IMPORT	int	__isnan(double);
IMPORT	int	__isnanf(float);
IMPORT	int	__isnanl(long double);
IMPORT	int	__isnormal(double);
IMPORT	int	__isnormalf(float);
IMPORT	int	__isnormall(long double);
IMPORT	int	__signbit(double);
IMPORT	int	__signbitf(float);
IMPORT	int	__signbitl(long double);

#ifdef __cplusplus
}
#endif
#endif /* _MATH_H_ */

