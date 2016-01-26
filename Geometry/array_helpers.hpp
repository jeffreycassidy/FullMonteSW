/*
 * array_helpers.hpp
 *
 *  Created on: Jan 18, 2015
 *      Author: jcassidy
 */

#ifndef ARRAY_HELPERS_HPP_
#define ARRAY_HELPERS_HPP_

#include <array>
#include <iostream>

template<typename T,size_t N>std::istream& operator>>(std::istream& is,std::array<T,N>& a)
{
	for(unsigned i=0;i<N;++i)
		is >> a[i];
	return is;
}

template<typename T,size_t N>std::ostream& operator<<(std::ostream& os,const std::array<T,N>& a)
{
	os << a[0];
	for(size_t i=1;i<N;++i)
		os << ' ' << a[i];
	return os;
}


struct project_xy {
	static constexpr size_t N=2;
	static const std::array<unsigned char,N> idx;
};

struct project_yz {
	static constexpr size_t N=2;
	static const std::array<unsigned char,N> idx;
};

struct project_xz {
	static constexpr size_t N=2;
	static const std::array<unsigned char,N> idx;
};

template<typename OrthoProjection,typename T,size_t Di> std::array<T,OrthoProjection::N> ortho_project(const std::array<T,Di>& a)
{
	std::array<T,OrthoProjection::N> o;
	for(unsigned i=0;i<OrthoProjection::N;++i)
		o[i] = a[OrthoProjection::idx[i]];
	return o;
}

template<typename T,size_t D>T dot(const std::array<T,D>& a,const std::array<T,D>& b)
{
	T sum=a[0]*b[0];
	for(unsigned i=1;i<D;++i)
		sum += a[i]*b[i];
	return sum;
}


template<typename T,size_t N>T norm2_l2(const std::array<T,N>& lhs){ return dot(lhs,lhs); }

template<typename T,size_t N>T norm_l2(const std::array<T,N>& v){ return sqrt(norm2_l2(v)); }



#define BINARY_ARRAY_OP(op) 																						\
	template<typename T,size_t N>std::array<T,N> operator op(const std::array<T,N> lhs,const std::array<T,N> rhs)	\
	{																												\
		std::array<T,N> y;																							\
		for(unsigned i=0;i<N;++i)																					\
			y[i] = lhs[i] op rhs[i];																				\
		return y;																									\
	}

#define SCALAR_ARRAY_OP(op)																							\
	template<typename T,size_t N>std::array<T,N> operator op(T k,const std::array<T,N> rhs)							\
	{																												\
		std::array<T,N> y;																							\
		for(unsigned i=0;i<N;++i)																					\
			y[i] = k op rhs[i];																						\
		return y;																									\
	}																												\
	template<typename T,size_t N>std::array<T,N> operator op(const std::array<T,N> lhs,T k)							\
	{																												\
		std::array<T,N> y;																							\
		for(unsigned i=0;i<N;++i)																					\
			y[i] = lhs[i] op k;		 																				\
		return y;																									\
	}																												\

BINARY_ARRAY_OP(+)
BINARY_ARRAY_OP(-)
BINARY_ARRAY_OP(*)
BINARY_ARRAY_OP(/)

SCALAR_ARRAY_OP(+)
SCALAR_ARRAY_OP(-)
SCALAR_ARRAY_OP(*)
SCALAR_ARRAY_OP(/)

#endif /* ARRAY_HELPERS_HPP_ */
