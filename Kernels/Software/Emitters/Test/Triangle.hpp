/*
 * Triangle.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_TRIANGLE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_TRIANGLE_HPP_

#include "ImplicitPlane.hpp"

/** Simple geometric description of a triangle, not optimized for space or speed. */

template<class FT>struct Triangle
{
	typedef std::array<FT,3> Point3;
	typedef std::array<FT,3> Vector3;

	Triangle(){}

	Triangle(Point3 A,Point3 B,Point3 C) :
		m_A(A), m_B(B),m_C(C)
	{
		Vector3 AB = normalize(Vector3{ B[0]-A[0], B[1]-A[1], B[2]-A[2] });
		Vector3 AC = normalize(Vector3{ C[0]-A[0], C[1]-A[1], C[2]-A[2] });
		Vector3 BC = normalize(Vector3{ C[0]-B[0], C[1]-B[1], C[2]-B[2] });

		Vector3 n = normalize(cross(AB,AC));

		// the planes defined by each face
		m_nAB = ImplicitPlane<float,3>(cross(n,AB), A);
		if (m_nAB(C) < 0)
			m_nAB.flip();

		m_nAC = ImplicitPlane<float,3>(cross(n,AC), A);
		if (m_nAC(B) < 0)
			m_nAC.flip();

		m_nBC = ImplicitPlane<float,3>(cross(n,BC), B);
		if (m_nBC(A) < 0)
			m_nBC.flip();

		// the plane of the triangle
		m_plane = ImplicitPlane<float,3>(n, A);
	}


	Point3 m_A,m_B,m_C;

	ImplicitPlane<float,3> m_nAB, m_nAC, m_nBC, m_plane;
};



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_TRIANGLE_HPP_ */
