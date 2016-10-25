/*
 * PlacementBase.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#include <stdexcept>

#include <FullMonteSW/Geometry/Sources/Abstract.hpp>
#include <FullMonteSW/Geometry/Sources/Composite.hpp>
#include <FullMonteSW/Geometry/Placement/PlacementBase.hpp>
#include <FullMonteSW/Geometry/Placement/PlacementMediatorBase.hpp>

using namespace std;

PlacementBase::PlacementBase()
{

}

PlacementBase::~PlacementBase()
{
	for(PlacementMediatorBase* pm : m_sources)
		pm->placement(nullptr);
}

void PlacementBase::addSource(PlacementMediatorBase* pmb)
{
	m_sources.push_back(pmb);
}

void PlacementBase::addDetector(PlacementMediatorBase* pmb)
{
	m_detectors.push_back(pmb);
}

void PlacementBase::removeSource(PlacementMediatorBase* pmb)
{
	auto it = std::find(m_sources.begin(), m_sources.end(), pmb);
	if (it != m_sources.end())
		m_sources.erase(it);
	else
		throw std::logic_error("PlacementBase::removeSource() source does not exist in this placement");
}

void PlacementBase::removeDetector(PlacementMediatorBase* pmb)
{
	auto it = std::find(m_detectors.begin(), m_detectors.end(), pmb);
	if (it != m_detectors.end())
		m_detectors.erase(it);
	else
		throw std::logic_error("PlacementBase::removeDetector() detector does not exist in this placement");
}

void PlacementBase::update()
{
	for (PlacementMediatorBase* pmb : m_sources)
		pmb->update();
	for (PlacementMediatorBase* pmb : m_detectors)
		pmb->update();
}

Source::Abstract* PlacementBase::source()
{
	if (m_sources.size()==0)
		return nullptr;
	else if (m_sources.size()==1)
		return m_sources.front()->source()->clone();
	else
	{
		vector<Source::Abstract*> s;

		for(const auto m : m_sources)
			s.push_back(m->source()->clone());

		return new Source::Composite(1.0f,std::move(s));
	}
}
