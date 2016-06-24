/*
 * VTKLineCluster.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#include "VTKLineCluster.hpp"

#include <array>
#include <string>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>

#include <vtkPolyDataWriter.h>

VTKLineCluster::VTKLineCluster()
{
	m_vtkP = vtkPoints::New();

	m_vtkD = vtkFloatArray::New();
	m_vtkD->SetNumberOfComponents(3);
}

void VTKLineCluster::write(const std::string& fn)
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
	pd->GetPointData()->SetNormals(m_vtkD);


	vtkPolyDataWriter* W = vtkPolyDataWriter::New();
	W->SetFileName(fn.c_str());
	W->SetInputData(pd);
	W->Update();
	W->Delete();

	pd->Delete();
	ca->Delete();
	idps->Delete();
}

void VTKLineCluster::clear()
{
	m_vtkP->SetNumberOfPoints(0);
}

void VTKLineCluster::add(std::array<float,3> p,std::array<float,3> d)
{
	m_vtkP->InsertNextPoint(p[0],p[1],p[2]);
	m_vtkD->InsertNextTuple3(d[0],d[1],d[2]);
}

void VTKLineCluster::add(std::array<double,3> p,std::array<double,3> d)
{
	m_vtkP->InsertNextPoint(p[0],p[1],p[2]);
	m_vtkD->InsertNextTuple3(d[0],d[1],d[2]);
}

