/*
 * PlanePlacementLineSource.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_PLACEMENT_PLANEPLACEMENTLINESOURCE_HPP_
#define GEOMETRY_PLACEMENT_PLANEPLACEMENTLINESOURCE_HPP_

#include <FullMonteSW/Geometry/Placement/PlacementMediatorBase.hpp>
#include <FullMonteSW/Geometry/Placement/PlanePlacement.hpp>

namespace Source { class Line; }

class PlanePlacementLineSource : public PlacementMediatorInstance<PlanePlacement,Source::Line>
{
public:
	/// Extend/shorten the source from the end
	void extend(float dl);
	void shorten(float dl){	extend(-dl);	}

	/// Push/pull the source along the normal, conserving length
	void push(float dz);
	void pull(float dz){	push(-dz);	}

	/// Get/set the position in the plane
	void 	planePosition(Point2 p);
	Point2 	planePosition() 			const;

	/// Get the insertion direction
	UnitVector3 direction()				const;

	/// Get/set the length of the probe, extending/shortening from the end
	void 	length(float l);
	float 	length() 					const;

	/// Get the tip depth
	float 	tipDepth() 					const;

	/// Update the subject source def
	virtual void update() override;

private:
	Point2					m_pos;
	std::pair<float,float>	m_endDepths;
};



#endif /* GEOMETRY_PLACEMENT_PLANEPLACEMENTLINESOURCE_HPP_ */
