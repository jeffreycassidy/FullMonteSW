/*
 * TetraFixture.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_TETRAFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_TETRAFIXTURE_HPP_

#include <array>
#include "Tetra.hpp"

/** Fixture to check whether the emitted point lies within . Has a small tolerance that it's allowed to be outside the tetra by */

struct TetraFixture
{
	TetraFixture(){};
	~TetraFixture(){};

	typedef Test::Tetra::Point3 Point3;

	TetraFixture(Point3 A,Point3 B,Point3 C,Point3 D) : T(A,B,C,D){}

	Test::Tetra T;

	void testPosition(Point3 p)
	{
		// check that point is above all four faces, with some tolerance
		BOOST_CHECK(T.m_fABC(p) > m_minFaceHeight);
		BOOST_CHECK(T.m_fABD(p) > m_minFaceHeight);
		BOOST_CHECK(T.m_fACD(p) > m_minFaceHeight);
		BOOST_CHECK(T.m_fBCD(p) > m_minFaceHeight);
	}

	float m_minFaceHeight=-1e-6;
};



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_TETRAFIXTURE_HPP_ */
