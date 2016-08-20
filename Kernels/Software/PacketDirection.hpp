/*
 * PacketDirection.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_PACKETDIRECTION_HPP_
#define KERNELS_SOFTWARE_PACKETDIRECTION_HPP_

#include "SSEMath.hpp"

struct PacketDirection
{
	PacketDirection(){}
	PacketDirection(const PacketDirection&) = default;
	PacketDirection(SSE::UnitVector3 d);
	PacketDirection(SSE::UnitVector3 di,SSE::UnitVector3 ai,SSE::UnitVector3 bi) : d(di),a(ai),b(bi){}

	SSE::UnitVector3		d;
	SSE::UnitVector3		a;
	SSE::UnitVector3		b;

    /** Check if packet directions are orthonormal within some tolerance.
     * For all pairs \f$(\vect u,\vect v)\;\vect u \ne \vect v\f$, checks that vectors are orthogonal to each other and have unit
     * length, both to within a given tolerance \f$\epsilon\f$
     * \f{eqnarray}{ |\vect u \cdot \vect v| < \epsilon \\
     * |\vect u \cdot \vect u -1| < \epsilon \f}
     *
     * @param eps (optional) Tolerance for comparisons (default 1e-5)
     * @return True if packet is within tolerance
     */

	PacketDirection scatter(SSE::Vector<4> deflAz) const;
	PacketDirection scatter(SSE::Vector2 deflection,SSE::Vector2 azimuth) const;

    bool checkOrthonormal(float eps=1e-5) const;
    bool checkOrthonormalVerbose(float eps=1e-5) const;
};



inline PacketDirection PacketDirection::scatter(SSE::Vector2 theta,SSE::Vector2 phi) const
{

	return scatter(SSE::Vector<4>(
			_mm_movelh_ps(__m128(theta),__m128(phi)))		// _mm_movelh_ps(a,b) moves lower 2 elements from b to higher 2 of a
			);
}


/** Scatter the direction by the given deflection and azimuth angles
 * [0] cos(theta)	[1] sin(theta)	[2] cos(phi)	[3] sin(phi)
 */

// TODO: Hoist SSE-specific code out of here

inline PacketDirection PacketDirection::scatter(SSE::Vector<4> deflAz) const
{
    // Trying to conform to MSDN docs
    //                          3 2 1 0
    //  _mm_set_ps(a,b,c,d) =   a b c d

    __m128 M0,M1,M2;
    const __m128 d0=__m128(d), a0=__m128(a), b0=__m128(b);

    // rows of matrix M
//    M0 = _mm_setr_ps(costheta,sintheta,0,0);
//    M1 = _mm_setr_ps(-sintheta*cosphi,costheta*cosphi,sinphi,0);	// 0 sinphi (costheta * cosphi)  (-sintheta * cosphi)
//    M2 = _mm_setr_ps(sinphi*sintheta,-sinphi*costheta,cosphi,0);	// 0 cosphi (-sinphi * costheta) (sinphi * sintheta)

	__m128 zero = _mm_setzero_ps();
	__m128 strig = _mm_addsub_ps(zero,__m128(deflAz));	// (-sin phi) (cos phi) (-sin theta) (cos theta)

	__m128 prods = _mm_mul_ps(
        strig,                                              // -sinp cosp -sint cost
        _mm_shuffle_ps(strig,strig,_MM_SHUFFLE(1,0,2,3)));  // -sint cost cosp  -sinp
    // prods = (sintheta*sinphi) (costheta*cosphi) (-sintheta*cosphi) (-costheta*sinphi)

	__m128 _0_sp_0_cp = _mm_unpackhi_ps(__m128(deflAz),zero);  // 0 sinp 0 cosp

    // The following 3 defs are verified to match M0..M2 in comments above
	M0 = _mm_movelh_ps(__m128(deflAz),zero);                                  // 0 0 sint cost
	M1 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,2,2,1));     // 0 sinp cost*cosp -sint*cosp
	M2 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,0,0,3));     // 0 cosp -cost*sinp sint*sinp


    // d = cos(theta)*d0 - sin(theta)*cos(phi)*a0 + sin(theta)*sin(phi)*b0
    __m128 newd = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(0,0,0,0)));
    newd 		= _mm_add_ps(newd,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(0,0,0,0))));
    newd 		= _mm_add_ps(newd,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(0,0,0,0))));

    // a = sin(theta)*d0 +cos(theta)*cos(phi)*a0 - cos(theta)*sin(phi)*b0
    __m128 newa = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(1,1,1,1)));
    newa 		= _mm_add_ps(newa,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(1,1,1,1))));
    newa 		= _mm_add_ps(newa,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(1,1,1,1))));

    // b = sin(phi)*a0 + cos(phi)*b0
    __m128 newb = _mm_add_ps(
    					_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(2,2,2,2))),
    					_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(2,2,2,2))));

    return PacketDirection(
    		SSE::UnitVector3(SSE::Vector3(newd),SSE::NoCheck),
			SSE::UnitVector3(SSE::Vector3(newa),SSE::NoCheck),
			SSE::UnitVector3(SSE::Vector3(newb),SSE::NoCheck));

}



#endif /* KERNELS_SOFTWARE_PACKETDIRECTION_HPP_ */
