/*
 * PlanePlacementLineSource.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Geometry/Sources/Line.hpp>
#include <FullMonteSW/Geometry/Placement/PlanePlacementLineSource.hpp>

void PlanePlacementLineSource::planePosition(Point2 p)
{
	m_pos = p;
	update();
}

Point2 PlanePlacementLineSource::planePosition() const
{
	return m_pos;
}

float PlanePlacementLineSource::tipDepth() const
{
	return m_endDepths.second;
}

void PlanePlacementLineSource::length(float l)
{
	m_endDepths.second = m_endDepths.first+l;
	update();
}

void PlanePlacementLineSource::extend(float dl)
{
	m_endDepths.second += dl;
	update();
}

void PlanePlacementLineSource::push(float dz)
{
	m_endDepths.first += dz;
	m_endDepths.second+= dz;
	update();
}

float PlanePlacementLineSource::length() const
{
	return m_endDepths.second-m_endDepths.first;
}

void PlanePlacementLineSource::update()
{
	source()->endpoint(0,placement()->basis().invert(appendCoord(m_pos,-m_endDepths.first)));
	source()->endpoint(1,placement()->basis().invert(appendCoord(m_pos,-m_endDepths.second)));
}


