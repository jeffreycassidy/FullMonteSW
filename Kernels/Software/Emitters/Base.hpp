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

/** Base class for all emitters. All it needs to do is emit a LaunchPacket. */

template<class RNG>class EmitterBase
{
public:
	EmitterBase(){}
	virtual ~EmitterBase(){}

	/// Emit a packet (direction, position, element ID) - constness ensures it is thread-safe to share (NOTE: RNG must be thread-safe)
	virtual LaunchPacket emit(RNG&) const=0;

private:
};


/** Convenience class that permits composing a position distribution with a direction distribution. By design, the source emits
 * only within a single geometry-model element.
 */

template<class RNG,class Position,class Direction>class PositionDirectionEmitter : public EmitterBase<RNG>
{
public:

	PositionDirectionEmitter(const Position& P,const Direction& D,unsigned element=-1U) :
		m_pos(P),
		m_dir(D),
		m_element(element){}

	virtual LaunchPacket emit(RNG& rng) const override
	{
		LaunchPacket lpkt;
		lpkt.pos = m_pos.position(rng);
		lpkt.dir = m_dir.direction(rng);
		lpkt.element = m_element;

		return lpkt;
	}

	Position& 	positionDef()	{ return m_pos; }
	Direction& 	directionDef()	{ return m_dir; }

	unsigned	element()			const	{ return m_element; }
	void		element(unsigned e)			{ m_element=e; }


private:
	Position		m_pos;
	Direction		m_dir;
	unsigned		m_element=-1U;
};


};

#endif /* KERNELS_SOFTWARE_EMITTERS_BASE_HPP_ */
