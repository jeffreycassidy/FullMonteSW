/*
 * PacketPositionTrace.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#include "PacketPositionTrace.hpp"
#include <algorithm>
#include <FullMonteSW/Geometry/StandardArrayKernel.hpp>

PacketPositionTrace::PacketPositionTrace()
{
}

PacketPositionTrace::PacketPositionTrace(const std::vector<Step>& tr) : m_trace(tr)
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
	const auto lb = std::lower_bound(m_trace.begin(), m_trace.end(), t, compareTime);

	if (lb == m_trace.end())
		return std::array<float,3>{NAN,NAN,NAN};
	else
	{
		float w =  (t - lb->t) / ((lb+1)->t - lb->t);
		return w*(lb+1)->pos + (1-w)*lb->pos;
	}
}

std::array<float,3> PacketPositionTrace::positionAfterLength(float l) const
{
	//std::lower_bound(m_trace.begin(), m_trace.end(), l, compareLength);
	return std::array<float,3>{0.0f,0.0f,0.0f};
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
