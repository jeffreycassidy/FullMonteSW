/*
 * Cone.hpp
 *
 *  Created on: Aug 18, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_CONE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_CONE_HPP_

#include <mmintrin.h>
#include <array>

namespace Emitter
{

/** Conical emitter pattern (eg. cut-end fiber) */

template<class RNG>class Cone
{
public:
	/// Construct from half-angle theta
	Cone(float theta) : m_oneMinusCosThetaMax(1.0f-std::cos(theta)){}

	PacketDirection direction(RNG& rng) const
	{
		// uniform distribution for phi -> cos(phi), sin(phi)
		// uniform distribution for cos(theta)

		__m128 = rng.
		float u01theta = rng.drawU01float();

		float costheta = (1.0f - m_oneMinusCosThetaMax) * u01theta;
		float sintheta = std::sqrt(1.0f - costheta*costheta);

		return PacketDirection(
				SSE::UnitVector3(SSE::Vector3(d),SSE::NoCheck),
				SSE::UnitVector3(SSE::Vector3(a),SSE::NoCheck),
				SSE::UnitVector3(SSE::Vector3(b),SSE::NoCheck));
	}

private:
	/// 1-cos(theta_max)
	float					m_oneMinusCosThetaMax=0.0f;
	std::array<float,3>		m_direction{0.0f,0.0f,1.0f};
};

};



#endif /* KERNELS_SOFTWARE_EMITTERS_CONE_HPP_ */
