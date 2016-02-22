/*
 * UniformSinDistribution.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_UNIFORMSINDISTRIBUTION_HPP_
#define KERNELS_SOFTWARE_UNIFORMSINDISTRIBUTION_HPP_

/** Returns eight pairs of floats (x, sqrt(1-x*x))
 * Typical use would be as (sin(phi), cos(phi)) where phi is the elevation angle in an isotropic emission pattern.
 *
 */

class FloatUniformSinDistribution : public FloatVectorBase
{
public:
	typedef float					result_type;
	static constexpr std::size_t	OutputsPerInputBlock=8;		///< Outputs generated per invocation
	static constexpr std::size_t	OutputElementSize=2;		///< Number of output elements per input

	typedef uint32_t				input_type;
	static constexpr std::size_t	InputBlockSize=8;			///< Inputs consumed per invocation

	template<class RNG>void calculate(RNG& rng,float *dst)
	{
		const uint32_t* rp = rng.draw();
		__m256i r = _mm256_load_si256((const __m256i*)rp);

		__m256 u01 = ui32ToU01(r);

		__m256 sintheta = u01;
		__m256 costheta = _mm256_sqrt_ps(
								_mm256_sub_ps(
										one(),
										_mm256_mul_ps(u01,u01)));

		_mm256_store_ps(dst,	_mm256_unpacklo_ps(sintheta,costheta));
		_mm256_store_ps(dst+8,	_mm256_unpackhi_ps(sintheta,costheta));
	}
};



#endif /* KERNELS_SOFTWARE_UNIFORMSINDISTRIBUTION_HPP_ */
