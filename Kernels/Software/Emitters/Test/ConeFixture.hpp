/*
 * ConeFixture.hpp
 *
 *  Created on: Aug 19, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_CONEFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_CONEFIXTURE_HPP_

#include <array>
#include <cmath>

#include <boost/test/floating_point_comparison.hpp>

#include <boost/math/constants/constants.hpp>

#include <FullMonteSW/Geometry/StandardArrayKernel.hpp>

/** Checks that a source has an isotropic emission pattern
 * TODO: Need to check the accumulated statistics when it's done! */

struct ConeFixture
{
	ConeFixture();
	ConeFixture(std::array<float,3> direction,float halfAngleRadians);
	~ConeFixture();

	void testDirection(std::array<float,3> dir);

	std::array<float,3> m_direction;
	float m_cosHalfAngle;
};

ConeFixture::ConeFixture()
{

}

ConeFixture::ConeFixture(std::array<float,3> direction,float halfAngleRadians)
{
	m_direction = direction;
	m_cosHalfAngle = std::cos(halfAngleRadians);
}

ConeFixture::~ConeFixture()
{
}

void ConeFixture::testDirection(std::array<float,3> dir)
{
	BOOST_CHECK(dot(dir,m_direction) >= m_cosHalfAngle);
}


#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_CONEFIXTURE_HPP_ */
