/*
 * TetraMeshFilter.cpp
 *
 *  Created on: Mar 16, 2016
 *      Author: jcassidy
 */

#include "TetraMeshFilterBase.hpp"

TetraMeshFilterBase::~TetraMeshFilterBase()
{
}

TetraMeshFilterBase::TetraMeshFilterBase(const TetraMesh* m)
{
	mesh(m);
}

const TetraMesh* TetraMeshFilterBase::mesh() const
{
	return m_mesh;
}

void TetraMeshFilterBase::mesh(const TetraMesh* m)
{
	m_mesh=m;
	updateMesh();

}

void TetraMeshFilterBase::postMeshUpdate()
{

}

void TetraMeshFilterBase::updateMesh()
{
	this->postMeshUpdate();
}
