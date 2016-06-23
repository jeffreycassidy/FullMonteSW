/*
 * PlacementMediatorBase.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Geometry/Placement/PlacementBase.hpp>
#include <FullMonteSW/Geometry/Placement/PlacementMediatorBase.hpp>

PlacementMediatorBase::~PlacementMediatorBase()
{
	if (m_placement)
		m_placement->removeSource(this);
};

void PlacementMediatorBase::placement(PlacementBase* p)
{
	if (m_placement)
		m_placement->removeSource(this);

	m_placement=p;
	if (p)
		p->addSource(this);
}
