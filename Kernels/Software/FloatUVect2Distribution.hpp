/*
 * FloatUVect2Distribution.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_FLOATUVECT2DISTRIBUTION_HPP_
#define KERNELS_SOFTWARE_FLOATUVECT2DISTRIBUTION_HPP_

#include "FloatVectorBase.hpp"

/** 2D unit vector distribution returning 2 floats per draw.
 *
 */

class FloatUVect2Distribution : public FloatVectorBase
{
public:
	typedef float					result_type;
	static constexpr std::size_t	OutputsPerInputBlock=8;		///< Outputs generated per invocation
	static constexpr std::size_t	OutputElementSize=2;		///< Number of output elements per input

	typedef uint32_t				input_type;
	static constexpr std::size_t	InputBlockSize=8;			///< Inputs consumed per invocation

	template<class RNG>void calculate(RNG& rng,float *dst)
	{
		const uint32_t* rp = rng.getBlock();
		__m256i r = _mm256_load_si256((const __m256i*)rp);

		__m256 theta = _mm256_mul_ps(						// theta = 2*pi*u01
				twopi(),
				_mm256_sub_ps(									// [1,2) - 1.0 -> [0,1)
						_mm256_or_ps(
								_mm256_castsi256_ps(
#ifdef HAVE_AVX2
										_mm256_srli_epi32(r,9)
#else
										_EMU_mm256_srli_epi32(r,9)
#endif
								),
								ldexp(0)),
						one()
				)
				);

		__m256 costheta, sintheta;
		std::tie(sintheta,costheta) = sincos_psp(theta);

		// swizzle (sin_0,cos_0,sin_1,cos_1,...)
		_mm256_store_ps(dst,	_mm256_unpacklo_ps(sintheta,costheta));
		_mm256_store_ps(dst+8,	_mm256_unpackhi_ps(sintheta,costheta));
	}
};





#endif /* KERNELS_SOFTWARE_FLOATUVECT2DISTRIBUTION_HPP_ */
