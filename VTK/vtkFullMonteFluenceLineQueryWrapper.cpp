/*
 * vtkFullMonteFluenceLineQueryWrapper.cpp
 *
 *  Created on: Mar 6, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Queries/FluenceLineQuery.hpp>
#include "vtkFullMonteFluenceLineQueryWrapper.h"

#include <vtkPoints.h>
#include <vtkIdTypeArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkFullMonteFluenceLineQueryWrapper);

vtkFullMonteFluenceLineQueryWrapper::~vtkFullMonteFluenceLineQueryWrapper()
{
	m_vtkPD->Delete();
}

vtkFullMonteFluenceLineQueryWrapper::vtkFullMonteFluenceLineQueryWrapper()
{
	m_vtkPD = vtkPolyData::New();
}

void vtkFullMonteFluenceLineQueryWrapper::fluenceLineQuery(const FluenceLineQuery* q)
{
	m_fluenceLineQuery=q;
}

const FluenceLineQuery* vtkFullMonteFluenceLineQueryWrapper::fluenceLineQuery() const
{
	return m_fluenceLineQuery;
}


void vtkFullMonteFluenceLineQueryWrapper::update()
{
	// get/clear points
	vtkPoints* P = vtkPoints::New();
	m_vtkPD->SetPoints(P);

	// cell connectivity (0..N-1)
	vtkIdTypeArray* ids = vtkIdTypeArray::New();

	// create data arrays
	vtkFloatArray* tetPhi = vtkFloatArray::New();
		tetPhi->SetName("Volume Fluence J/cm2");

	vtkIdTypeArray* tetID = vtkIdTypeArray::New();
		tetID->SetName("Tetra ID");

	vtkUnsignedShortArray* tetMat = vtkUnsignedShortArray::New();
		tetMat->SetName("Tissue Type");


	// TODO: Enable to handle multiple traces

	// -- BEGIN add single trace
	vtkIdType offset_npts = ids->InsertNextValue(0);

	for(const auto seg : m_fluenceLineQuery->result())
	{
		if (seg.lSeg > m_minSegmentLength)
		{
			vtkIdType IDp = P->InsertNextPoint(seg.p0.data());		// start point of current segment
			ids->InsertNextValue(IDp);

			tetPhi->InsertNextValue(seg.phi);		// entering fluence
			tetMat->InsertNextValue(seg.matID);		// entering material
			tetID->InsertNextValue(seg.IDt);		// entering tetID
		}
	}

	ids->SetValue(offset_npts,ids->GetNumberOfTuples()-offset_npts-1);

	// -- END single trace

	// create
	vtkCellArray* ca = vtkCellArray::New();

	// set topology and associate with fields
	ca->SetCells(1,ids);

	m_vtkPD->SetLines(ca);
	m_vtkPD->GetPointData()->AddArray(tetPhi);
	m_vtkPD->GetPointData()->AddArray(tetID);
	m_vtkPD->GetPointData()->AddArray(tetMat);

	Modified();
}

vtkPolyData* vtkFullMonteFluenceLineQueryWrapper::getPolyData() const
{
	return m_vtkPD;
}


