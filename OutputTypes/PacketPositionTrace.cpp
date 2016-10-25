/*
 * PacketPositionTrace.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#include "PacketPositionTrace.hpp"
#include <algorithm>

PacketPositionTrace::PacketPositionTrace()
{
}

PacketPositionTrace::PacketPositionTrace(std::vector<Step>&& tr) : m_trace(std::move(tr))
{
}

PacketPositionTrace::~PacketPositionTrace()
{
}

float PacketPositionTrace::length() const
{
	return m_trace.back().l;
}

unsigned PacketPositionTrace::count() const
{
	return m_trace.size();
}

float PacketPositionTrace::duration() const
{
	return m_trace.back().t;
}

std::array<float,3> PacketPositionTrace::positionAtTime(float t) const
{
	std::lower_bound(m_trace.begin(), m_trace.end(), t, compareTime);

}

std::array<float,3> PacketPositionTrace::positionAfterLength(float l) const
{
	std::lower_bound(m_trace.begin(), m_trace.end(), l, compareLength);
}

const PacketPositionTrace::Step& PacketPositionTrace::operator[](unsigned i) const
{
	return m_trace[i];
}

bool PacketPositionTrace::compareLength(const Step& s,float l)
{
	return s.l < l;
}

bool PacketPositionTrace::compareTime(const Step& s,float t)
{
	return s.t < t;
}
