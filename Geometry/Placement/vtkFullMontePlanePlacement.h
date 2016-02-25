/*
 * vtkFullMontePlanePlacement.h
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_PLACEMENT_VTKFULLMONTEPLANEPLACEMENT_H_
#define GEOMETRY_PLACEMENT_VTKFULLMONTEPLANEPLACEMENT_H_

class PlanePlacement;
class vtkPlaneWidget;

class vtkFullMontePlanePlacement
{
public:

private:
	vtkPlaneWidget*		m_vtkPlane=nullptr;
	PlanePlacement		m_placement=nullptr;
};



#endif /* GEOMETRY_PLACEMENT_VTKFULLMONTEPLANEPLACEMENT_H_ */
