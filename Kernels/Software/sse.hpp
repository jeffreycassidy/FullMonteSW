#ifndef SSE_INCLUDED
#define SSE_INCLUDED

#include <mmintrin.h>

#include <pmmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

#include <iostream>

// A whole lot of SSE helper routines for small math functions

inline __m128 dot4bcast(__m128 a,__m128 b);
inline __m128 reflect(__m128 d,__m128 n,__m128 sincos);
unsigned getMinIndex3(__m128 v,float* =NULL);
unsigned getMinIndex4(__m128 v,float* =NULL);
inline std::pair<unsigned,__m128> getMinIndex4p(__m128 v);
unsigned getMinAbsIndex3(__m128 v,float* =NULL);
unsigned getMinAbsIndex4(__m128 v,float* =NULL);
inline float selectFrom(__m128 v,unsigned i);
inline __m128 getNormalTo(__m128 d);
inline __m128 normalProjectionUnit(__m128 v,__m128 n);

inline __m128 reflect(__m128 d,__m128 n,__m128 sincos)
{
    // reflect d' = d + 2*(d dot n)*n = d + 2*costheta*n
    __m128 ndot = _mm_shuffle_ps(sincos,sincos,_MM_SHUFFLE(1,1,1,1));
    ndot = _mm_add_ps(ndot,ndot);
    return _mm_add_ps(d,_mm_mul_ps(ndot,n));
}

inline unsigned getMaxIndex4(__m128 v,float *ptr)
{
    __m128 halfmax = _mm_max_ps(v,_mm_shuffle_ps(v,v,_MM_SHUFFLE(2,3,0,1)));
    __m128 allmax  = _mm_max_ps(halfmax,_mm_shuffle_ps(halfmax,halfmax,_MM_SHUFFLE(0,0,2,2)));
    __m128 eqmask = _mm_cmpeq_ps(v,allmax);
    if (ptr)
        _mm_store_ss(ptr,allmax);
    return __builtin_ctzll(_mm_movemask_ps(eqmask));
}

inline unsigned getMinIndex3(__m128 v,float *ptr)
{
	int mask;
    __m128 halfmin = _mm_min_ps(v,_mm_shuffle_ps(v,v,_MM_SHUFFLE(2,2,0,1)));
    __m128 allmin  = _mm_min_ps(halfmin,_mm_shuffle_ps(halfmin,halfmin,_MM_SHUFFLE(0,0,2,2)));
    __m128 eqmask = _mm_cmpeq_ps(v,allmin);
	mask = _mm_movemask_ps(eqmask);
    if (ptr)
        _mm_store_ss(ptr,allmin);
    return mask == 0 ? 3 : __builtin_ctz(mask);
}

inline std::pair<unsigned,__m128> getMinIndex4p(__m128 v)
{
	int mask;
    __m128 halfmin = _mm_min_ps(v,_mm_shuffle_ps(v,v,_MM_SHUFFLE(2,3,0,1)));
    __m128 allmin  = _mm_min_ps(halfmin,_mm_shuffle_ps(halfmin,halfmin,_MM_SHUFFLE(0,0,2,2)));
    __m128 eqmask = _mm_cmpeq_ps(v,allmin);
	mask = _mm_movemask_ps(eqmask);
    return std::make_pair(mask == 0 ? 4 : __builtin_ctz(mask),allmin);
}

inline unsigned getMinIndex4(__m128 v,float* ptr)
{
    __m128 halfmin = _mm_min_ps(v,_mm_shuffle_ps(v,v,_MM_SHUFFLE(2,3,0,1)));
    __m128 allmin  = _mm_min_ps(halfmin,_mm_shuffle_ps(halfmin,halfmin,_MM_SHUFFLE(0,0,2,2)));
    __m128 eqmask = _mm_cmpeq_ps(v,allmin);
    if (ptr)
        _mm_store_ss(ptr,allmin);
    return __builtin_ctzll(_mm_movemask_ps(eqmask));
}

inline unsigned getMinAbsIndex3(__m128 v,float *p){ return getMinIndex3(_mm_mul_ps(v,v),p); }
inline unsigned getMinAbsIndex4(__m128 v,float *p){ return getMinIndex4(_mm_mul_ps(v,v),p); }

inline __m128 selectFrom_v(__m128 v,unsigned i)
{
    if (i>1)
        v = _mm_movehl_ps(v,v);
    if (i&1)
        v = _mm_shuffle_ps(v,v,_MM_SHUFFLE(0,0,0,1));
    return v;
}

inline float selectFrom(__m128 v,unsigned i)
{
    float f;
    if (i>1)
        v = _mm_movehl_ps(v,v);
    if (i&1)
        v = _mm_shuffle_ps(v,v,_MM_SHUFFLE(0,0,0,1));
    _mm_store_ss(&f,v);
    return f;
}

inline __m128 getBasis(unsigned i)
{
    float f[4] = { 0,0,0,0 };
    f[i] = 1;
    return _mm_load_ps(f);
}
//
//inline __m128 getNormalTo(__m128 d)
//{
//    unsigned i=getMinAbsIndex3(d);
//    __m128 b = getBasis(i);
//    return normalize(cross(d,b));
//}


inline __m128 FresnelSSE(__m128 n1n2,__m128 sincos)
{
    // Rs = (n1*costheta_i - n2*costheta_t) / (n1*costheta_i + n2*costheta_t)
    // Rp = (n1*costheta_t - n2*costheta_i) / (n1*costheta_t + n2*costheta_i)

    // n = [n1 n2 n1 n2]
    __m128 n = _mm_movelh_ps(n1n2,n1n2);

    // terms: r0=n1*costheta_i r1=n2*costheta_t r2=n1*costheta_t r3=n2*costheta_i
    __m128 trig = _mm_shuffle_ps(sincos,sincos,_MM_SHUFFLE(1,3,3,1));
    __m128 terms = _mm_mul_ps(trig,n);

    // numden: (n1*cos_i - n2*cos_t) (n2*cos_t + n1*cos_i) (n1*cos_t - n2*cos_i) (n2*cos_i + n1*cos_t)
    __m128 numden = _mm_addsub_ps(terms,_mm_shuffle_ps(terms,terms,_MM_SHUFFLE(2,3,0,1)));

    __m128 rs_rp = _mm_div_ps(_mm_shuffle_ps(numden,numden,_MM_SHUFFLE(0,0,2,0)),_mm_shuffle_ps(numden,numden,_MM_SHUFFLE(0,0,3,1)));

    // calculate rs^2, rp^2
    __m128 rs2_rp2 = _mm_mul_ps(rs_rp,rs_rp);

    // get (rs^2+rp^2) / 2
    return _mm_mul_ss(_mm_hadd_ps(rs2_rp2,rs2_rp2),_mm_set1_ps(0.5));
}

inline double FresnelSSE_d(__m128 n1n2,__m128 sincos)
{
	float f;
	_mm_store_ss(&f,FresnelSSE(n1n2,sincos));
	return f;
}

// sincos r0=sin(theta_i) r1=cos(theta_i)
// returns r0=sin(theta_i) r1=cos(theta_i) r2=sin(theta_t) r3=cos(theta_t)

inline __m128 RefractSSE(__m128 n1_n2_ratio,__m128 cosi)
{
    __m128 one = _mm_set_ss(1.0);
    __m128 sini = _mm_sqrt_ss(_mm_sub_ss(one,_mm_mul_ss(cosi,cosi)));
    __m128 sini_cosi = _mm_unpacklo_ps(sini,cosi);

	__m128 sint = _mm_mul_ss(_mm_movehl_ps(n1_n2_ratio,n1_n2_ratio),sini_cosi);
	__m128 cost = _mm_sqrt_ss(_mm_sub_ss(one,_mm_mul_ss(sint,sint)));

	return _mm_movelh_ps(sini_cosi,_mm_unpacklo_ps(sint,cost));
}

#endif
