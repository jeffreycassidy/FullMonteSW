/*
 * StandardArrayKernel.hpp
 *
 *  Created on: May 8, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_STANDARDARRAYKERNEL_HPP_
#define GEOMETRY_STANDARDARRAYKERNEL_HPP_

#include <array>
#include <boost/range/algorithm/fill.hpp>

template<typename FT>std::array<FT,3> cross(const std::array<FT,3> lhs,const std::array<FT,3> rhs);
template<typename FT>FT scalartriple(const std::array<FT,3> a,const std::array<FT,3> b,const std::array<FT,3> c);

template<typename FT,std::size_t D>FT dot(const std::array<FT,D> lhs,const std::array<FT,D> rhs);

template<typename FT,std::size_t D>FT norm2(const std::array<FT,D> v);
template<typename FT,std::size_t D>FT norm(const std::array<FT,D> v);

template<typename FT,std::size_t D>std::array<FT,D> operator-(const std::array<FT,D> v);
template<typename FT,std::size_t D>std::array<FT,D> operator-(const std::array<FT,D> lhs,const std::array<FT,D> rhs);

template<typename FT,std::size_t D>std::array<FT,D> operator+(const std::array<FT,D> lhs,const std::array<FT,D> rhs);

template<typename FT,std::size_t D>std::array<FT,D> operator*(const std::array<FT,D> v,FT k);
template<typename FT,std::size_t D>std::array<FT,D> operator*(FT k,const std::array<FT,D> v);

template<typename FT,std::size_t D>std::array<FT,D> operator/(const std::array<FT,D> v,float k);

template<typename FT,std::size_t D>std::array<FT,D> normalize(const std::array<FT,D> v);

template<typename T,std::size_t D>struct StandardArrayKernel
{
	typedef std::array<T,D>		Vector;
	typedef std::array<T,D>		Point;
	typedef std::array<T,D>		UnitVector;
	typedef T					Scalar;

	static std::array<T,D> zeroVector()
		{
			std::array<T,D> a;
			boost::fill(a, 0);
			return a;
		}

	static Scalar dot(Vector,Vector);

};

typedef StandardArrayKernel<float,3> Kernel3f;


template<typename FT>std::array<FT,3> cross(const std::array<FT,3> lhs,const std::array<FT,3> rhs)
{
	return std::array<FT,3>{
		lhs[1]*rhs[2] - lhs[2]*rhs[1],
		lhs[2]*rhs[0] - lhs[0]*rhs[2],
		lhs[0]*rhs[1] - lhs[1]*rhs[0]
	};
}

template<typename FT,std::size_t D>FT StandardArrayKernel<FT,D>::dot(const std::array<FT,D> lhs,const std::array<FT,D> rhs)
{
	FT o(0);
	for(unsigned i=0;i<D;++i)
		o += lhs[i]*rhs[i];
	return o;
}


template<typename FT,std::size_t D>FT dot(const std::array<FT,D> lhs,const std::array<FT,D> rhs)
{
	FT o(0);
	for(unsigned i=0;i<D;++i)
		o += lhs[i]*rhs[i];
	return o;
}

template<typename FT>FT scalartriple(const std::array<FT,3> a,const std::array<FT,3> b,const std::array<FT,3> c)
{
	return dot(a,cross(b,c));
}

template<typename FT,std::size_t D>FT norm2(const std::array<FT,D> v)
{
	return dot(v,v);
}

template<typename FT,std::size_t D>FT norm(const std::array<FT,D> v)
{
	return std::sqrt(norm2(v));
}

template<typename FT,std::size_t D>std::array<FT,D> operator-(const std::array<FT,D> v)
{
	std::array<FT,D> o;
	for(unsigned i=0;i<D;++i)
		o[i]=-v[i];
	return o;
}

template<typename FT,std::size_t D>std::array<FT,D> operator-(const std::array<FT,D> lhs,const std::array<FT,D> rhs)
{
	std::array<FT,D> o;
	for(unsigned i=0;i<D;++i)
		o[i]=lhs[i]-rhs[i];
	return o;
}

template<typename FT,std::size_t D>std::array<FT,D> operator+(const std::array<FT,D> lhs,const std::array<FT,D> rhs)
{
	std::array<FT,D> o;
	for(unsigned i=0;i<D;++i)
		o[i]=lhs[i]+rhs[i];
	return o;
}

template<typename FT,std::size_t D>std::array<FT,D> operator*(const std::array<FT,D> v,FT k)
{
	std::array<FT,D> o;
	for(unsigned i=0;i<D;++i)
		o[i]=k*v[i];
	return o;
}

template<typename FT,std::size_t D>std::array<FT,D> operator*(FT k,const std::array<FT,D> v)
{
	return v*k;
}

template<typename FT,std::size_t D>std::array<FT,D> operator/(const std::array<FT,D> v,float k)
{
	return v*(FT(1)/k);
}


template<typename FT,std::size_t D>std::array<FT,D> normalize(const std::array<FT,D> v)
{
	return (FT(1)/norm(v))*v;
}

#endif /* GEOMETRY_STANDARDARRAYKERNEL_HPP_ */
