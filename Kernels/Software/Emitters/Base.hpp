/*
 * Base.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_BASE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_BASE_HPP_

#include "../Packet.hpp"

namespace Emitter
{

/** Base class for all emitters. All it needs to do is emit a LaunchPacket using the virtual emit(RNG&) method.
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
 * only within a single geometry-model element.
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
		m_dir(D),
		m_element(element){}

	/// Dispatch the position and direction requests to the component elements (note these can be inlined due to template)
	virtual LaunchPacket emit(RNG& rng) const override
	{
		LaunchPacket lpkt;
		lpkt.pos = m_pos.position(rng);
		lpkt.dir = m_dir.direction(rng);
		lpkt.element = m_element;

		return lpkt;
	}

	/// Access the position distribution
	Position& 	positionDef()	{ return m_pos; }

	/// Access the direction distribution
	Direction& 	directionDef()	{ return m_dir; }

	/// Get/set the geometry element ID
	unsigned	element()			const	{ return m_element; }
	void		element(unsigned e)			{ m_element=e; }


private:
	Position		m_pos;				///< Position distribution
	Direction		m_dir;				///< Direction distribution
	unsigned		m_element=-1U;		///< Element ID for emission
};

};

#endif /* KERNELS_SOFTWARE_EMITTERS_BASE_HPP_ */
