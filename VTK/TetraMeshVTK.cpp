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

void TetraMeshVTK::setInputTetraMesh(const char* pStr)
{
	// kludgy, home-brewed pointer extraction
	// format is _XXXXXXXXX_p_TTTTTT where X is pointer value (little-endian hex) and T is type
	std::stringstream ss(pStr);
	std::string typeStr;
	char buf[5];
	uint64_t pval;
	ss.read(buf,1);
	ss >> std::hex >> pval;
	ss.read(buf+1,3);
	buf[4]=0;
	if (strcmp("__p_",buf))
		std::cout << "ERROR: Improperly formatted string" << std::endl;
	ss >> typeStr;

	// For some reason, SWIG stores pointers in little-endian strings; have to swap them before using!
	pval = (pval >> 32) | (pval << 32);
	pval = ((pval >> 16) & 0x0000ffff0000ffffULL) | ((pval << 16) & 0xffff0000ffff0000ULL);
	pval = ((pval >> 8 ) & 0x00ff00ff00ff00ffULL) | ((pval << 8)  & 0xff00ff00ff00ff00ULL);

	m_M = reinterpret_cast<const TetraMesh*>(pval);
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

vtkUnstructuredGrid* TetraMeshVTK::getBlankMesh() const
{
	vtkUnstructuredGrid* ug = vtkUnstructuredGrid::New();

	ug->SetPoints(m_vtkP);
	ug->SetCells(VTK_TETRA,m_vtkT);
	return ug;
}

vtkUnstructuredGrid* TetraMeshVTK::getRegions() const
{
	vtkUnstructuredGrid* ug = getBlankMesh();
	ug->GetCellData()->SetActiveScalars("regions");
	ug->GetCellData()->SetScalars(m_vtkRegions);

	return ug;
}
