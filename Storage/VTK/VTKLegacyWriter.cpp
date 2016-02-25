/*
 * VTKLegacyReader.cpp
 *
 *  Created on: May 27, 2015
 *      Author: jcassidy
 */

#include "VTKLegacyWriter.hpp"
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkIdTypeArray.h>
#include <vtkCellArray.h>
#include <vtkCharArray.h>
#include <vtkCellData.h>
#include <vtkCellTypes.h> 

/** Convert a VTK Unstructured Grid into a TetraMesh. Uses the current scalars to assign regions, unless a name is
 * specified, in which case the active scalars are set to that name first.
 *
 */

vtkUnstructuredGrid* VTKLegacyWriter::convertFromMesh(const TetraMeshBase& M) const
{
	vtkUnstructuredGrid* g = vtkUnstructuredGrid::New();

	vtkPoints* P = vtkPoints::New();

	P->SetNumberOfPoints(M.getNp()+1);
	for(unsigned i=0;i<=M.getNp();++i)
		P->SetPoint(i,M.getPoint(i).data());

	g->SetPoints(P);

	vtkIdTypeArray* ids = vtkIdTypeArray::New();
	ids->SetNumberOfTuples(5*(M.getNt()+1));

	unsigned j=0;
	for(unsigned i=0;i<=M.getNt(); ++i)
	{
		ids->SetTuple1(j++,4);
		TetraByPointID IDps = M.getTetraPointIDs(i);
		for(unsigned k=0;k<4;++k)
			ids->SetTuple1(j++,IDps[k]);
	}

	vtkCellArray *ca = vtkCellArray::New();
	ca->SetCells(M.getNt()+1,ids);

	g->SetCells(VTK_TETRA,ca);

	return g;
}

void VTKLegacyWriter::writeMeshRegions(const std::string fn,const TetraMeshBase& M) const
{
	vtkUnstructuredGridWriter* W = vtkUnstructuredGridWriter::New();

	W->SetFileName(fn.c_str());
	vtkUnstructuredGrid* g = convertFromMesh(M);
	cout << "Writing unstructured grid with " << g->GetNumberOfPoints() << " points and " << g->GetNumberOfCells() << " cells to " << fn << endl;

	// get the regions
	vtkCharArray* r = vtkCharArray::New();
	r->SetNumberOfTuples(M.getNt()+1);
	for(unsigned i=0;i<=M.getNt();++i)
		r->SetTuple1(i,M.getMaterial(i));

	g->GetCellData()->SetScalars(r);
	W->SetScalarsName("regions");

	// write and delete
	W->SetInputData(g);
	W->Update();
	cout << "  Done" << endl;
	g->Delete();
	W->Delete();
}


