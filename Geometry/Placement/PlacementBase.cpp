/*
 * PlacementBase.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#include <stdexcept>

#include <FullMonte/Geometry/Sources/Base.hpp>
#include <FullMonte/Geometry/Sources/Composite.hpp>
#include <FullMonte/Geometry/Placement/PlacementBase.hpp>
#include <FullMonte/Geometry/Placement/PlacementMediatorBase.hpp>

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

void PlacementBase::removeSource(PlacementMediatorBase* pmb)
{
	auto it = std::find(m_sources.begin(), m_sources.end(), pmb);
	if (it != m_sources.end())
		m_sources.erase(it);
	else
		throw std::logic_error("PlacementBase::removeSource() source does not exist in this placement");
}

void PlacementBase::update()
{
	for (PlacementMediatorBase* pmb : m_sources)
		pmb->update();
}

Source::Base* PlacementBase::source()
{
	if (m_sources.size()==0)
		return nullptr;
	else if (m_sources.size()==1)
		return m_sources.front()->source()->cloneSelf();
	else
	{
		vector<Source::Base*> s;

		for(const auto m : m_sources)
			s.push_back(m->source()->cloneSelf());

		return new Source::Composite(1.0f,std::move(s));
	}

}
