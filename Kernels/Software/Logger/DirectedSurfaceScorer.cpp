/*
 * DirectedSurfaceScorer.cpp
 *
 *  Created on: Oct 20, 2016
 *      Author: jcassidy
 */

#include "DirectedSurfaceScorer.hpp"

#include <FullMonteSW/OutputTypes/DirectedSurfaceElement.hpp>
#include <FullMonteSW/OutputTypes/SpatialMap.hpp>

#include <FullMonteSW/Geometry/Units/Quantity.hpp>
#include <FullMonteSW/Geometry/Units/BaseUnit.hpp>

DirectedSurfaceScorer::DirectedSurfaceScorer()
{
}

DirectedSurfaceScorer::~DirectedSurfaceScorer()
{
}


std::list<OutputData*> DirectedSurfaceScorer::results() const
{
	std::vector<DirectedSurfaceElement<float>> se(m_elements);

	se[0] = DirectedSurfaceElement<float>{ float(m_accumulator[m_elements]), float(m_accumulator[m_elements]) };

	for(unsigned i=1;i<m_elements;++i)
		se[i] = DirectedSurfaceElement<float>( float(m_accumulator[m_elements+i]), float(m_accumulator[m_elements-i]) );

	auto m = new SpatialMap<DirectedSurfaceElement<float>>(se,AbstractSpatialMap::Surface,AbstractSpatialMap::UnknownValueType);
	m->quantity(&Units::energy);
	m->units(&Units::packet);

	return std::list<OutputData*>{ m };
}

void DirectedSurfaceScorer::dim(std::size_t N)
{
	m_accumulator.resize(2*N+1);
	m_elements=N;
}

void DirectedSurfaceScorer::clear()
{
	m_accumulator.clear();
}

DirectedSurfaceScorer::Logger DirectedSurfaceScorer::createLogger()
{
	return Logger(m_accumulator.createThreadHandle());
}
