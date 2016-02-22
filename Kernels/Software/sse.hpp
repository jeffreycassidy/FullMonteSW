#ifndef SSE_INCLUDED
#define SSE_INCLUDED

#include <mmintrin.h>

#include <iostream>

inline __m128 reflect(__m128 d,__m128 n,__m128 sincos);
unsigned getMinIndex4(__m128 v,float* =NULL);
inline std::pair<unsigned,__m128> getMinIndex4p(__m128 v);

inline __m128 reflect(__m128 d,__m128 n,__m128 sincos)
{
    // reflect d' = d + 2*(d dot n)*n = d + 2*costheta*n
    __m128 ndot = _mm_shuffle_ps(sincos,sincos,_MM_SHUFFLE(1,1,1,1));
    ndot = _mm_add_ps(ndot,ndot);
    return _mm_add_ps(d,_mm_mul_ps(ndot,n));
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
