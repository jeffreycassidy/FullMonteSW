/*
 * VTKPointCloud.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#include "VTKPointCloud.hpp"

#include <array>
#include <string>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <vtkPolyDataWriter.h>

VTKPointCloud::VTKPointCloud()
{
	m_vtkP = vtkPoints::New();
}

void VTKPointCloud::write(const std::string& fn)
{
	vtkIdTypeArray *idps = vtkIdTypeArray::New();
	idps->SetNumberOfTuples(2*m_vtkP->GetNumberOfPoints());

	for(unsigned i=0;i<m_vtkP->GetNumberOfPoints();i++)
	{
		idps->SetValue(i<<1,		1);
		idps->SetValue((i<<1)+1,	i);
	}

	vtkCellArray *ca = vtkCellArray::New();
	ca->SetCells(m_vtkP->GetNumberOfPoints(),idps);

	vtkPolyData *pd = vtkPolyData::New();
	pd->SetPoints(m_vtkP);
	pd->SetVerts(ca);


	vtkPolyDataWriter* W = vtkPolyDataWriter::New();
	W->SetFileName(fn.c_str());
	W->SetInputData(pd);
	W->Update();
	W->Delete();

	pd->Delete();
	ca->Delete();
	idps->Delete();
}

void VTKPointCloud::clear()
{
	m_vtkP->SetNumberOfPoints(0);
}

void VTKPointCloud::add(std::array<float,3> f)
{
	m_vtkP->InsertNextPoint(f.data());
}

void VTKPointCloud::add(std::array<double,3> d)
{
	m_vtkP->InsertNextPoint(d.data());
}

