/*
 * PacketPositionTraceSet.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#include "PacketPositionTraceSet.hpp"

PacketPositionTraceSet::PacketPositionTraceSet()
{
}

unsigned PacketPositionTraceSet::nPoints() const
{
	unsigned Np=0;
	for(const auto& tr : m_traces)
		Np += tr.count();
	return Np;
}

unsigned PacketPositionTraceSet::nTraces() const
{
	return m_traces.size();
}
