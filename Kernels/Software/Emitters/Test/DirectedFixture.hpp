/*
 * DirectedFixture.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_DIRECTEDFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_DIRECTEDFIXTURE_HPP_


/** Checks that the direction matches a specified vector, ||d-d_expect|| < eps */

struct DirectedFixture
{
	typedef std::array<float,3> Vector3;

	DirectedFixture(){}

	DirectedFixture(Vector3 dir) : m_dir(dir){}

	void testDirection(Vector3 dir)
	{
		Vector3 delta{dir[0]-m_dir[0], dir[1]-m_dir[1], dir[2]-m_dir[2] };

		float deltaNorm =std::sqrt(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);

		BOOST_CHECK_SMALL(deltaNorm,m_maxDirectionDifference);
	}


	Vector3 m_dir;
	float m_maxDirectionDifference=1e-6;
};


#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_DIRECTEDFIXTURE_HPP_ */
