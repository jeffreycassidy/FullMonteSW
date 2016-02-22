/*
 * UnitVectorFixture.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_UNITVECTORFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_UNITVECTORFIXTURE_HPP_

#include <array>
#include <boost/test/floating_point_comparison.hpp>

/** Test fixture to check that a vector is indeed unit (within epsilon) */

struct UnitVectorFixture
{
	template<typename FT,std::size_t D>void testUnitVector(std::array<FT,D> uv)
	{
		FT rr=0.0f;
		for(unsigned i=0;i<D;++i)
			rr += uv[i]*uv[i];

		FT r = std::sqrt(rr);

		BOOST_CHECK_SMALL(r-FT(1.0),FT(m_nonUnitEpsilon));
	}

	double m_nonUnitEpsilon = 1e-5;
};




#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_UNITVECTORFIXTURE_HPP_ */
