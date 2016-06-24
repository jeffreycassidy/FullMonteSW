/*
 * TetraMeshFilterBase.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Geometry/Filters/TetraMeshBaseFilter.hpp>

TetraMeshBaseFilter::TetraMeshBaseFilter(const TetraMeshBase* m)
{
	mesh(m);
}

const TetraMeshBase* TetraMeshBaseFilter::mesh() const
{
	return m_mesh;
}

void TetraMeshBaseFilter::mesh(const TetraMeshBase* m)
{
	m_mesh=m;
	postMeshUpdate();
}
