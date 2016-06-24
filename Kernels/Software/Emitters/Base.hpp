/*
 * Base.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_BASE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_BASE_HPP_

#include "../Packet.hpp"
#include <tuple>

namespace Emitter
{

/** Base class for all emitters. All it needs to do is emit a LaunchPacket (dir, pos, element ID) using the virtual emit(RNG&) method.
 *
 * @tparam 	RNG		The random-number generator to be used with this class
 */

template<class RNG>class EmitterBase
{
public:
	EmitterBase(){}
	virtual ~EmitterBase(){}

	/// Emit a packet (direction, position, element ID) - constness ensures it is thread-safe to share
	virtual LaunchPacket emit(RNG&) const=0;

private:
};


/** Convenience class that permits composing a position distribution with a direction distribution. By design, the source emits
 * only within a single geometry-model element maintained by this base class.
 *
 * @tparam		RNG			Random number generator class
 * @tparam		Position	Class with a position(RNG&) function to provide the origin point
 * @tparam		Direction	Class with a direction(RNG&) function provided the PacketDirection
 */

template<class RNG,class Position,class Direction>class PositionDirectionEmitter : public EmitterBase<RNG>
{
public:
	/// Create from a Position&, Direction&, and element
	PositionDirectionEmitter(const Position& P,const Direction& D,unsigned element=-1U) :
		m_pos(P),
		m_dir(D){}

	/// Dispatch the position and direction requests to the component elements (note these can be inlined due to template)
	virtual LaunchPacket emit(RNG& rng) const override
	{
		LaunchPacket lpkt;
		std::tie(lpkt.element,lpkt.pos) = m_pos.position(rng);
		lpkt.dir = m_dir.direction(rng);

		return lpkt;
	}

	/// Access the position distribution
	Position& 	positionDef()	{ return m_pos; }

	/// Access the direction distribution
	Direction& 	directionDef()	{ return m_dir; }

private:
	Position		m_pos;				///< Position distribution
	Direction		m_dir;				///< Direction distribution
};

};

#endif /* KERNELS_SOFTWARE_EMITTERS_BASE_HPP_ */
