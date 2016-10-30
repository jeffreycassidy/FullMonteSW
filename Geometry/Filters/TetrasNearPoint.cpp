/*
 * TetrasNearPoint.cpp
 *
 *  Created on: Oct 26, 2016
 *      Author: jcassidy
 */

#include "TetrasNearPoint.hpp"

#include <FullMonteSW/Geometry/newgeom.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>

#include <cmath>

TetrasNearPoint::TetrasNearPoint()
{
}

TetrasNearPoint::~TetrasNearPoint()
{
}

bool TetrasNearPoint::operator()(unsigned IDt) const
{
	bool any=false,all=true;
	TetraByPointID IDps=m_mesh->getTetraPointIDs(IDt);

	for(unsigned i=0;i<4;++i)
	{
		bool within = Vector<3,double>(m_centre, get(point_coords,*m_mesh,TetraMeshBase::PointDescriptor(IDps[i])) ).norm2_l2()<m_radius2;
		any |= within;
		all &= within;
	}

	return (m_requireAll & all) | any;
}

void TetrasNearPoint::centre(Point<3,double> P)
{
	m_centre=P;
}

Point<3,double> TetrasNearPoint::centre() const
{
	return m_centre;
}

float TetrasNearPoint::radius() const
{
	return std::sqrt(m_radius2);
}

void TetrasNearPoint::radius(float r)
{
	m_radius2 = r*r;
}

void TetrasNearPoint::mesh(const TetraMesh* M)
{
	m_mesh=M;
}

const TetraMesh* TetrasNearPoint::mesh() const
{
	return m_mesh;
}
