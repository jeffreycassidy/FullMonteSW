/*
 * VTKLegacyReader.cpp
 *
 *  Created on: May 27, 2015
 *      Author: jcassidy
 */

#include "VTKLegacyReader.hpp"
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkCellTypes.h>
#include <vtkCellData.h>

VTKLegacyReader::~VTKLegacyReader()
{

}

/** Convert a VTK Unstructured Grid into a TetraMesh. Uses the current scalars to assign regions, unless a name is
 * specified, in which case the active scalars are set to that name first.
 *
 * If renumberZero_ is defined, then move element zero to back of list and replace it with a null element
 *   Needs to be done if cell/point 0 is a non-null element because FullMonte assumes 0 is a null
 *   We move element 0 to back so that all other indices are the same instead of shifted by one
 *
 */

TetraMeshBase VTKLegacyReader::convertToMesh(vtkUnstructuredGrid* ug) const
{
	unsigned Nvertices = ug->GetNumberOfPoints();
	unsigned Ncells = ug->GetNumberOfCells();

	vector<Point<3,double>> 		P	(Nvertices+renumberZero_,	Point<3,double>{0,0,0});
	vector<TetraByPointID> 			T	(Ncells+renumberZero_,		std::array<unsigned,4>{0,0,0,0});
	vector<unsigned>				T_m	(Ncells+renumberZero_,		0);

	vtkCellData* cd = ((vtkUnstructuredGrid*)ug)->GetCellData();

	for(unsigned i=renumberZero_;i<Nvertices+renumberZero_;++i)
	{
		array<double,3> Pd;
		ug->GetPoint(i,Pd.data());
		boost::copy(Pd,P[i-renumberZero_].data());
	}

	if (renumberZero_)
		((vtkUnstructuredGrid*)ug)->GetPoint(0,P.back().data());

	for(unsigned i=renumberZero_;i<Ncells+renumberZero_; ++i)
	{
		vtkIdType* p;
		vtkIdType Npc=0;

		// get source data, either from i'th element, or element 0 to wrap around at end
		unsigned src = i < Ncells ? i : 0;

		assert(((vtkUnstructuredGrid*)ug)->GetCellType(src)==VTK_TETRA);
		((vtkUnstructuredGrid*)ug)->GetCellPoints(src,Npc,p);
		assert(Npc==4);

		// destination is always i'th element
		copy(p,p+4,T[i].data());
		T_m[i] = cd->GetScalars()->GetTuple1(i);
	}

	return TetraMeshBase(P,T,T_m);
}

TetraMeshBase VTKLegacyReader::mesh() const
{
	vtkUnstructuredGridReader* R = vtkUnstructuredGridReader::New();

	R->SetFileName(meshFn_.c_str());
	R->Update();
	vtkUnstructuredGrid* g = R->GetOutput();
	cout << "Read unstructured grid with " << g->GetNumberOfPoints() << " points and " << g->GetNumberOfCells() << " cells" << endl;

	TetraMeshBase M = convertToMesh(g);
	g->Delete();
	return M;
}


