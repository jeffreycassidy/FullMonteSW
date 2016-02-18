/*
 * SSEConvert.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_SSECONVERT_HPP_
#define KERNELS_SOFTWARE_SSECONVERT_HPP_

#include <mmintrin.h>
#include <array>

inline std::array<float,4> as_array(__m128 x)
{
	std::array<float,4> o;
	_mm_storeu_ps(o.data(),x);
	return o;
}


#endif /* KERNELS_SOFTWARE_SSECONVERT_HPP_ */
