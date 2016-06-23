/*
 * SFMTWrapper.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_SFMTWRAPPER_HPP_
#define KERNELS_SOFTWARE_SFMTWRAPPER_HPP_

#include <cassert>

#define SFMT_MEXP 19937
#include <FullMonteSW/SFMT/SFMT.h>

namespace URNG
{
	template<typename RNG>void 				seed(RNG& rng,unsigned s);						///< Seed RNG with unsigned value
	template<typename RNG,typename T>void 	generate(RNG& rng,T* dst,std::size_t N);		///< Generate N elements of type T

	template<>inline void seed(sfmt_t& rng,unsigned s)
	{
		sfmt_init_gen_rand(&rng,s);
	}

	template<>inline void generate(sfmt_t& rng,uint32_t* dst,std::size_t N)
	{
		assert(N >= 624);
		assert(N % 4 == 0);
		sfmt_fill_array32(&rng,dst,N);
	}
};



#endif /* KERNELS_SOFTWARE_SFMTWRAPPER_HPP_ */

