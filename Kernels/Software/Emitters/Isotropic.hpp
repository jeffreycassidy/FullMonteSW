/*
 * IsotropicEmitter.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_ISOTROPIC_HPP_
#define KERNELS_SOFTWARE_ISOTROPIC_HPP_

#include <mmintrin.h>

namespace Emitter
{

/** Isotropic emitter - 3D unit vector */

template<class RNG>class Isotropic
{
public:

	PacketDirection direction(RNG& rng) const
	{
		__m128 one = _mm_set_ss(1.0f);				// [ 1 0 0 0]
		__m128 zero = _mm_setzero_ps();				// [ 0 0 0 0]

		__m128 azuv = _mm_loadu_ps(rng.uvect2D());
		__m128 b = _mm_shuffle_ps(_mm_addsub_ps(zero,azuv),zero,_MM_SHUFFLE(0,1,0,1));

		azuv = _mm_movelh_ps(azuv,azuv);			// [cos(theta), sin(theta), cos(theta), sin(theta)]

		__m128 sinphi = _mm_load1_ps(rng.floatPM1());								// sin(phi) = [-1,1)
		__m128 cosphi = _mm_sqrt_ss(_mm_sub_ss(one,_mm_mul_ss(sinphi,sinphi)));		// cos(phi) = 1-sin2(phi)

		__m128 cpcpspsp = _mm_shuffle_ps(cosphi,sinphi,_MM_SHUFFLE(0,0,0,0));		// [cos(phi) cos(phi) sin(phi) sin(phi)]

		azuv = _mm_mul_ps(azuv,cpcpspsp);											// [ct*cp st*cp ct*sp st*sp]

		__m128 d = _mm_shuffle_ps(azuv,sinphi,_MM_SHUFFLE(1,0,1,0));
		__m128 a = _mm_shuffle_ps(azuv,_mm_sub_ps(zero,cosphi),_MM_SHUFFLE(1,0,3,2));

		return PacketDirection(
				SSE::UnitVector3(SSE::Vector3(d),SSE::NoCheck),
				SSE::UnitVector3(SSE::Vector3(a),SSE::NoCheck),
				SSE::UnitVector3(SSE::Vector3(b),SSE::NoCheck));
	}
};

};


#endif /* KERNELS_SOFTWARE_ISOTROPIC_HPP_ */
