/*
 * FloatUnitExpDistribution.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_FLOATUNITEXPDISTRIBUTION_HPP_
#define KERNELS_SOFTWARE_FLOATUNITEXPDISTRIBUTION_HPP_

#include "FloatVectorBase.hpp"

#include "avx_mathfun.h"

/** Floating-point unit exponential distribution (mean = 1/lambda = 1.0, var = 1/lambda^2 = 1.0)
 *
 */

class FloatUnitExpDistribution : public FloatVectorBase
{
public:
	typedef float					result_type;
	static constexpr std::size_t	OutputsPerInputBlock=8;		///< Outputs generated per invocation
	static constexpr std::size_t	OutputElementSize=1;		///< Number of output elements per input

	typedef uint32_t				input_type;
	static constexpr std::size_t	InputBlockSize=8;			///< Inputs consumed per invocation

	template<class RNG>void calculate(RNG& rng,float *dst)
	{
		const uint32_t* rp = rng.getBlock();
		__m256i r = _mm256_load_si256((const __m256i*)rp);

		__m256 u10 = _mm256_sub_ps(									// r = 2.0 - [1,2) -> [1.0,0) avoiding zero for log(x)
				broadcast(2.0f),
				_mm256_or_ps(
						_mm256_castsi256_ps(_mm256_srli_epi32(r,9)),	// uniform random mantissa
						ldexp(0)));										// exponent 2^0 -> [1,2) due to implicit leading 1

		__m256 exprnd = _mm256_sub_ps(
							_mm256_setzero_ps(),
							log_ps(u10));							// -log(X) is unit exponential

		_mm256_store_ps(dst, exprnd);
	}
};




#endif /* KERNELS_SOFTWARE_FLOATUNITEXPDISTRIBUTION_HPP_ */
