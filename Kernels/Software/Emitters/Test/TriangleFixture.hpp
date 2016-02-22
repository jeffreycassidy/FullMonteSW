/*
 * TriangleFixture.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_TRIANGLEFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_TRIANGLEFIXTURE_HPP_

#include "Triangle.hpp"

/** Test fixture to ensure the emitted point is not more than epsilon off the plane or outside the triangle bounds */

struct TriangleFixture
{
	Triangle<float> T;

	typedef std::array<float,3> Point3;

	TriangleFixture(){}

	TriangleFixture(Point3 A,Point3 B,Point3 C) : T(A,B,C)
	{
	}

	void testPosition(std::array<float,3> p)
	{
		BOOST_CHECK_MESSAGE(T.m_plane(p) < m_maxDistanceFromPlane,"TriangleFixture: out of plane, h=" << T.m_plane(p));
		BOOST_CHECK_MESSAGE(T.m_nAB(p) > m_minEdgeAltitude,"TriangleFixture: above line AB, altitude " << T.m_nAB(p));
		BOOST_CHECK_MESSAGE(T.m_nAC(p) > m_minEdgeAltitude,"TriangleFixture: above line AC, altitude " << T.m_nAC(p));
		BOOST_CHECK_MESSAGE(T.m_nBC(p) > m_minEdgeAltitude,"TriangleFixture: above line BC, altitude " << T.m_nBC(p));
	}

	float m_maxDistanceFromPlane=1e-5;
	float m_minEdgeAltitude=-1e-6;
};



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_TRIANGLEFIXTURE_HPP_ */
