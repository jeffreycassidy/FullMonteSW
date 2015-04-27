#pragma once
#include <immintrin.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <boost/math/constants/constants.hpp>

#include <FullMonte/Kernels/Software/AVXMath/avx_mathfun.h>
#define SFMT_MEXP 19937
#include <FullMonte/SFMT/SFMT.h>

using namespace std;

// RNG wraps Boost Random Number Generation
// RNG_SFMT wraps SFMT by Saito and Matsumoto
//  Provides block generation and buffering for performance, plus commonly-used accessor functions
//  eg. unit vectors, different types, etc
//  Produces its floating-point types by direct bit manipulation of the random number instead of division (faster)

// The size of array (32b words) generated must be at least 624 and a multiple of 4
// Nbuf must be at least 156

template<class T>inline T* posix_allocator(unsigned N_el)
{
	void *p;
	posix_memalign(&p,alignof(T),sizeof(T)*N_el);
	if (!p)
	{
		stringstream ss;
		ss <<  "Failed to return a " << alignof(T) << "B-aligned address when using posix_memalign";
		std::cerr << ss.str() << std::endl;
		throw ss.str();
	}
	return (T*)p;
}

class RNG_SFMT_AVX {
    __m256i * const randBuf;					///< Address of SFMT output buffer
    __m256i * const lastRand;					///< Address of the last random number
    __m256i *nextRand;							///< Location of next random value
    unsigned Nbuf;								///< Buffer size in __m256 units
    sfmt_t sfmt;								///< Matsumoto & Saito SFMT state object

    float		f_pm1[8]__attribute__((aligned(32)));
    float       f_exp[8]__attribute__((aligned(32)));
    float       f_u01[8]__attribute__((aligned(32)));
    float		uv2d[16]__attribute__((aligned(32)));
    float	    uv3d[32]__attribute__((aligned(32)));
    double		d[4] 	__attribute__((aligned(32))) ;
    uint32_t	ui32[8] __attribute__((aligned(32)));

    unsigned char count_f_u01,d_count,ui32_count,count_f_exp,uv2d_count,uv3d_count,exp_count,count_f_pm1;
    
    public:

    static const unsigned int  exp_float24 = 0x40000000;
    static const unsigned int  exp_float =0x3f800000;
    static const uint64_t exp_double_h = 0x3ff0000000000000ULL;

    RNG_SFMT_AVX(unsigned int Nbuf_=4096,unsigned seed_=1) :  randBuf(posix_allocator<__m256i>(Nbuf_/8)),
    		lastRand(randBuf+Nbuf_/8),
    		nextRand(randBuf),
    		Nbuf(Nbuf_),
    		count_f_u01(8),
    		d_count(4),
    		ui32_count(8),
    		count_f_exp(8),
    		uv2d_count(16),
    		uv3d_count(32),
    		exp_count(16),
    		count_f_pm1(8)
    {
    	if ((long long unsigned)this & 0x1F)
    		throw string("RNG_SFMT_AVX: Failed to align on 32B boundary");
    	seed(seed_);
    }

    // requirements for Boost RNG concept
    typedef uint32_t result_type;
    result_type min() const { return std::numeric_limits<uint32_t>::min(); }
    result_type max() const { return std::numeric_limits<uint32_t>::max(); }
    result_type operator()(){ return draw_uint32(); }

    void seed(uint32_t seed_)
    {
    	sfmt_init_gen_rand(&sfmt,seed_);
    	std::cout << "Initialized SFMT RNG with seed of " << seed_ << std::endl;
    	refill();
    }

    void refill();

    inline __m256i draw();

    inline uint32_t draw_uint32();
    //inline double draw_double_u01();
    //inline float  draw_float_u01();

    // Floating-point [0,1)

    inline float draw_float_u01(){ return _mm_cvtss_f32(draw_m128f1_u01()); }
    inline __m128 draw_m128f1_u01();
    inline __m256 draw_m256f8_u01();

    // Floating-point [-1,1)
    inline __m128 draw_m128f1_pm1();
    inline __m256 draw_m256f8_pm1();

    // Unit exponential distribution
    inline __m128 draw_m128f1_exp();

    //inline __m128d draw_m128d1_u01();
    //inline __m128d draw_m128d2_u01();

    // Unit vectors
    inline __m128 draw_m128f2_uvect();
    inline __m128 draw_m128f3_uvect();

    inline pair<__m256,__m256> draw_m256f8_uvect2();

    //inline const uint64_t* draw_u64_2();
    //inline const uint32_t* draw_u32_4();
};

//inline __m128d RNG_SFMT::draw_m128d2_u01()
//{
//    __m128d rnd=_mm_castps_pd(draw().m128_f);
//    __m128d  exp = _mm_castsi128_pd(_mm_set_epi32(exp_double_h,exp_double_l,exp_double_h,exp_double_l));
//    __m128d offs = _mm_set1_pd(1.0);
//    rnd = _mm_castsi128_pd(_mm_srli_epi64(_mm_castpd_si128(rnd),12));
//    rnd = _mm_or_pd(rnd,exp);
//    __m128d res = _mm_sub_pd(rnd,offs);
//    return res;
//}
//
//// pm1 -> plus/minus 1
//

inline __m256i RNG_SFMT_AVX::draw()
{
    if (nextRand == lastRand)
        refill();
    return *(nextRand++);
}


inline __m256 RNG_SFMT_AVX::draw_m256f8_pm1()
{
	//TODO: Can we do this faster with a random sign bit and a U01 variable?
    __m256i rnd=draw();
    __m256i exp = _mm256_set1_epi32(exp_float24);
    __m256 offs = _mm256_set1_ps(3.0);

    rnd = _emu_mm256_srli_epi32(rnd,9);

    __m256 rndf = _mm256_or_ps(_mm256_castsi256_ps(rnd),_mm256_castsi256_ps(exp));
    return _mm256_sub_ps(rndf,offs);
}

inline __m256 RNG_SFMT_AVX::draw_m256f8_u01()
{
    __m256i rnd = draw();
    __m256i  exp = _mm256_set1_epi32(exp_float);
    __m256 offs = _mm256_set1_ps(1.0);

    rnd = _emu_mm256_srli_epi32(rnd,9);
    __m256 rndf = _mm256_or_ps(_mm256_castsi256_ps(rnd),_mm256_castsi256_ps(exp));
    return _mm256_sub_ps(rndf,offs);
}

//// Return a single float
//inline float RNG_SFMT::draw_float_u01()
//{
//    if (f_count == 4)
//    {
//        _mm_store_ps(f,draw_m128f4_u01());
//        f_count=0;
//    }
//    return f[f_count++];
//}


//
//inline __m128d RNG_SFMT::draw_m128d1_u01()
//{
//    if (d_count == 2)
//    {
//        _mm_store_pd(d,draw_m128d2_u01());
//        d_count = 0;
//    }
//    return _mm_load_sd(d+(d_count++));
//}
//
//inline double RNG_SFMT::draw_double_u01()
//{
//    if (d_count == 2)
//    {
//        _mm_store_pd(d,draw_m128d2_u01());
//        d_count=0;
//    }
//    return d[d_count++];
//}
//

inline uint32_t RNG_SFMT_AVX::draw_uint32()
{
    if (ui32_count == 8)
    {
        _mm256_storeu_si256((__m256i*)ui32,draw());
        ui32_count=0;
    }
    return ui32[ui32_count++];
}


// this version returning a scalar is a bit faster than returning a float
inline __m128 RNG_SFMT_AVX::draw_m128f1_u01()
{
    if (count_f_u01 == 8)
    {
        _mm256_store_ps(f_u01,draw_m256f8_u01());
        count_f_u01 = 0;
    }
    return _mm_load_ss(f_u01+(count_f_u01++));
}

inline __m128 RNG_SFMT_AVX::draw_m128f1_pm1()
{
    if (count_f_pm1 == 8)
    {
        _mm256_store_ps(f_pm1,draw_m256f8_pm1());
        count_f_pm1 = 0;
    }
    return _mm_load_ss(f_pm1+(count_f_pm1++));
}


inline __m128 RNG_SFMT_AVX::draw_m128f1_exp()
{
	if (count_f_exp == 8)
	{
		// log (1-r)	r = [0,1) => 1-r = (0,1]
		__m256 l = log_ps(_mm256_sub_ps(_mm256_set1_ps(1.0),draw_m256f8_u01()));

		// store -log(1-r)
		_mm256_store_ps(f_exp,_mm256_sub_ps(_mm256_setzero_ps(),l));
		count_f_exp =0;
	}
	return _mm_load1_ps(f_exp+(count_f_exp++));
}

inline __m128 RNG_SFMT_AVX::draw_m128f2_uvect()
{
	if (uv2d_count == 16)
	{
		__m256 twopi = _mm256_set1_ps(2.0*boost::math::constants::pi<float>());
		__m256 x,y;

		std::make_pair(std::ref(x),std::ref(y)) = sincos_psp(_mm256_mul_ps(twopi,draw_m256f8_u01()));

		_mm256_store_ps(uv2d,  _mm256_unpacklo_ps(x,y));
		_mm256_store_ps(uv2d+8,_mm256_unpackhi_ps(x,y));

		uv2d_count=0;
	}
	__m128 ret = _mm_unpacklo_ps(
				_mm_load_ss(uv2d),
				_mm_load_ss(uv2d+1));
	uv2d_count += 2;
	return ret;
}

inline pair<__m256,__m256> RNG_SFMT_AVX::draw_m256f8_uvect2()
{
	__m256 twopi = _mm256_set1_ps(2.0*boost::math::constants::pi<float>());
	__m256 x,y;

	std::make_pair(std::ref(x),std::ref(y)) = sincos_psp(_mm256_mul_ps(twopi,draw_m256f8_u01()));

	return make_pair(_mm256_unpacklo_ps(x,y), _mm256_unpackhi_ps(x,y));
}

/** Returns a 3D unit vector packed into the lower 3 elements of a __m128 (upper is zero).
 *
 * TODO: It produces unit vectors but distribution has not been thoroughly checked
 *
 * Works by taking a uniform [-1,1) RV for cos(theta), setting sin(theta)=sqrt(1-cos2(theta)),
 * and then rotating that by
 */
inline __m128 RNG_SFMT_AVX::draw_m128f3_uvect()
{
	if(uv3d_count == 32)
	{
		__m256 cosphi = draw_m256f8_pm1();
		__m256 sinphi = _mm256_sqrt_ps(
				_mm256_sub_ps(
						_mm256_set1_ps(1.0),
						_mm256_mul_ps(cosphi,cosphi)));

		__m256 sintheta,costheta;
		__m256 twopi = _mm256_set1_ps(2.0*boost::math::constants::pi<float>());
		std::make_pair(std::ref(sintheta),std::ref(costheta)) = sincos_psp(_mm256_mul_ps(twopi,draw_m256f8_u01()));

		__m256 x = _mm256_mul_ps(sinphi,sintheta);
		__m256 y = _mm256_mul_ps(sinphi,costheta);
		__m256 z = cosphi;
		uv3d_count=0;

		// want to stripe it so we've got [x y z 0] in each element

		// get to [x0 z0 x1 z1], [y0 0 y1 0]
		__m256 a = _mm256_unpacklo_ps(x,z);
		__m256 b = _mm256_unpackhi_ps(x,z);

		__m256 zero = _mm256_setzero_ps();

		__m256 c = _mm256_unpacklo_ps(y,zero);
		__m256 d = _mm256_unpackhi_ps(y,zero);

		// and now to [x0 y0 z0 0]...
		_mm256_store_ps(uv3d   ,_mm256_unpacklo_ps(a,c));
		_mm256_store_ps(uv3d+8 ,_mm256_unpackhi_ps(a,c));
		_mm256_store_ps(uv3d+16,_mm256_unpacklo_ps(b,d));
		_mm256_store_ps(uv3d+24,_mm256_unpackhi_ps(b,d));
	}

	__m128 u = _mm_load_ps(uv3d+uv3d_count);
	uv3d_count += 4;
	return u;
}
