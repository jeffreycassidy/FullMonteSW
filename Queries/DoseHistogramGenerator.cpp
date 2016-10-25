/*
 * DoseSurfaceHistogramGenerator.cpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/OutputTypes/SpatialMap.hpp>
#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include "DoseHistogramGenerator.hpp"

#include "DoseHistogram.hpp"

#include "EmpiricalCDF.hpp"
#include <vector>

using namespace std;
#include <iostream>

DoseHistogramGenerator::DoseHistogramGenerator()
{
}

void DoseHistogramGenerator::mesh(const TetraMesh* M)
{
	m_mesh=M;
}

const TetraMesh* DoseHistogramGenerator::mesh() const
{
	return m_mesh;
}

void DoseHistogramGenerator::filter(FilterBase<int>* F)
{
	m_filter=F;
}

const FilterBase<int>* DoseHistogramGenerator::filter() const
{
	return m_filter;
}

OutputData* DoseHistogramGenerator::result() const
{
	vector<pair<float,float>> v;

//	cout << "mesh=" << m_mesh << " filter=" << m_filter << " phi=" << m_phi << endl;
//
//	cout << "Calculating dose-surface histogram for mesh with " << m_mesh->getNp() << " points and " << m_mesh->getNf() << " faces" << endl;
//	cout << "  Fluence output has dimension " << m_phi->dim() << endl;

	for(int i=0;i<=m_mesh->getNf();++i)
	{
		if ((*m_filter)(i))
			v.emplace_back((*m_phi)[i],m_mesh->getFaceArea(i));
	}

	DoseHistogram* DH = new DoseHistogram(v);

	//DH->print();

	return DH;
}

void DoseHistogramGenerator::dose(const OutputData* phi)
{
	if (auto p = dynamic_cast<const SpatialMap<float>*>(phi))
		m_phi = p;
	else
	{
		m_phi = nullptr;
		cout << "ERROR: DoseHistogramGenerator::dose(OutputData*) can't be cast to a SpatialMap<float>" << endl;
	}
}

const SpatialMap<float>* DoseHistogramGenerator::dose() const
{
	return m_phi;
}
