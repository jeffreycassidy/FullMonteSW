/*
 * FloatVectorBase.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_FLOATVECTORBASE_HPP_
#define KERNELS_SOFTWARE_FLOATVECTORBASE_HPP_

#include <immintrin.h>
#include <boost/math/constants/constants.hpp>

// Emulate the AVX2 _mm256_srli_epi32 instruction
inline __m256i _EMU_mm256_srli_epi32(__m256i x,unsigned N)
{
	__m128i h = _mm256_extractf128_si256(x,1);
	__m128i l = _mm256_castsi256_si128(x);

	return _mm256_insertf128_si256(
			_mm256_castsi128_si256(_mm_srli_epi32(h,N)),
			_mm_srli_epi32(l,N),
			1);
}

struct FloatVectorBase
{
private:
    static constexpr uint32_t  	exp_float24 = 0x40000000;		// exponent for float in [2,4)
    static constexpr uint32_t  	exp_float12	= 0x3f800000;		// exponent for float in [1,2)

    static constexpr uint32_t	float_signmask	= 0x80000000;	// 1b  sign
    static constexpr uint32_t	float_expmask	= 0x7f800000;	// 8b  excess-127 exponent
    static constexpr uint32_t	float_mantmask	= 0x007fffff;	// 23b mantissa (implicit leading 1)

public:

    /// Assign a scalar to all elements of the vector
    static inline __m256 broadcast(float x)			{ return _mm256_set1_ps(x);										}
    static inline __m256 broadcastBits(uint32_t u)	{ return _mm256_castsi256_ps(_mm256_set1_epi32(u));				}

    static inline __m256 one()						{ return broadcast(1.0f);										}
    static inline __m256 twopi()					{ return broadcast(boost::math::constants::two_pi<float>());	}
    static inline __m256 pi()						{ return broadcast(boost::math::constants::pi<float>());		}
    static inline __m256 nan()						{ return broadcast(std::numeric_limits<float>::quiet_NaN());	}
    static inline __m256 infinity()					{ return broadcast(std::numeric_limits<float>::infinity());		}
    static inline __m256 zero()						{ return _mm256_setzero_ps();									}

    /// Take absolute value by bitmask
    static inline __m256 abs(__m256 x)
    {
    	return _mm256_andnot_ps(x,broadcastBits(float_signmask));			// zero the sign bit
    }

protected:

    /// Masks and inverse masks for IEEE754 float
    static inline __m256 expmask()					{ return broadcastBits(float_expmask); 							}
    static inline __m256 signmask()					{ return broadcastBits(float_signmask);							}
    static inline __m256 mantmask()					{ return broadcastBits(float_mantmask);							}

    /// Returns a vector filled with 2^p
    static inline __m256 ldexp(int p)				{ return broadcastBits(((p+127)&0xff) << 23);					}

    /// Convert a vector of uint32_t to a vector of [0,1)
    static inline __m256 ui32ToU01(__m256i u)
    {
    	return _mm256_sub_ps(
    				_mm256_or_ps(
    						_mm256_castsi256_ps(
#ifdef HAVE_AVX2
    								_mm256_srli_epi32(u,9)	// random mantissa [1,2) due to implicit leading 1
#else
									_EMU_mm256_srli_epi32(u,9)
#endif
							),
							ldexp(0)),										// set exp s.t. [1,2)
					one());
    }

    /// Convert a vector of uint32_t to [-1,1)
    static inline __m256 ui32ToPM1(__m256i u)
    {
		return _mm256_sub_ps(
					_mm256_or_ps(
						_mm256_castsi256_ps(
#ifdef HAVE_AVX2
								_mm256_srli_epi32(u,9)		// random mantissa [1,2) due to implicit leading 1
#else
								_EMU_mm256_srli_epi32(u,9)
#endif
						),
						ldexp(1)),											// set exp s.t. [2,4)
					broadcast(3.0f));
    }
};

std::ostream& operator<<(std::ostream& os,__m256 x);


#endif /* KERNELS_SOFTWARE_FLOATVECTORBASE_HPP_ */
