/*
 * TetraMeshVTK.cpp
 *
 *  Created on: Nov 7, 2015
 *      Author: jcassidy
 */

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkObjectFactory.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellData.h>

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/BoundingBox.hpp>

#include <FullMonte/VTK/TetraMeshVTK.h>

vtkCxxRevisionMacro(TetraMeshVTK,"Rev 0.0");
vtkStandardNewMacro(TetraMeshVTK);

TetraMeshVTK::TetraMeshVTK()
{
}

TetraMeshVTK::~TetraMeshVTK()
{
}

void TetraMeshVTK::PrintSelf(std::ostream& os,vtkIndent indent)
{
	vtkObject::PrintSelf(os,indent);
}

void TetraMeshVTK::setInputTetraMesh(const TetraMesh* M)
{
	////// Update pointer to source
	m_M = M;
	update();
}

void TetraMeshVTK::update()
{
	////// Update points

	if(!m_vtkP)
		m_vtkP=vtkPoints::New();

	m_vtkP->SetNumberOfPoints(m_M->getNp()+1);

	OrthoBoundingBox<double,3> bb;

	unsigned i=0;
	for(Point<3,double> p : m_M->points())
	{
		if (i!=0)
			bb.insert(p);
		m_vtkP->SetPoint(i++,p.data());
	}
	m_vtkP->SetPoint(0,bb.corners().first.data());		// set point 0 to be lower-left corner of bounding box



	////// Update tetras, including dummy element (0)
	size_t Nt=m_M->getNt()+1;

	// Create tetra ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);
	ids->SetNumberOfTuples(5*Nt);

	unsigned j=0;
	for(TetraByPointID IDps : m_M->getTetrasByPointID())
	{
		if (j != 0)
		{
			// copy regular elements 1..Nt to tetras 1..Nt
			ids->SetTuple1(j++,4);
			for(unsigned k=0;k<4;++k)
				ids->SetTuple1(j++,IDps[k]);
		}
		else
		{
			// copy dummy element (0,0,0,0) to tetra 0
			ids->SetTuple1(0,4);
			for(unsigned k=1;k<5;++k)
				ids->SetTuple1(k,0);
			j += 5;
		}
	}

	// Form cell array
	if (!m_vtkT)
		m_vtkT = vtkCellArray::New();

	m_vtkT->SetCells(Nt, ids);



	////// Update regions

	if (!m_vtkRegions)
		m_vtkRegions = vtkUnsignedShortArray::New();

	m_vtkRegions->SetNumberOfComponents(1);
	m_vtkRegions->SetNumberOfTuples(m_M->getNt()+1);

	for(unsigned i=1; i <= m_M->getNt(); ++i)
		m_vtkRegions->SetTuple1(i,m_M->getMaterial(i));

	m_vtkRegions->SetTuple1(0,0);


	Modified();
}
//
//vtkUnstructuredGrid* TetraMeshVTK::getBlankMesh() const
//{
//	vtkUnstructuredGrid* ug = vtkUnstructuredGrid::New();
//
//	ug->SetPoints(m_vtkP);
//	ug->SetCells(VTK_TETRA,m_vtkT);
//	return ug;
//}
//
//vtkUnstructuredGrid* TetraMeshVTK::getRegions() const
//{
//	vtkUnstructuredGrid* ug = getBlankMesh();
//	ug->GetCellData()->SetActiveScalars("regions");
//	ug->GetCellData()->SetScalars(m_vtkRegions);
//
//	return ug;
//}
