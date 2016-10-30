/*
 * TetrasNearPoint.hpp
 *
 *  Created on: Oct 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_FILTERS_TETRASNEARPOINT_HPP_
#define GEOMETRY_FILTERS_TETRASNEARPOINT_HPP_

#include "FilterBase.hpp"

#include <FullMonteSW/Geometry/newgeom.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <cmath>

class TetrasNearPoint : public FilterBase<unsigned>
{
public:
	TetrasNearPoint();
	virtual ~TetrasNearPoint();

	virtual bool operator()(unsigned IDt) const override;

	void				mesh(const TetraMesh* M);
	const TetraMesh*	mesh() 						const;

	void				centre(Point<3,double> P);
	Point<3,double>		centre() 					const;

	void				radius(float r);
	float				radius()					const;

private:
	Point<3,double>		m_centre;
	float				m_radius2=0.0f;
	bool				m_requireAll=false;
	const TetraMesh*	m_mesh=nullptr;
};


#endif /* GEOMETRY_FILTERS_TETRASNEARPOINT_HPP_ */
