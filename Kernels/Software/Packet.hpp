#ifndef PACKET_INCLUDED
#define PACKET_INCLUDED
#include <immintrin.h>

#include "SSEMath.hpp"
#include "PacketDirection.hpp"


/** Represents a packet at the moment of launch, without a step length (weight implicitly 1.0) */

struct LaunchPacket
{
	LaunchPacket(){}

	PacketDirection		dir;
	SSE::Vector3		pos;
	unsigned			element;
};



class Packet {
public:
	PacketDirection dir;
    __m128 p;     		///< Position [x, y, z, 0]
    __m128 s;           ///< Dimensionless step length remaining (?)
    float w=1.0f;     	///< Packet weight

public:

    Packet(const LaunchPacket& lp) :
    	dir(lp.dir),
		p(lp.pos)
    {}

    float weight() const { return w; }

    SSE::UnitVector3 direction() const { return dir.d; }


    /// Initialize a brand-new packet with no step length to go, and unit weight
//    Packet() : s(_mm_setzero_ps()),w(1.0){}

    /// Initialize a new packet with a given position and direction
    //Packet(const Ray<3,double>& r) : s(_mm_setzero_ps()),w(1.0){ setRay(r); }

    Packet& operator=(const Packet& p_) = default;

//    /// Sets the packet position and direction using a Ray<3,double>
//    void setRay(const Ray<3,double>& r)
//        {
//    		Point<3,double> p0=r.getOrigin();
//    		p = _mm_set_ps(0.0,p0[2],p0[1],p0[0]);
//
//    		UnitVector<3,double> d0=r.getDirection();
//    		setDirection(d0[0],d0[1],d0[2]);
//        }

//    void setPosition(Point<3,double> p_)
//    {
//    	p = _mm_set_ps(0.0,p_[2],p_[1],p_[0]);
//    }

    /** Sets the direction for the packet, selecting auxiliary vectors (a,b) as necessary.
     * Current implementation uses \f{eqnarray}{ \uvect a = \frac{\uvect d \times \uvect k}{|\uvect d \times \uvect k|}
     *  \uvect b =\uvect{d}\times\uvect{a} \f}
     *
     * @param d_ New direction
     */
//    void setDirection(__m128 d_)
//        { d=d_; a=getNormalTo(d_); b=cross(d,a); }
//
//
//    /** Sets the new direction for the packet, with input specifying the direction and auxiliary vectors.
//     * NOTE: Does not perform error checking.
//     * TODO: Could be (a lot?) faster by using _mm_load_ps
//     */
//
//    void setDirection(const std::array<UnitVector<3,double>,3>& d0)
//    {
//    	d=_mm_set_ps(0.0,d0[0][2],d0[0][1],d0[0][0]);
//    	a=_mm_set_ps(0.0,d0[1][2],d0[1][1],d0[1][0]);
//    	b=_mm_set_ps(0.0,d0[2][2],d0[2][1],d0[2][0]);
//    }
//
//    void setDirection(const UnitVector<3,double>& d_){ setDirection(_mm_set_ps(0,d_[2],d_[1],d_[0])); }
//
//    /** Sets the direction for the packet using (x,y,z) float input constants
//     * @param dx,dy,dz	New direction vector (must be unit)
//     */
//    void setDirection(float dx,float dy,float dz)
//    {
//    	d = _mm_set_ps(0,dz,dy,dx);
//    	a = getNormalTo(d);
//    	b = cross(d,a);
//    }
};

#endif
