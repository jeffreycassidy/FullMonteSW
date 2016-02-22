/*
 * FloatU01Distribution.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_FLOATU01DISTRIBUTION_HPP_
#define KERNELS_SOFTWARE_FLOATU01DISTRIBUTION_HPP_

#include "FloatVectorBase.hpp"

/** Returns 8 floats in U [0,1) distribution */

class FloatU01Distribution : public FloatVectorBase
{
public:
	typedef float					result_type;
	static constexpr std::size_t	OutputsPerInputBlock=8;		///< Outputs generated per invocation
	static constexpr std::size_t	OutputElementSize=1;		///< Number of output elements per input

	typedef uint32_t				input_type;
	static constexpr std::size_t	InputBlockSize=8;			///< Inputs consumed per invocation

	template<class RNG>void calculate(RNG& rng,float *dst)
	{
		const uint32_t* ip = rng.getBlock();
		__m256i i = _mm256_load_si256((const __m256i*)ip);
		_mm256_store_ps(dst, ui32ToU01(i));
	}
};



#endif /* KERNELS_SOFTWARE_FLOATU01DISTRIBUTION_HPP_ */
