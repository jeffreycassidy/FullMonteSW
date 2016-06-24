/*
 * DirectionFixture.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_PACKETDIRECTIONFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_PACKETDIRECTIONFIXTURE_HPP_

#include <FullMonteSW/Kernels/Software/Packet.hpp>

#include <boost/test/floating_point_comparison.hpp>

#include "UnitVectorFixture.hpp"
#include "OrthogonalFixture.hpp"

/** Checks that the direction is correct (d,a,b all unit, and all mutually orthogonal) */

struct PacketDirectionFixture : public UnitVectorFixture, public AbsoluteOrthogonalFixture
{
	void testPacketDirection(PacketDirection dir)
	{
		std::array<float,3> d = dir.d.array();
		std::array<float,3> a = dir.a.array();
		std::array<float,3> b = dir.b.array();

		BOOST_CHECK_SMALL(OrthogonalFixture::dot(d,a), m_nonOrthogonalEps);
		BOOST_CHECK_SMALL(OrthogonalFixture::dot(d,b), m_nonOrthogonalEps);
		BOOST_CHECK_SMALL(OrthogonalFixture::dot(a,b), m_nonOrthogonalEps);

		BOOST_CHECK_SMALL(OrthogonalFixture::dot(d,d)-1.0f, m_nonUnitVectorEps);
		BOOST_CHECK_SMALL(OrthogonalFixture::dot(a,a)-1.0f, m_nonUnitVectorEps);
		BOOST_CHECK_SMALL(OrthogonalFixture::dot(b,b)-1.0f, m_nonUnitVectorEps);
	}

	float m_nonOrthogonalEps=1e-5f;
	float m_nonUnitVectorEps=1e-5f;
};



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_PACKETDIRECTIONFIXTURE_HPP_ */
