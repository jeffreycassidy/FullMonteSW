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

PacketPositionTraceSet::PacketPositionTraceSet(const std::list<PacketPositionTrace*>& traces) :
		m_traces(traces)
{

}


PacketPositionTraceSet::PacketPositionTraceSet(std::list<PacketPositionTrace*>&& traces) :
		m_traces(std::move(traces))
{

}

unsigned PacketPositionTraceSet::nPoints() const
{
	unsigned Np=0;
	for(const auto& tr : m_traces)
		if (tr)
			Np += tr->count();
		else
			std::cout << "INFO: Skipping null trace" << std::endl;
	return Np;
}

unsigned PacketPositionTraceSet::nTraces() const
{
	return m_traces.size();
}
