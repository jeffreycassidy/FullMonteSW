#ifndef PACKET_INCLUDED
#define PACKET_INCLUDED
#include <immintrin.h>
#include "Geometry/newgeom.hpp"
#include "sse.hpp"

// TODO: Relocate this somewhere more appropriate
/** Computes the dot product of two 3D float vectors (__m128) using _mm_dp_ps.
 * @param	a,b		Vectors to compute
 * @returns A scalar float holding the result.
 */

inline float dot3(__m128 a,__m128 b)
{
	return _mm_cvtss_f32(_mm_dp_ps(a,b,0x71));
}

/** Represents a photon packet traversing the medium
 *
 * TODO: Add variance estimator in w
 * TODO: Incorporate roulette logic
 * TODO: Fold in age counter from mainloop
 */

class Packet {
    public:
    __m128 d;			///< Direction of propagation
    __m128 a;			///< Azimuthal auxiliary vector 1
    __m128 b;			///< Azimuthal auxiliary vector 2
    __m128 p;     		///< Position [x, y, z, 0]
    __m128 s;           ///< Dimensionless step length remaining (?)
    double w;           ///< Packet weight (double used in accumulation)


    /// Initialize a brand-new packet with no step length to go, and unit weight
    Packet() : s(_mm_setzero_ps()),w(1.0){}

    /// Initialize a new packet with a given position and direction
    Packet(const Ray<3,double>& r) : s(_mm_setzero_ps()),w(1.0){ setRay(r); }

    Packet& operator=(const Packet& p_) = default;

    /// Sets the packet position and direction using a Ray<3,double>
    void setRay(const Ray<3,double>& r)
        {
    		Point<3,double> p0=r.getOrigin();
    		p = _mm_set_ps(0.0,p0[2],p0[1],p0[0]);

    		UnitVector<3,double> d0=r.getDirection();
    		setDirection(d0[0],d0[1],d0[2]);
        }

    void setPosition(Point<3,double> p_)
    {
    	p = _mm_set_ps(0.0,p_[2],p_[1],p_[0]);
    }

    /** Sets the direction for the packet, selecting auxiliary vectors (a,b) as necessary.
     * Current implementation uses \f{eqnarray}{ \uvect a = \frac{\uvect d \times \uvect k}{|\uvect d \times \uvect k|} \\
     *  \uvect b =\uvect{d}\times\uvect{a} \f}
     *
     * @param d_ New direction
     */
    void setDirection(__m128 d_)
        { d=d_; a=getNormalTo(d_); b=cross(d,a); }


    /** Sets the new direction for the packet, with input specifying the direction and auxiliary vectors.
     * NOTE: Does not perform error checking.
     * TODO: Could be (a lot?) faster by using _mm_load_ps
     */

    void setDirection(const std::array<UnitVector<3,double>,3>& d0)
    {
    	d=_mm_set_ps(0.0,d0[0][2],d0[0][1],d0[0][0]);
    	a=_mm_set_ps(0.0,d0[1][2],d0[1][1],d0[1][0]);
    	b=_mm_set_ps(0.0,d0[2][2],d0[2][1],d0[2][0]);
    }

    void setDirection(const UnitVector<3,double>& d_){ setDirection(_mm_set_ps(0,d_[2],d_[1],d_[0])); }

    /** Sets the direction for the packet using (x,y,z) float input constants
     * @param dx,dy,dz	New direction vector (must be unit)
     */
    void setDirection(float dx,float dy,float dz)
    {
    	d = _mm_set_ps(0,dz,dy,dx);
    	a = getNormalTo(d);
    	b = cross(d,a);
    }

    /** Return a representation of the argument in the direction-of-travel basis (d,a,b).
     * Calculated as v = < dot(u,d) dot(u,a) dot(u,b) >
     *
     * @param u Vector to be represented
     * @return v Vector in the direction-of-travel basis <d,a,b>
     */

    __m128 getDots(__m128 u) const {
    	return _mm_or_ps(_mm_dp_ps(d,u,0x71),
    			_mm_or_ps(
    				_mm_dp_ps(a,u,0x72),
    				_mm_dp_ps(b,u,0x74)));
    }


    /// Calculate a spin using the matrix representation
    Packet matspin(Packet pkt,__m128 uv2d) const;

    /** Check if packet directions are orthonormal within some tolerance.
     * For all pairs \f$(\vect u,\vect v)\;\vect u \ne \vect v\f$, checks that vectors are orthogonal to each other and have unit
     * length, both to within a given tolerance \f$\epsilon\f$
     * \f{eqnarray}{ |\vect u \cdot \vect v| < \epsilon \\
     * |\vect u \cdot \vect u -1| < \epsilon \f}
     *
     * @param eps (optional) Tolerance for comparisons (default 1e-5)
     * @return True if packet is within tolerance
     */

    bool checkOrthonormal(float eps=1e-5) const;
    bool checkOrthonormalVerbose(float eps=1e-5) const;
};


/** "New" matrix-spin routine using SSE instructions.
 *
 * @param pkt	Incoming packet
 * @param uv2d	Unit vector representing spin: [cos(theta), sin(theta), cos(phi), sin(phi)]
 *
 * @return New packet
 */

inline Packet Packet::matspin(Packet pkt,__m128 uv2d) const
{
    Packet res=pkt;

    // Trying to conform to MSDN docs
    //                          3 2 1 0
    //  _mm_set_ps(a,b,c,d) =   a b c d

    __m128 M0,M1,M2;
    const __m128 d0=pkt.d, a0=pkt.a, b0=pkt.b;

    // rows of matrix M
//    M0 = _mm_setr_ps(costheta,sintheta,0,0);
//    M1 = _mm_setr_ps(-sintheta*cosphi,costheta*cosphi,sinphi,0);	// 0 sinphi (costheta * cosphi)  (-sintheta * cosphi)
//    M2 = _mm_setr_ps(sinphi*sintheta,-sinphi*costheta,cosphi,0);	// 0 cosphi (-sinphi * costheta) (sinphi * sintheta)

	__m128 zero = _mm_setzero_ps();
	__m128 strig = _mm_addsub_ps(zero,uv2d);	// (-sin phi) (cos phi) (-sin theta) (cos theta)

	__m128 prods = _mm_mul_ps(
        strig,                                              // -sinp cosp -sint cost
        _mm_shuffle_ps(strig,strig,_MM_SHUFFLE(1,0,2,3)));  // -sint cost cosp  -sinp
    // prods = (sintheta*sinphi) (costheta*cosphi) (-sintheta*cosphi) (-costheta*sinphi)

	__m128 _0_sp_0_cp = _mm_unpackhi_ps(uv2d,zero);  // 0 sinp 0 cosp

    // The following 3 defs are verified to match M0..M2 in comments above
	M0 = _mm_movelh_ps(uv2d,zero);                                  // 0 0 sint cost
	M1 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,2,2,1));     // 0 sinp cost*cosp -sint*cosp
	M2 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,0,0,3));     // 0 cosp -cost*sinp sint*sinp

    // d = cos(theta)*d0 - sin(theta)*cos(phi)*a0 + sin(theta)*sin(phi)*b0
    res.d = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(0,0,0,0)));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(0,0,0,0))));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(0,0,0,0))));

    // a = sin(theta)*d0 +cos(theta)*cos(phi)*a0 - cos(theta)*sin(phi)*b0
    res.a = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(1,1,1,1)));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(1,1,1,1))));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(1,1,1,1))));

    // b = sin(phi)*a0 + cos(phi)*b0
    res.b = _mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(2,2,2,2)));
    res.b = _mm_add_ps(res.b,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(2,2,2,2))));

    // TODO: Why did it look like below in the old version??
    // res.b = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(2,2,2,2)));
    // res.b = _mm_add_ps(res.b,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(2,2,2,2))));
    // res.b = _mm_add_ps(res.b,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(2,2,2,2))));

    return res;
}

#endif
