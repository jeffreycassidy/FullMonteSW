/*
 * Tetra.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_TETRA_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_TETRA_HPP_

#include <array>
#include "ImplicitPlane.hpp"

/** Geometric description of a tetrahedron (maximum simplicity, no hardware acceleration) */

namespace Test {

struct Tetra
{
	Tetra(){}

	typedef std::array<float,3> Point3;
	typedef std::array<float,3> Vector3;

	Tetra(Point3 A,Point3 B,Point3 C,Point3 D) :
		m_A(A),m_B(B),m_C(C),m_D(D)
	{
		Vector3 AB(B-A);
		Vector3 AC{ C[0]-A[0], C[1]-A[1], C[2]-A[2] };
		Vector3 AD{ D[0]-A[0], D[1]-A[1], D[2]-A[2] };
		Vector3 BC{ C[0]-B[0], C[1]-B[1], C[2]-B[2] };
		Vector3 BD{ D[0]-B[0], D[1]-B[1], D[2]-B[2] };

		m_fABC = ImplicitPlane<float,3>(cross(AB,AC),A);
		if (m_fABC(D) < 0)
			m_fABC.flip();

		m_fABD = ImplicitPlane<float,3>(cross(AB,AD),A);
		if (m_fABD(C) < 0)
			m_fABD.flip();

		m_fACD = ImplicitPlane<float,3>(cross(AC,AD),A);
		if (m_fACD(B) < 0)
			m_fACD.flip();

		m_fBCD = ImplicitPlane<float,3>(cross(BC,BD),B);
		if (m_fBCD(A) < 0)
			m_fBCD.flip();
	}

	// returns the minimum height over any of the four faces (>0 indicates inside the tetra)
	float operator()(const Point3 p)
	{
		std::array<float,4> heights{
			m_fABC(p),
			m_fABD(p),
			m_fACD(p),
			m_fBCD(p)
		};

		return *boost::min_element(heights);
	}

	Point3 m_A,m_B,m_C,m_D;
	ImplicitPlane<float,3> m_fABC, m_fABD, m_fACD, m_fBCD;
};

};

#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_TETRA_HPP_ */
