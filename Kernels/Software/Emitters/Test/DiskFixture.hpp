/*
 * DiskFixture.hpp
 *
 *  Created on: Aug 19, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_DISKFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_DISKFIXTURE_HPP_

#include <array>
#include <boost/test/floating_point_comparison.hpp>

#include <FullMonteSW/Geometry/StandardArrayKernel.hpp>

struct DiskFixture
{
	DiskFixture();
	~DiskFixture();

	DiskFixture(std::array<float,3> centre,std::array<float,3> normal,float radius);

	void testPosition(std::array<float,3> p);

	std::array<float,3>	m_centre;
	std::array<float,3>	m_normal;
	float				m_radiusSquared;
};

DiskFixture::DiskFixture()
{

}

DiskFixture::~DiskFixture()
{

}

DiskFixture::DiskFixture(std::array<float,3> centre,std::array<float,3> normal,float radius) :
		m_centre(centre),
		m_normal(normal),
		m_radiusSquared(radius*radius)
{
}

void DiskFixture::testPosition(std::array<float,3> p)
{
	// check that point lies in plane
	BOOST_CHECK_SMALL(dot(m_normal,p-m_centre),1e-5f);

	// check that point is sufficiently close to centre
	BOOST_CHECK_LE(norm2(p-m_centre), m_radiusSquared);
}


#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_DISKFIXTURE_HPP_ */
