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
 *	longlong.h
 *
 *	64 bit length integer
 */

#ifndef _LONGLONG_
#define _LONGLONG_

#ifdef __cplusplus
extern "C" {
#endif

typedef long long	longlong;

#define ltoll(a)	( (longlong)(a) )
#define ultoll(a)	( (longlong)(a) )
#define uitoll(a)	( (longlong)(a) )
#define lltol(a)	( (D)(a) )
#define ll_add(a,b)	( (a) + (b) )
#define ll_sub(a,b)	( (a) - (b) )
#define ll_mul(a,b)	( (a) * (b) )
#define li_mul(a,b)	( (a) * (b) )
#define lui_mul(a,b)	( (a) * (b) )
#define ll_div(a,b)	( (a) / (b) )
#define li_div(a,b)	( (a) / (b) )
#define lui_div(a,b)	( (a) / (b) )
#define ll_mod(a,b)	( (a) % (b) )
#define li_mod(a,b)	( (a) % (b) )
#define lui_mod(a,b)	( (a) % (b) )
#define ll_cmp(a,b)	( (a) - (b) )	/* +:a>b,0:a=b,-:a<b */
#define ll_sign(a)	( (a) )		/* +:a>0,0:a=0,-:a<0 */
#define ll_neg(a)	( -(a) )
#define ll_inc(a)	( (*(a))++ )
#define ll_dec(a)	( (*(a))-- )

#define hilo_ll(ll, h, l)	( (ll) = ((longlong)(h) << 32) | (l) )
#define ll_hilo(h, l, ll)	( (h) = (long)((ll) >> 32), \
				  (l) = (unsigned long)(ll) )

#ifdef __cplusplus
}
#endif
#endif /* _LONGLONG_ */
