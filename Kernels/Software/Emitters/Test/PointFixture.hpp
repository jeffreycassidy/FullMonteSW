/*
 * PointFixture.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_POINTFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_POINTFIXTURE_HPP_

#include <array>
#include <boost/test/floating_point_comparison.hpp>

/** Checks that the packet is emitted close to a specified point  */

struct PointFixture
{
	PointFixture(){}
	PointFixture(std::array<float,3> p) : m_p(p){}
	void testPosition(std::array<float,3> p)
	{
		std::array<float,3> delta{ m_p[0]-p[0], m_p[1]-p[1], m_p[2]-p[2] };
		float d = std::sqrt(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);
		BOOST_CHECK_SMALL(d, m_maxDistance);
	}

	std::array<float,3> m_p;

	float m_maxDistance=1e-5;
};


#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_POINTFIXTURE_HPP_ */
