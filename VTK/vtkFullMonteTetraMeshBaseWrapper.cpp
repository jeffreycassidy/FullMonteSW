/*
 * vtkFullMonteTetraMeshBaseWrapper.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedShortArray.h>
#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>
#include <vtkObjectFactory.h>


#include "vtkFullMonteTetraMeshBaseWrapper.h"

#include <FullMonteSW/Geometry/TetraMeshBase.hpp>
#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>
#include <FullMonteSW/Geometry/BoundingBox.hpp>


using namespace std;


//vtkCxxRevisionMacro(vtkFullMonteTetraMeshBaseWrapper,"Rev 0.0")
vtkStandardNewMacro(vtkFullMonteTetraMeshBaseWrapper)

vtkFullMonteTetraMeshBaseWrapper::vtkFullMonteTetraMeshBaseWrapper()
{
	m_points=vtkPoints::New();
	m_tetras=vtkCellArray::New();

	m_regions=vtkUnsignedShortArray::New();
		m_regions->SetName("Tissue Type");
}

vtkFullMonteTetraMeshBaseWrapper::~vtkFullMonteTetraMeshBaseWrapper()
{
	m_points->Delete();
	m_regions->Delete();
	m_tetras->Delete();
}

void vtkFullMonteTetraMeshBaseWrapper::mesh(const TetraMeshBase* m)
{
	m_mesh = m;
	update();
}

const TetraMeshBase* vtkFullMonteTetraMeshBaseWrapper::mesh() const
{
	return m_mesh;
}

vtkPoints* vtkFullMonteTetraMeshBaseWrapper::points() const
{
	return m_points;
}

vtkUnsignedShortArray* vtkFullMonteTetraMeshBaseWrapper::regions() const
{
	return m_regions;
}

void vtkFullMonteTetraMeshBaseWrapper::update()
{
	assert(m_points);
	assert(m_regions);
	assert(m_tetras);
	assert(m_mesh);

	getVTKPoints(*m_mesh,m_points);
	getVTKTetraCells(*m_mesh,m_tetras);
	getVTKTetraRegions(*m_mesh,m_regions);

	Modified();
}

vtkUnstructuredGrid* vtkFullMonteTetraMeshBaseWrapper::regionMesh() const
{
	vtkUnstructuredGrid* ug = blankMesh();
	ug->GetCellData()->SetScalars(m_regions);
	return ug;
}

vtkUnstructuredGrid* vtkFullMonteTetraMeshBaseWrapper::blankMesh() const
{
	vtkUnstructuredGrid* ug = vtkUnstructuredGrid::New();
	ug->SetPoints(m_points);
	ug->SetCells(VTK_TETRA,m_tetras);
	return ug;
}


/** Extract vtkPoints from a TetraMeshBase. Point 0 is set to minimum bounding-box corner.
 * If 0 is set to (0,0,0) then can give some funny results when computing bounding box for point sets that are far from the
 * origin.
 *
 * @param 	M		The mesh
 * @param	P		Destination point set (will be overwritten)
 */

void getVTKPoints(const TetraMeshBase& M,vtkPoints* P)
{
	assert(P);

	P->SetNumberOfPoints(M.getNp()+1);

	OrthoBoundingBox<double,3> bb;

	vtkIdType i=0;
	for(const auto p : M.points())
	{
		Point<3,double> Pc = get(point_coords,M,p);
		if (i != 0)
			bb.insert(Pc);
		P->SetPoint(i++,Pc.data());
	}
	P->SetPoint(0,bb.corners().first.data());
}



/** Copy the TetraMeshBase tetras, including the 0 element (dummy containing 0,0,0,0)
 *
 */

void getVTKTetraCells(const TetraMeshBase& M,vtkCellArray* ca,const FilterBase<unsigned> *F)
{
	assert(ca);

	size_t Nt=0;

	// Create tetra ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);

	for(unsigned i=0;i<=M.getNt();++i)
	{
		if (!F || (*F)(i))
		{
			TetraByPointID IDps = M.getTetraPointIDs(i);

			// copy regular elements 1..Nt to tetras 1..Nt
			ids->InsertNextTuple1(4);
			for(unsigned k=0;k<4;++k)
			{
				ids->InsertNextTuple1((vtkIdType)(IDps[k]));
				assert(IDps[k] < M.getNp()+1);
			}
			++Nt;
		}
	}

	ca->SetCells(Nt, ids);
	ids->Delete();
}


/** Get region codes for all tetra elements (assign tetra 0 -> region 0 always)
 * @param	M		The mesh
 * @param	R		vtkUnsignedShortArray to hold region codes (will be overwritten)
 */

void getVTKTetraRegions(const TetraMeshBase& M,vtkUnsignedShortArray* R,const FilterBase<unsigned>* F)
{
	assert(R);

	R->SetNumberOfComponents(1);

	if (F)
	{
		R->SetNumberOfTuples(0);
		for(unsigned i=1; i <= M.getNt(); ++i)
			if ((*F)(i))
				R->InsertNextTuple1(M.getMaterial(i));
	}
	else
	{
		R->SetNumberOfTuples(M.getNt()+1);
		for(unsigned i=1; i <= M.getNt(); ++i)
			R->SetTuple1(i,M.getMaterial(i));
		R->SetTuple1(0,0);
	}

}

vtkIdList* vtkFullMonteTetraMeshBaseWrapper::getTetraIDsFromFilter(const FilterBase<unsigned int>* F)
{
	if (!F)
	{
		return nullptr;
	}
	else if (!m_mesh)
	{
		return nullptr;
	}

	vtkIdList* L = vtkIdList::New();

	for(unsigned i=1;i<m_mesh->getNt()+1;++i)
		if ((*F)(i))
			L->InsertNextId(i);

	return L;
}


