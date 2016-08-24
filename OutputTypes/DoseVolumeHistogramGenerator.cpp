/*
 * DoseVolumeHistogramGenerator.cpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */

#include "DoseVolumeHistogramGenerator.hpp"

#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/OutputTypes/FluenceMapBase.hpp>

DoseVolumeHistogramGenerator::DoseVolumeHistogramGenerator()
{
}

void DoseVolumeHistogramGenerator::mesh(const TetraMesh* M)
{
	m_mesh=M;
}

const TetraMesh* DoseVolumeHistogramGenerator::mesh() const
{
	return m_mesh;
}

void DoseVolumeHistogramGenerator::filter(FilterBase<unsigned>* F)
{
	m_filter=F;
}

const FilterBase<unsigned>* DoseVolumeHistogramGenerator::filter() const
{
	return m_filter;
}

EmpiricalCDF<float,float> DoseVolumeHistogramGenerator::result() const
{
	vector<pair<float,float>> v;

	cout << "mesh=" << m_mesh << " filter=" << m_filter << " phi=" << m_phi << endl;

	cout << "Calculating dose-volume histogram for mesh with " << m_mesh->getNp() << " points and " << m_mesh->getNf() << " faces" << endl;
	cout << "  Fluence output has dimension " << (*m_phi)->dim() << endl;

	for(unsigned i=0;i<=m_mesh->getNt();++i)
	{
		if ((*m_filter)(i))
			v.emplace_back((*m_phi)[i],m_mesh->getTetraVolume(i));
	}

	return EmpiricalCDF<float,float>(v);
}

void DoseVolumeHistogramGenerator::fluence(const VolumeFluenceMap* phi)
{
	m_phi = phi;
}

const VolumeFluenceMap* DoseVolumeHistogramGenerator::fluence() const
{
	return m_phi;
}
