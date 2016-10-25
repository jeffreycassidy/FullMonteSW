/*
 * PacketPositionTraceSet.hpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_PACKETPOSITIONTRACESET_HPP_
#define OUTPUTTYPES_PACKETPOSITIONTRACESET_HPP_

#include <list>
#include <boost/range.hpp>

#include "PacketPositionTrace.hpp"

class PacketPositionTraceSet
{
public:
	PacketPositionTraceSet();
	PacketPositionTraceSet(std::list<PacketPositionTrace>&& traces);

	unsigned nPoints() const;
	unsigned nTraces() const;

	boost::iterator_range<std::list<PacketPositionTrace>::const_iterator> traces() const { return m_traces; }

private:
	std::list<PacketPositionTrace>		m_traces;

};




#endif /* OUTPUTTYPES_PACKETPOSITIONTRACESET_HPP_ */
