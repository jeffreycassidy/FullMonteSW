/*
 * vtkFullMonteTetraMeshBaseWrapper.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/BoundingBox.hpp>
#include <FullMonte/Geometry/Filters/FilterBase.hpp>

#include "vtkFullMonteTetraMeshWrapper.h"

#include "SwigWrapping.hpp"

using namespace std;

vtkStandardNewMacro(vtkFullMonteTetraMeshWrapper)


vtkFullMonteTetraMeshWrapper::vtkFullMonteTetraMeshWrapper()
{
	m_faces=vtkCellArray::New();
}

vtkFullMonteTetraMeshWrapper::~vtkFullMonteTetraMeshWrapper()
{
	m_faces->Delete();
}

void vtkFullMonteTetraMeshWrapper::mesh(const char* mptr)
{
	SwigPointerInfo pi = readSwigPointer(mptr);

	string s(pi.type.first,pi.type.second-pi.type.first);

	cout << "vtkFullMonteTetraMeshWrapper::mesh(const char* mptr) received type " << s << " (SWIG string " << mptr << ")" << endl;
	mesh(static_cast<const TetraMesh*>(pi.p));
}

void vtkFullMonteTetraMeshWrapper::mesh(const TetraMesh* m)
{
	vtkFullMonteTetraMeshBaseWrapper::mesh(static_cast<const TetraMeshBase*>(m));
	vtkFullMonteTetraMeshWrapper::update();
}

const TetraMesh* vtkFullMonteTetraMeshWrapper::mesh() const
{
	return static_cast<const TetraMesh*>(vtkFullMonteTetraMeshBaseWrapper::mesh());
}

void vtkFullMonteTetraMeshWrapper::update()
{
	assert(m_faces);
	getVTKTriangleCells(*mesh(),m_faces,m_faceFilter);
	Modified();
}

vtkPolyData* vtkFullMonteTetraMeshWrapper::faces() const
{
	vtkPolyData *pd = vtkPolyData::New();
	pd->SetPoints(points());
	pd->SetPolys(m_faces);
	return pd;
}

void vtkFullMonteTetraMeshWrapper::faceFilter(FilterBase<int>* f)
{
	m_faceFilter=f;
	update();
}

void vtkFullMonteTetraMeshWrapper::faceFilter(const char* fptr)
{
	SwigPointerInfo pi = readSwigPointer(fptr);
	if (pi.p)
	{
		cout << "Used string " << fptr << " to set SWIG faceFilter" << endl;
		m_faceFilter = static_cast<FilterBase<int>*>(pi.p);
	}
	else
	{
		cout << "Need to look up vtk TCL wrapped object " << fptr << endl;
		cout << "  value: " << endl;
		//m_faceFilter = static_cast<FilterBase<int>*>
	}
}


/** Copy the TetraMesh faces, including the 0 element (dummy containing 0,0,0)
 *
 */

void getVTKTriangleCells(const TetraMesh& M,vtkCellArray* ca,const FilterBase<int>* F)
{
	assert(ca);

	size_t Nf=0;

	// Create triangle ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);

	for(unsigned i=0;i<=M.getNf();++i)
	{
		if (!F || (*F)(i))
		{
			FaceByPointID IDps = M.getFacePointIDs(i);
			// copy regular elements 1..Nt to tetras 1..Nt
			ids->InsertNextValue(3);
			for(unsigned k=0;k<3;++k)
			{
				ids->InsertNextValue((vtkIdType)(IDps[k]));
				assert(IDps[k] < M.getNp()+1);
			}
			++Nf;
		}
	}

	ca->SetCells(Nf, ids);
	ids->Delete();
}
