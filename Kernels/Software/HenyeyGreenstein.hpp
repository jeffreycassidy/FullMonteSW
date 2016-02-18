/*
 * HenyeyGreenstein.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_HENYEYGREENSTEIN_HPP_
#define KERNELS_SOFTWARE_HENYEYGREENSTEIN_HPP_

#include <immintrin.h>

#pragma vector aligned

#include "avx_mathfun.h"

#include "FloatVectorBase.hpp"


/** Broadcasts one element of an __m128 to all elements of an __m256 */

template<unsigned I>inline __m256 broadcastElement8f(__m128 x)
{
	__m128 tmp = _mm_shuffle_ps(x,x,_MM_SHUFFLE(I,I,I,I));
	return _mm256_insertf128_ps(_mm256_castps128_ps256(tmp),tmp,1);
}




/** Scalar Henyey-Greenstein function for cos(theta)
 *
 * @param	x		u [0,1) variable for which to calculate inverse
 * @param	g		[-1,1) anisotropy factor
 *
 * @return	cos(theta)
 */

float henyeyGreensteinDeflection(float g,float x);




/** Vector Henyey-Greenstein function, returning 8 4-element float vectors per invocation.
 * Each block of 4 output floats gives cos(theta) sin(theta) cos(phi) sin(phi) where theta is deflection and phi is azimuth.
 */

class HenyeyGreenstein8f : public FloatVectorBase
{
public:
	typedef float					result_type;
	static constexpr std::size_t	OutputsPerInputBlock=8;		///< Outputs generated per invocation
	static constexpr std::size_t	OutputElementSize=4;		///< Number of output elements per input

	typedef uint32_t				input_type;
	static constexpr std::size_t	InputBlockSize=16;			///< Inputs consumed per invocation (8 azimuth, 8 deflection)

	template<class RNG>void calculate(RNG& rng,float* o) const;

	/// Get/set g parameter
	void gParam(float g);
	float gParam() const;

private:

	/// Convenience functions for accessing elements of the parameter vector by name
	inline __m256 g() const					{ return broadcastElement8f<0>(m_params);	}
	inline __m256 one_minus_gg() const		{ return broadcastElement8f<1>(m_params); 	}
	inline __m256 one_plus_gg() const		{ return broadcastElement8f<2>(m_params);	}
	inline __m256 recip_2g() const			{ return broadcastElement8f<3>(m_params);	}

	__m128		m_params;			// 0: g, 1: 1-g^2 , 2: 1+g^2, 3: 1/2g
	bool 		m_isotropic=false;	// true if g=0
};




/** Evaluate the Henyey-Greenstein function for random numbers, storing output in 32 floats at o.
 */

template<class RNG>void HenyeyGreenstein8f::calculate(RNG& rng,float* o) const
{
	/// Calculate uniform-random azimuth angle
	__m256 az 	= _mm256_mul_ps(
			ui32ToU01(*(const __m256i*)rng.getBlock()),
			twopi());

	__m256 sinaz, cosaz;
	std::tie(sinaz,cosaz) = sincos_psp(az);

//		std::cout << "cos( phi ): " << std::fixed << std::setprecision(6) << std::setw(8) << cosaz << std::endl;
//		std::cout << "sin( phi ): " << std::fixed << std::setprecision(6) << std::setw(8) << sinaz << std::endl;

	__m256 pm1defl = ui32ToPM1(*(const __m256i*)rng.getBlock());
	__m256 cosdefl, sindefl;

	if (!m_isotropic)
	{
		// t = 1-g^2 / (1+g*P)			where P is random [-1,1)
		__m256 t = _mm256_mul_ps(
						one_minus_gg(),
						_mm256_rcp_ps(										// use approximate reciprocal (error ~1e-11 from Intel)
								_mm256_add_ps(
										one(),
										_mm256_mul_ps(
												g(),
												pm1defl))));

		cosdefl = _mm256_mul_ps(
						recip_2g(),
						_mm256_sub_ps(
								one_plus_gg(),
								_mm256_mul_ps(t,t)));

		// clip to [-1,1]
		cosdefl = _mm256_max_ps(
					_mm256_min_ps(cosdefl,one()),
					_mm256_sub_ps(zero(),one()));
	}
	else
		cosdefl = pm1defl;


	// compute sin x = sqrt(1-cos2 x)
	sindefl = _mm256_sqrt_ps(
				_mm256_sub_ps(
						one(),
						_mm256_mul_ps(cosdefl,cosdefl)));

//		std::cout << "cos(theta): " << std::fixed << std::setprecision(6) << std::setw(8) << cosdefl << std::endl;
//		std::cout << "sin(theta): " << std::fixed << std::setprecision(6) << std::setw(8) << sindefl << std::endl;

	// interleave sine & cosine values independently
	__m256 sin_lo = _mm256_unpacklo_ps(sindefl,sinaz);
	__m256 sin_hi = _mm256_unpackhi_ps(sindefl,sinaz);

	__m256 cos_lo = _mm256_unpacklo_ps(cosdefl,cosaz);
	__m256 cos_hi = _mm256_unpackhi_ps(cosdefl,cosaz);

	// interleave sines & cosines and store to get cos(theta) sin(theta) cos(phi) sin(phi)
	_mm256_store_ps(o,   _mm256_unpacklo_ps(cos_lo,sin_lo));
	_mm256_store_ps(o+8, _mm256_unpackhi_ps(cos_lo,sin_lo));
	_mm256_store_ps(o+16,_mm256_unpacklo_ps(cos_hi,sin_hi));
	_mm256_store_ps(o+24,_mm256_unpackhi_ps(cos_hi,sin_hi));
}



#endif /* KERNELS_SOFTWARE_HENYEYGREENSTEIN_HPP_ */
