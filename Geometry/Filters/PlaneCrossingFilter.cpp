/*
 * PlaneCrossingFilter.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: jcassidy
 */

#include "PlaneCrossingFilter.hpp"
#include <FullMonteSW/Geometry/StandardArrayKernel.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>

using namespace std;

PlaneCrossingFilter::PlaneCrossingFilter()
{
}

PlaneCrossingFilter::~PlaneCrossingFilter()
{
}

void PlaneCrossingFilter::plane(const std::array<float,3> normal,float constant)
{
	m_normal = normal;
	m_constant = constant;
	postMeshUpdate();
}




/** Use previously-computed m_pointAbovePlane[IDp] on each point of the tetra. If any of them is on the opposite side to the
 * first point, then it crosses the plane.
 */

bool PlaneCrossingFilter::operator()(unsigned IDt) const
{
	TetraByPointID IDps = mesh()->getTetraPointIDs(IDt);

	bool p0above = m_pointAbovePlane[IDps[0]];

	for(unsigned p=1;p<4;++p)
		if (m_pointAbovePlane[IDps[p]] != p0above)
			return true;

	return false;
}


/** After mesh or plane is updated, recompute m_pointAbovePlane for all points
 *
 */

void PlaneCrossingFilter::postMeshUpdate()
{
	m_pointAbovePlane.resize(mesh()->getNp()+1);

	m_pointAbovePlane[0] = false;

	for(unsigned i=1;i<=mesh()->getNp();++i)
	{
		Point<3,double> P = mesh()->getPoint(i);
		array<float,3> Pf{ float(P[0]), float(P[1]), float(P[2]) };

		m_pointAbovePlane[i] = dot(m_normal,Pf) > m_constant;
	}
}
