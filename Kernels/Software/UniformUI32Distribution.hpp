/*
 * UniformUI32Distribution.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_UNIFORMUI32DISTRIBUTION_HPP_
#define KERNELS_SOFTWARE_UNIFORMUI32DISTRIBUTION_HPP_

/** Returns 8 uint32_t's */

class UniformUI32Distribution
{
public:
	typedef uint32_t				result_type;
	static constexpr std::size_t	OutputsPerInputBlock=8;
	static constexpr std::size_t	OutputElementSize=1;

	typedef uint32_t				input_type;
	static constexpr std::size_t	InputBlockSize=8;

	template<class RNG>void calculate(RNG& rng,uint32_t* dst)
	{
		const __m256i* ip = (const __m256i*)rng.getBlock();
		_mm256_store_si256((__m256i*)dst,_mm256_load_si256(ip));
	}
};




#endif /* KERNELS_SOFTWARE_UNIFORMUI32DISTRIBUTION_HPP_ */
