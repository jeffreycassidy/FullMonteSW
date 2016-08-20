/*
 * DiskFixture.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_PLANEFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_PLANEFIXTURE_HPP_

#include <boost/test/floating_point_comparison.hpp>

/** Checks that the direction lies within a 2D disk by |dot(n_disk, d)| < eps */

struct PlaneFixture
{
	PlaneFixture(){}
	PlaneFixture(std::array<float,3> n) : normal(n)
	{
		float nn=0.0f;
		for(unsigned i=0;i<3;++i)
			nn += normal[i];
		float k=1.0/std::sqrt(nn);
		for(unsigned i=0;i<3;++i)
			normal[i] *= k;
	}

	void testDirection(std::array<float,3> dir)
	{
		float dot = dir[0]*normal[0] + dir[1]*normal[1] + dir[2]*normal[2];
		BOOST_CHECK_SMALL(dot,m_maxOutOfPlane);
	}

	std::array<float,3> normal;
	float m_maxOutOfPlane=1e-5;
};



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_PLANEFIXTURE_HPP_ */
