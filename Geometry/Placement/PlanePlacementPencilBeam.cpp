/*
 * PlanePlacementPencilBeam.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Geometry/Sources/PencilBeam.hpp>
#include <FullMonteSW/Geometry/Placement/PlanePlacementPencilBeam.hpp>

void PlanePlacementPencilBeam::planePosition(Point2 p)
{
	m_pos = p;
	update();
}

void PlanePlacementPencilBeam::update()
{
	if (!source() || !placement())
		return;

	// get 3D from the plane placement
	source()->position(placement()->invert(m_pos));
	source()->direction(placement()->normal());
}
