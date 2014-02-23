#include <immintrin.h>
#include <stdlib.h>

//#include "sse.hpp"

#include <string>

#include <boost/math/constants/constants.hpp>

#define SFMT_MEXP 19937
#include "SFMT.h"

#include "avx_mathfun.h"

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

    float		__attribute__((aligned(32))) f_pm1[8];
    float       __attribute__((aligned(32))) f_exp[8];
    float       __attribute__((aligned(32))) f_u01[8];
    //double      __attribute__((aligned(32))) d[4];
    float		__attribute__((aligned(32))) uv2d[16];
    float		__attribute__((aligned(32))) exprv[8];

    unsigned char count_f_u01,d_count,i32_count,count_f_exp,uv2d_count,exp_count,count_f_pm1;
    
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
    		i32_count(8),
    		count_f_exp(8),
    		uv2d_count(16),
    		exp_count(16),
    		count_f_pm1(8)
    {
    	if ((unsigned long long)randBuf % 32 != 0)
    	{
    		std::cerr << "Failed to return a 32-byte aligned address when using new[] for __m256" << std::endl;
    		throw std::string("Whoa, baby horse!");
    	}
    	std::cout << "Initialized SFMT RNG with seed of " << seed_ << std::endl;
    	sfmt_init_gen_rand(&sfmt,seed_);
    	refill();
    }

    // requirements for Boost RNG concept
    /*typedef uint32_t result_type;
    result_type min() const { return std::numeric_limits<uint32_t>::min(); }
    result_type max() const { return std::numeric_limits<uint32_t>::max(); }
    result_type operator()(){ return draw_uint32(); }*/

    void refill();

    inline __m256i draw();

    //inline uint32_t draw_uint32();
    //inline double draw_double_u01();
    //inline float  draw_float_u01();
    //inline __m128 draw_m128f3_u01();
    //inline __m128 draw_m128f4_u01();

    // Floating-point [0,1)
    inline __m128 draw_m128f1_u01();
    inline __m256 draw_m256f8_u01();

    // Floating-point [-1,1)
    inline __m128 draw_m128f1_pm1();
    inline __m256 draw_m256f8_pm1();

    // Unit exponential distribution
    inline __m128 draw_m128f1_exp();

    //inline __m128d draw_m128d1_u01();
    //inline __m128d draw_m128d2_u01();

    //inline __m128 draw_m128f4_pm1();

    // Unit vectors
    inline __m128 draw_m128f2_uvect();
    //inline __m128 draw_m128f3_uvect();

    //inline __m128 draw_m128f3_uvect();

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

void RNG_SFMT_AVX::refill()
{
    nextRand = randBuf;
    sfmt_fill_array32(&sfmt,(uint32_t*)randBuf,Nbuf);
}

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

    rnd = _mm256_srli_epi32(rnd,9);

    __m256 rndf = _mm256_or_ps(_mm256_castsi256_ps(rnd),_mm256_castsi256_ps(exp));
    return _mm256_sub_ps(rndf,offs);
}

inline __m256 RNG_SFMT_AVX::draw_m256f8_u01()
{
    __m256i rnd = draw();
    __m256i  exp = _mm256_set1_epi32(exp_float);
    __m256 offs = _mm256_set1_ps(1.0);

    rnd = _mm256_srli_epi32(rnd,9);
    __m256 rndf = _mm256_or_ps(_mm256_castsi256_ps(rnd),_mm256_castsi256_ps(exp));
    return _mm256_sub_ps(rndf,offs);
}

//inline __m128 RNG_SFMT::draw_m128f3_u01()
//{
//    __m128 rnd = draw().m128_f;
//    __m128  exp = __m128(_mm_set_epi32(0,exp_float,exp_float,exp_float));
//    __m128 offs = _mm_set_ps(0,1.0,1.0,1.0);
//    rnd = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(rnd),9));
//    rnd = _mm_or_ps(rnd,exp);
//    rnd = _mm_sub_ps(rnd,offs);
//    return rnd;
//}
//
//__m128 RNG_SFMT::draw_m128f1_log_u01()
//{
//    __m128 r,l;
//    if (f_log_count == 4)
//    {
//        r = _mm_sub_ps(_mm_set1_ps(1.0),draw_m128f4_u01());     // 1 - [0,1) => (0,1] to avoid -Inf
//        l = log_ps(r);
//        _mm_store_ps(f_log,l);
//        f_log_count=0;
//    }
//    return _mm_load1_ps(f_log+(f_log_count++));
//}
//
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
//inline uint32_t RNG_SFMT::draw_uint32()
//{
//    if (i32_count == 4)
//    {
//        _mm_store_ps(i32.f,draw().m128_f);
//        i32_count=0;
//    }
//    return i32.i32[i32_count++];
//}
//

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

inline __m128 RNG_SFMT_AVX::draw_m128f1_exp()
{
	if (count_f_exp == 8)
	{
		_mm256_store_ps(f_exp,_mm256_abs_ps(log_ps(draw_m256f8_u01())));
		count_f_exp =0;
	}

	return _mm_load_ss(f_exp+(count_f_exp++));
}

inline __m128 RNG_SFMT_AVX::draw_m128f2_uvect()
{
	if (uv2d_count == 16)
	{
		__m256 twopi = _mm256_set1_ps(2.0*boost::math::constants::pi<float>());
		__m256 x,y;

		std::make_pair(std::ref(x),std::ref(y)) = sincos_psp(_mm256_mul_ps(twopi,draw_m256f8_u01()));

		_mm256_store_ps(uv2d,  _mm256_unpackhi_ps(x,y));
		_mm256_store_ps(uv2d+8,_mm256_unpacklo_ps(x,y));

		uv2d_count=0;
	}
	uv2d_count += 2;

	return _mm_unpacklo_ps(
				_mm_load_ss(uv2d+uv2d_count-2),
				_mm_load_ss(uv2d+uv2d_count-1));
}

//
//inline __m128 RNG_SFMT::draw_m128f3_uvect()
//{
//    __m128 ones = _mm_set1_ps(1.0);
//    __m128 rnd,norm2,rnd2,twice_rnd;
//    unsigned mask;
//
//    do {
//        rnd = draw_m128f4_pm1();
//        rnd2 = _mm_mul_ps(rnd,rnd);
//        norm2 = _mm_hadd_ps(rnd2,rnd2);
//    }
//    while(!(mask=_mm_movemask_ps(_mm_cmpgt_ps(ones,norm2))&3));
//
//    if (mask & 2)
//        rnd = _mm_shuffle_ps(rnd,norm2,_MM_SHUFFLE(3,3,3,2));
//    else
//        rnd = _mm_shuffle_ps(rnd,norm2,_MM_SHUFFLE(2,2,1,0));
//
//    // now rnd  = x1  x2  (x1^2 + x2^2)  (x1^2 + x2^2)
//
//    twice_rnd = _mm_add_ps(rnd,rnd);
//    // and 2rnd = 2(x1)  2(x2)  2(x1^2 + x2^2)  2(x1^2 + x2^2)
//
//    __m128 one_minus = _mm_movehl_ps(_mm_setzero_ps(),_mm_sqrt_ps(_mm_sub_ps(ones,rnd)));
//    // one_minus = sqrt(1-x1^2-x2^2) sqrt(1-x1^2-x2^2)  0  0
//
//    __m128 xy = _mm_mul_ps(one_minus,twice_rnd);
//
//    // xy = 2(x1)sqrt(1-x1^2-x2^2) 2(x2)sqrt(1-x1^2-x2^2) 0 0
//
//    return _mm_shuffle_ps(xy,_mm_sub_ss(_mm_set_ss(1.0),_mm_movehl_ps(twice_rnd,twice_rnd)),_MM_SHUFFLE(3,0,1,0));
//}
