/*
 * LineFixture.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_LINEFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_LINEFIXTURE_HPP_

#include <array>
#include <boost/test/floating_point_comparison.hpp>

/** Checks that the emission point lies along a line.
 * First computes the nearest point on the line (q) to the emission point (p), then checks ||q-p|| < eps
 */

struct LineFixture
{
	LineFixture(){}
	LineFixture(std::array<float,3> p0,std::array<float,3> p1) :
			m_p0(p0),m_p1(p1),
			m_displacement{ p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2] }
		{
			m_length = std::sqrt(m_displacement[0]*m_displacement[0] + m_displacement[1]*m_displacement[1] + m_displacement[2]*m_displacement[2]);
			float k = 1.0/m_length;
			for(unsigned i=0;i<3;++i)
				m_displacementUnit[i] = m_displacement[i]*k;
		}

	void testPosition(std::array<float,3> p)
	{
		std::array<float,3> v{ p[0]-m_p0[0], p[1]-m_p0[1], p[2]-m_p0[2] };

		// distance to closest point along displacement vector
		float d = v[0]*m_displacementUnit[0] + v[1]*m_displacementUnit[1] + v[2]*m_displacementUnit[2];

		std::array<float,3> q { m_p0[0] + d*m_displacementUnit[0], m_p0[1] + d*m_displacementUnit[1], m_p0[2] + d*m_displacementUnit[2] };
		std::array<float,3> n { p[0]-q[0], p[1]-q[1], p[2]-q[2] };

		float absN = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		BOOST_CHECK_SMALL(absN, m_maxDistanceToLine);

		BOOST_CHECK_LE(d,m_length);
		BOOST_CHECK_GE(d,0.0f);
	}

	std::array<float,3> m_p0,m_p1;
	std::array<float,3> m_displacement;
	std::array<float,3> m_displacementUnit;
	float m_length;

	float m_maxDistanceToLine=1e-5;
};




#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_LINEFIXTURE_HPP_ */
