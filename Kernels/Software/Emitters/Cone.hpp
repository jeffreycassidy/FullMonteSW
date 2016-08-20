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

#include <FullMonteSW/Kernels/Software/SSEMath.hpp>

namespace Emitter
{

/** Conical emitter pattern (eg. cut-end fiber) */

template<class RNG>class Cone
{
public:
	/// Construct from half-angle theta
	Cone(SSE::UnitVector3 direction,float halfAngleRadians);

	PacketDirection direction(RNG& rng) const;

private:
	/// 1-cos(theta_max)
	float							m_oneMinusCosThetaMax=0.0f;
	PacketDirection					m_centralDirection;
};

template<class RNG>PacketDirection Cone<RNG>::direction(RNG& rng) const
{
	// 2D unit vector for azimuth component
	const float* az = rng.uvect2D();

	// 2D unit vector for deflection component cos(theta) = 1-(1-cos(theta_max))*F(x)
	float u01theta = *rng.floatU01();

	std::array<float,2> defl;
	defl[0] = 1.0f - m_oneMinusCosThetaMax * u01theta;
	defl[1] = std::sqrt(1.0f - defl[0]*defl[0]);

	SSE::UnitVector2 uv_defl(_mm_set_ps(0.0f,0.0f,defl[1],defl[0]));
	SSE::UnitVector2 uv_az(_mm_set_ps(0.0f,0.0f,az[1],az[0]));

	PacketDirection d_out = m_centralDirection.scatter(uv_defl,uv_az);

	return d_out;
}

template<class RNG>Cone<RNG>::Cone(SSE::UnitVector3 direction,float halfAngle)
{
	m_centralDirection.d = direction;
	std::tie(m_centralDirection.a, m_centralDirection.b) = normalsTo(direction);

	m_oneMinusCosThetaMax = 1.0f-std::cos(halfAngle);

};

};



#endif /* KERNELS_SOFTWARE_EMITTERS_CONE_HPP_ */
