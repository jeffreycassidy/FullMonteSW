/*
 * DoseSurfaceHistogramGenerator.cpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */

#include "DoseSurfaceHistogramGenerator.hpp"

#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/OutputTypes/FluenceMapBase.hpp>

DoseSurfaceHistogramGenerator::DoseSurfaceHistogramGenerator()
{
}

void DoseSurfaceHistogramGenerator::mesh(const TetraMesh* M)
{
	m_mesh=M;
}

const TetraMesh* DoseSurfaceHistogramGenerator::mesh() const
{
	return m_mesh;
}

void DoseSurfaceHistogramGenerator::filter(FilterBase<int>* F)
{
	m_filter=F;
}

const FilterBase<int>* DoseSurfaceHistogramGenerator::filter() const
{
	return m_filter;
}

EmpiricalCDF<float,float> DoseSurfaceHistogramGenerator::result() const
{
	vector<pair<float,float>> v;

	cout << "mesh=" << m_mesh << " filter=" << m_filter << " phi=" << m_phi << endl;

	cout << "Calculating dose-surface histogram for mesh with " << m_mesh->getNp() << " points and " << m_mesh->getNf() << " faces" << endl;
	cout << "  Fluence output has dimension " << (*m_phi)->dim() << endl;

	for(int i=0;i<=m_mesh->getNf();++i)
	{
		if ((*m_filter)(i))
			v.emplace_back((*m_phi)[i],m_mesh->getFaceArea(i));
	}


	return EmpiricalCDF<float,float>(v);
}

void DoseSurfaceHistogramGenerator::fluence(const SurfaceFluenceMap* phi)
{
	m_phi = phi;
}

const SurfaceFluenceMap* DoseSurfaceHistogramGenerator::fluence() const
{
	return m_phi;
}
