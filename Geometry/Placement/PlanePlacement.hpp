/*
 * PlanePlacement.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_PLACEMENT_PLANEPLACEMENT_HPP_
#define GEOMETRY_PLACEMENT_PLANEPLACEMENT_HPP_

#include <FullMonteSW/Geometry/Basis.hpp>
#include <FullMonteSW/Geometry/Placement/PlacementBase.hpp>

/** Provides placement of sources relative to a plane.
 *
 * Looking along the direction of source insertion (-z), +x is right and +y is up.
 */

class PlanePlacement : public PlacementBase
{
public:
	virtual ~PlanePlacement();

	UnitVector3			normal() 		const;
	Point3				invert(Point2) 	const;

	Basis&				basis(){ return m_basis; }


private:
	Basis m_basis = Basis::standard();
};



#endif /* GEOMETRY_PLACEMENT_PLANEPLACEMENT_HPP_ */
