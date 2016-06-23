/*
 * PlanePlacement.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Geometry/Placement/PlanePlacement.hpp>

PlanePlacement::~PlanePlacement()
{
}

UnitVector3 PlanePlacement::normal() const
{
	return m_basis.normal();
}

Point3 PlanePlacement::invert(Point2 p) const
{
	return m_basis.invert(p);
}



