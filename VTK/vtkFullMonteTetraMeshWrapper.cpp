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

#include "vtkFullMonteTetraMeshWrapper.h"

using namespace std;


//vtkCxxRevisionMacro(vtkFullMonteTetraMeshBaseWrapper,"Rev 0.0")
vtkStandardNewMacro(vtkFullMonteTetraMeshWrapper)


vtkFullMonteTetraMeshWrapper::vtkFullMonteTetraMeshWrapper()
{
	m_faces=vtkCellArray::New();
}

vtkFullMonteTetraMeshWrapper::~vtkFullMonteTetraMeshWrapper()
{
	m_faces->Delete();
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
	getVTKTriangleCells(*mesh(),m_faces);
	Modified();
}

vtkPolyData* vtkFullMonteTetraMeshWrapper::faces() const
{
	vtkPolyData *pd = vtkPolyData::New();
	pd->SetPoints(points());
	pd->SetPolys(m_faces);
	return pd;
}


/** Copy the TetraMesh faces, including the 0 element (dummy containing 0,0,0)
 *
 */

void getVTKTriangleCells(const TetraMesh& M,vtkCellArray* ca)
{
	assert(ca);

	size_t Nf=M.getNf()+1;

	// Create triangle ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);
	ids->SetNumberOfTuples(4*Nf);

	vtkIdType j=0;
	for(int i=0;i<Nf;++i)
	{
		FaceByPointID IDps = M.getFacePointIDs(i);
		if (j != 0)
		{
			// copy regular elements 1..Nt to tetras 1..Nt
			ids->SetTuple1(j++,3);
			for(unsigned k=0;k<3;++k)
			{
				ids->SetValue(j++,(vtkIdType)(IDps[k]));
				assert(IDps[k] < M.getNp()+1);
			}
		}
		else
		{
			// copy dummy element (0,0,0) to tetra 0
			ids->SetTuple1(0,3);
			for(unsigned k=1;k<3;++k)
				ids->SetTuple1(k,0);
			j += 4;
		}
	}
	assert(j == 5*Nf);

	ca->SetCells(Nf, ids);
	ids->Delete();
}
