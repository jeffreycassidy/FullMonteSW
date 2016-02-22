/*
 * TetraMeshBaseVTK.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */

#include <vtkPoints.h>
#include <vtkIdTypeArray.h>
#include <vtkCellArray.h>
#include <vtkUnsignedShortArray.h>

#include <FullMonte/Geometry/TetraMeshBase.hpp>

#include <FullMonte/Geometry/BoundingBox.hpp>

#include <limits>

using namespace std;



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
	for(Point<3,double> p : M.points())
	{
		if (i != 0)
			bb.insert(p);
		P->SetPoint(i++,p.data());
	}
	P->SetPoint(0,bb.corners().first.data());
}



/** Copy the TetraMeshBase tetras, including the 0 element (dummy containing 0,0,0,0)
 *
 */

void getVTKTetraCells(const TetraMeshBase& M,vtkCellArray* ca)
{
	assert(ca);

	size_t Nt=M.getNt()+1;

	// Create tetra ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);
	ids->SetNumberOfTuples(5*Nt);

	vtkIdType j=0;
	for(TetraByPointID IDps : M.getTetrasByPointID())
	{
		if (j != 0)
		{
			// copy regular elements 1..Nt to tetras 1..Nt
			ids->SetTuple1(j++,4);
			for(unsigned k=0;k<4;++k)
			{
				ids->SetValue(j++,(vtkIdType)(IDps[k]));
				assert(IDps[k] < M.getNp()+1);
			}
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
	assert(j == 5*Nt);

	ca->SetCells(Nt, ids);
	ids->Delete();
}


/** Get region codes for all tetra elements (assign tetra 0 -> region 0 always)
 * @param	M		The mesh
 * @param	R		vtkUnsignedShortArray to hold region codes (will be overwritten)
 */

void getVTKTetraRegions(const TetraMeshBase& M,vtkUnsignedShortArray* R)
{
	assert(R);

	R->SetNumberOfComponents(1);
	R->SetNumberOfTuples(M.getNt()+1);

	for(unsigned i=1; i <= M.getNt(); ++i)
		R->SetTuple1(i,M.getMaterial(i));
	R->SetTuple1(0,0);
}
