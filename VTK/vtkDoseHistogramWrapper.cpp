/*
 * vtkDoseHistogramWrapper.cpp
 *
 *  Created on: Aug 12, 2016
 *      Author: jcassidy
 */

#include "vtkDoseHistogramWrapper.h"

#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkAbstractArray.h>
#include <vtkObjectFactory.h>

#include <string>
#include "SwigWrapping.hpp"

#include <FullMonteSW/OutputTypes/AbstractSpatialMap.hpp>
#include <FullMonteSW/OutputTypes/SpatialMap.hpp>
#include <FullMonteSW/Queries/DoseHistogram.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <sstream>
#include <iostream>

using namespace std;

vtkDoseHistogramWrapper::vtkDoseHistogramWrapper()
{
	m_vtkTable = vtkTable::New();

	vtkFloatArray* vtkArea = vtkFloatArray::New();
	vtkArea->SetName("Surface Area%");
	m_vtkTable->AddColumn(vtkArea);

	vtkFloatArray* vtkDose = vtkFloatArray::New();
	vtkDose->SetName("Dose (J/cm2)");
	m_vtkTable->AddColumn(vtkDose);
}

vtkDoseHistogramWrapper::~vtkDoseHistogramWrapper()
{
	m_vtkTable->Delete();
}

void vtkDoseHistogramWrapper::source(const char *mptr)
{
	SwigPointerInfo pInfo = readSwigPointer(mptr);
	string type(pInfo.type.first, pInfo.type.second-pInfo.type.first);

	if (pInfo.p)
	{
		if (type == "OutputData")
			source(static_cast<const OutputData*>(pInfo.p));
		else
		{
			cout << "ERROR: SWIG pointer '" << mptr << "' is not a VolumeAbsorbedEnergyDensityMap, actually a " << type << endl;
			source(static_cast<const OutputData*>(nullptr));
		}
	}
	else
	{
		cout << "ERROR: Failed to convert SWIG pointer '" << mptr << "'" << endl;
		source(static_cast<const OutputData*>(nullptr));
	}
}

void vtkDoseHistogramWrapper::source(const OutputData* D)
{
	if (const auto* p = dynamic_cast<const DoseHistogram*>(D))
	{
		m_histogram = p;
		update();
	}
	else
	{
		m_histogram = nullptr;
		cout << "ERROR: vtkDoseHistogramWrapper::source(OutputData*) can't be cast to DoseHistogram" << endl;
	}
}


void vtkDoseHistogramWrapper::update()
{
	if (!m_histogram)
	{
		std::cout << "ERROR: vtkDoseHistogramWrapper::update() with null histogram pointer" << std::endl;
		return;
	}

	m_vtkTable->SetNumberOfRows(m_histogram->dim());

	// Replace F(x) with 1-F(x) to match DVH convention

	// CDF returns (x,F(x))

	for(unsigned i=0;i<m_histogram->dim();++i)
	{
		// dose
		static_cast<vtkFloatArray*>(m_vtkTable->GetColumn(0))->SetValue(i,(*m_histogram)[i].first);

		// 1- %vol
		static_cast<vtkFloatArray*>(m_vtkTable->GetColumn(1))->SetValue(i,1.0f-(*m_histogram)[i].second);
	}
}

vtkTable* vtkDoseHistogramWrapper::table() const
{
	return m_vtkTable;
}

vtkStandardNewMacro(vtkDoseHistogramWrapper)

