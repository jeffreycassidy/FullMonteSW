/*
 * OrthogonalFixture.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_ORTHOGONALFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_ORTHOGONALFIXTURE_HPP_

#include <boost/test/floating_point_comparison.hpp>

/** Checks if two vectors are orthogonal with an absolute tolerance dot(a,b) < eps
 *
 * NOTE: For large vectors this may run into problems and a relative comparison may be more appropriate.
 */

struct OrthogonalFixture
{
	template<typename FT,std::size_t D>static FT dot(std::array<FT,D> lhs,std::array<FT,D> rhs)
	{
		FT dot=0.0f;

		for(unsigned i=0;i<D;++i)
			dot += lhs[i]*rhs[i];
		return dot;
	}
};

struct AbsoluteOrthogonalFixture : public OrthogonalFixture
{
	template<typename FT,std::size_t D>void testOrthogonal(std::array<FT,D> lhs,std::array<FT,D> rhs)
	{
		float dot = dot(lhs,rhs);
		BOOST_CHECK_SMALL(dot,FT(m_nonOrthogonalEpsilon));
	}

	double m_nonOrthogonalEpsilon=1e-5;
};


#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_ORTHOGONALFIXTURE_HPP_ */
