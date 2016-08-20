#ifndef PACKET_INCLUDED
#define PACKET_INCLUDED
#include <immintrin.h>

#include "SSEMath.hpp"
#include "PacketDirection.hpp"


/** Represents a packet at the moment of launch, without a step length (not yet drawn) or weight (implicitly 1.0) */

struct LaunchPacket
{
	LaunchPacket(){}

	PacketDirection		dir;
	SSE::Point3			pos;
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

    Packet& operator=(const Packet& p_) = default;
};

#endif
