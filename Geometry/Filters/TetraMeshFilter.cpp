/*
 * TetraMeshFilter.cpp
 *
 *  Created on: Mar 16, 2016
 *      Author: jcassidy
 */

#include <FullMonte/Geometry/Filters/TetraMeshFilter.hpp>

TetraMeshFilter::TetraMeshFilter(const TetraMesh* m)
{
	mesh(m);
}

const TetraMesh* TetraMeshFilter::mesh() const
{
	return m_mesh;
}

void TetraMeshFilter::mesh(const TetraMesh* m)
{
	m_mesh=m;
	this->postMeshUpdate();
}
