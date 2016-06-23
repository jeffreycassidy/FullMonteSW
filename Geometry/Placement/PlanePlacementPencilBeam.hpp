/*
 * PlanePlacementPencilBeam.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_PLACEMENT_PLANEPLACEMENTPENCILBEAM_HPP_
#define GEOMETRY_PLACEMENT_PLANEPLACEMENTPENCILBEAM_HPP_

#include <FullMonteSW/Geometry/Placement/PlacementMediatorBase.hpp>
#include <FullMonteSW/Geometry/Placement/PlanePlacement.hpp>

namespace Source { class PencilBeam; }

class PlanePlacementPencilBeam : public PlacementMediatorInstance<PlanePlacement,Source::PencilBeam>
{
public:
	virtual void update() override;

	void		planePosition(Point2 p);

private:
	Point2		m_pos;
};


#endif /* GEOMETRY_PLACEMENT_PLANEPLACEMENTPENCILBEAM_HPP_ */
