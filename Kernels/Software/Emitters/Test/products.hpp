/*
 * cross.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_PRODUCTS_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_PRODUCTS_HPP_

#include <array>
#include <cmath>

/** Supporting array operations. This is geared towards simplicity rather than speed (SSE kernels will be much faster) */

template<typename FT>std::array<FT,3> cross(std::array<FT,3> lhs,std::array<FT,3> rhs)
{
	return std::array<FT,3>{
		lhs[1]*rhs[2] - lhs[2]*rhs[1],
		lhs[2]*rhs[0] - lhs[0]*rhs[2],
		lhs[0]*rhs[1] - lhs[1]*rhs[0]
	};
}

template<typename FT,std::size_t D>FT dot(std::array<FT,D> lhs,std::array<FT,D> rhs)
{
	FT d=0;
	for(unsigned i=0;i<D;++i)
		d += lhs[i]*rhs[i];
	return d;
}

template<typename FT,std::size_t D>std::array<FT,D> operator-(std::array<FT,D> a)
{
	std::array<FT,D> o;
	for(unsigned i=0;i<D;++i)
		o[i] = -a[i];
	return o;
}

template<typename FT,std::size_t D>std::array<FT,D> scale(std::array<FT,D> v,FT k)
{
	std::array<FT,D> y;
	for(unsigned i=0;i<D;++i)
		y[i] = v[i]*k;
	return y;
}

template<typename FT,std::size_t D>std::array<FT,D> normalize(std::array<FT,D> v)
{
	FT k = 1.0/std::sqrt(dot(v,v));
	return scale(v,k);
}



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_PRODUCTS_HPP_ */
