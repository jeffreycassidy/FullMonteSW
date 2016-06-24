/*
 * AffineTransform.hpp
 *
 *  Created on: Apr 3, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_MODIFIERS_AFFINETRANSFORM_HPP_
#define GEOMETRY_MODIFIERS_AFFINETRANSFORM_HPP_

#include <array>
#include <boost/range/algorithm.hpp>
#include <FullMonteSW/Geometry/Basis.hpp>

template<typename FT,std::size_t D>class AffineTransform
{
public:
	static AffineTransform<FT,D> zero();
	static AffineTransform<FT,D> identity();

	/// Scale matrix (with optional scale origin)
	static AffineTransform<FT,D> scale(FT k,std::array<FT,D> origin=zeroVector());

	static AffineTransform<FT,D> translate(std::array<FT,D> delta);

	AffineTransform(const AffineTransform&) = default;
	AffineTransform& operator=(const AffineTransform&) = default;

	std::array<FT,D> operator()(std::array<FT,D> p) const;

private:
	AffineTransform();

	static std::array<FT,D> zeroVector();

	AffineMatrix3				m_matrix;
};

template<typename FT,std::size_t D>AffineTransform<FT,D>::AffineTransform()
{
}

template<typename FT,std::size_t D>std::array<FT,D> AffineTransform<FT,D>::zeroVector()
{
	std::array<FT,D> v;
	boost::fill(v,FT(0));
	return v;
}

template<typename FT,std::size_t D>AffineTransform<FT,D> AffineTransform<FT,D>::zero()
{
	AffineTransform<FT,D> T;
	T.m_matrix.clear();
	boost::fill(T.m_offset,FT(0));
	return T;
}

template<typename FT,std::size_t D>AffineTransform<FT,D> AffineTransform<FT,D>::identity()
{
	return AffineTransform<FT,D>::scale(FT(1));
}

template<typename FT,std::size_t D>AffineTransform<FT,D> AffineTransform<FT,D>::translate(const std::array<FT,D> delta)
{
	AffineTransform<FT,D> T;
	for(unsigned i=0;i<D;++i)
	{
		for(unsigned j=0;j<D;++j)
			T.m_matrix(i,j)=FT(0);
		T.m_matrix(i,i)=FT(1);
	}
	T.m_offset=delta;
	return T;
}

template<typename FT,std::size_t D>AffineTransform<FT,D> AffineTransform<FT,D>::scale(FT k,const std::array<FT,D> origin)
{
	AffineTransform<FT,D> A;
	FT ko = FT(1)-k;
	clear(A.m_matrix);
	for(unsigned i=0;i<D;++i)
	{
		A.m_matrix[i][i]=k;
		A.m_matrix[i][3] = ko*origin[i];
	}
	return A;
}

template<typename FT,std::size_t D>std::array<FT,D> AffineTransform<FT,D>::operator()(const std::array<FT,D> x) const
{
	return m_matrix*x;
}


#endif /* GEOMETRY_MODIFIERS_AFFINETRANSFORM_HPP_ */
