/*
 * OrientedPlane.hpp
 *
 *  Created on: Mar 7, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_BASIS_HPP_
#define GEOMETRY_BASIS_HPP_

#include <array>
#include <cmath>

#include <boost/range/algorithm.hpp>
#include <functional>

#include "StandardArrayKernel.hpp"

typedef std::array<float,3> Point3;
typedef std::array<float,2> Point2;

typedef std::array<float,4> Vector4;
typedef std::array<float,3> Vector3;
typedef std::array<float,2> Vector2;

typedef std::array<float,3> UnitVector3;

typedef std::array<std::array<float,3>,3> Matrix3;			// 3x3 transformation matrix
typedef std::array<std::array<float,4>,3> AffineMatrix3;	// 3x4, but last vector entry is implicitly 1 when multiplying

template<typename FT,std::size_t R,std::size_t C>void clear(std::array<std::array<FT,C>,R>& M)
{
	for(unsigned i=0;i<R;++i)
		for(unsigned j=0;j<C;++j)
			M[i][j]=FT(0);
}

template<typename FT,std::size_t R,std::size_t C>std::array<FT,C> row(unsigned i,const std::array<std::array<FT,C>,R> M)
{
	return M[i];
}

template<typename T,std::size_t D>std::array<T,D+1> appendCoord(const std::array<T,D> v,T x)
{
	std::array<T,D+1> o;
	for(unsigned i=0;i<D;++i)
		o[i] = v[i];
	o[D] = x;
	return o;
}

template<typename FT,std::size_t R,std::size_t C>std::array<FT,R> column(unsigned j,const std::array<std::array<FT,C>,R> M)
{
	std::array<FT,R> c;
	for(unsigned i=0;i<R;++i)
		c[i] = M[i][j];
	return c;
}

inline Vector3 operator*(const Matrix3 A,const Vector3 x)
{
	const unsigned R=3,C=3;
	Vector3 y;
	for(unsigned i=0;i<R;++i)
	{
		y[i] = A[i][0]*x[0];
		for(unsigned j=1;j<C;++j)
			y[i] += A[i][j]*x[j];
	}
	return y;
}


inline Vector3 operator*(AffineMatrix3 A,Vector3 x)
{
	const unsigned R=3,C=3;
	Vector3 y;
	for(unsigned i=0;i<R;++i)
	{
		y[i] = A[i][C];
		for(unsigned j=0;j<C;++j)
			y[i] += A[i][j]*x[j];
	}
	return y;
}



#define ARRAY_COMPARE(name,op)template<typename T,std::size_t D>																\
	unsigned elementwise_##name(const std::array<T,D> lhs,const std::array<T,D> rhs){											\
		unsigned mask=0;																										\
		for(unsigned i=0;i<D;++i)																								\
			mask |= (lhs[i] op rhs[i]) << i;																					\
		return mask;																											\
	}

/** Returns a mask in which bit i is set if the comparison is true for comp(lhs[i],rhs[i])
 *
 */

ARRAY_COMPARE(ge,>=)
ARRAY_COMPARE(gt,>)
ARRAY_COMPARE(le,<=)
ARRAY_COMPARE(lt,<)
ARRAY_COMPARE(eq,==)
ARRAY_COMPARE(ne,!=)


template<typename FT,std::size_t D>std::array<FT,D> elementwise_abs(std::array<FT,D> a)
{
	std::array<FT,D> o;
	for(unsigned i=0;i<D;++i)
		o[i] = std::abs(a[i]);
	return o;
}

template<typename T,std::size_t D>std::array<T,D> elementwise_min(std::array<T,D> lhs,std::array<T,D> rhs)
{
	std::array<T,D> o;
	for(unsigned i=0;i<D;++i)
		o[i] = std::min(lhs[i],rhs[i]);
	return o;
}

template<typename T,std::size_t D>std::array<T,D> elementwise_max(std::array<T,D> lhs,std::array<T,D> rhs)
{
	std::array<T,D> o;
	for(unsigned i=0;i<D;++i)
		o[i] = std::max(lhs[i],rhs[i]);
	return o;
}





class Basis
{
public:
	/// Returns the standard Cartesian basis (i'th component of i'th basis vector is unity, all else zero)
	static Basis standard();

	Basis();
	Basis(Vector3,Vector3,Vector3,Point3);

	/// Project finds the basis coordinates of the specified Cartesian point
	Point3 project(Point3) const;

	/// Invert takes a coordinate in terms of the basis and returns it to the standard Cartesian basis
	Point3 invert(Point3) const;
	Point3 invert(Point2) const;

	/// Returns the origin of the basis space (where (0,0,0) maps to)
	Point3		origin() const;
	void		origin(Point3);

	UnitVector3 normal() const;
	float		constant() const;

	UnitVector3 vector(unsigned i) const
	{
		return column(i,m_matrix);
	}

	UnitVector3 basis_x() const;
	UnitVector3 basis_y() const;

private:
	AffineMatrix3	m_matrix;
	//AffineMatrix3	m_inverse;
};


#endif /* GEOMETRY_BASIS_HPP_ */
