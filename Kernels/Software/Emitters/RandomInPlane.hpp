/*
 * Disk.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_RANDOMINPLANE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_RANDOMINPLANE_HPP_

#include <tuple>

namespace Emitter {

/** Provides a direction randomly oriented within a planar disk described by the plane's normal */

template<class RNG>class RandomInPlane
{
public:
	RandomInPlane(SSE::Vector3 n) :
		m_normal(n,SSE::Normalize)
	{
		std::tie(m_inPlane[0],m_inPlane[1]) = SSE::normalsTo(m_normal);
	}

	PacketDirection direction(RNG& rng) const
	{
		SSE::Vector2 azuv(rng.uvect2D());

		SSE::Scalar sinpsi = azuv.component<0>();
		SSE::Scalar cospsi = azuv.component<1>();

		return PacketDirection(
				SSE::UnitVector3(m_inPlane[0]*sinpsi + m_inPlane[1]*cospsi,SSE::NoCheck),
				SSE::UnitVector3(m_inPlane[0]*cospsi - m_inPlane[1]*sinpsi,SSE::NoCheck),
				m_normal);
	}

private:
	SSE::UnitVector3				m_normal;		///< Normal to plane
	std::array<SSE::UnitVector3,2>	m_inPlane;		///< In-plane components used to construct vector

};

};

#endif /* KERNELS_SOFTWARE_EMITTERS_RANDOMINPLANE_HPP_ */
