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
 * If a dummy point is prepended, uses the first point in the mesh
 * If a dummy cell is prepended, uses 0 0 0 0 as usual
 */

TetraMeshBase VTKLegacyReader::convertToMesh(vtkUnstructuredGrid* ug) const
{
	unsigned Nvertices = ug->GetNumberOfPoints();
	unsigned Ncells = ug->GetNumberOfCells();

	vector<Point<3,double>> 		P	(Nvertices+m_addZeroPoint);
	vector<TetraByPointID> 			T	(Ncells+m_addZeroCell);
	vector<unsigned>				T_m	(Ncells+m_addZeroCell);

	vtkCellData* cd = ug->GetCellData();
	vtkDataArray* regions=nullptr;

	if (!cd || !cd->GetScalars() || !(regions = cd->GetScalars()) || regions->GetNumberOfTuples() < Ncells)
	{
		cout << "VTKLegacyReader: No cell data provided - assuming mesh is a single region (material=1)";
		boost::fill(T_m,1);
	}

	for(unsigned i=0;i<Nvertices;++i)
	{
		array<double,3> Pd;
		ug->GetPoint(i,Pd.data());
		boost::copy(Pd,P[i+m_addZeroPoint].data());
	}

	if (m_addZeroPoint)
		ug->GetPoint(0,P.front().data());

	if (m_addZeroCell)
	{
		boost::fill(T[0], 0);
		T_m[0]=0;
	}

	for(unsigned i=0;i<Ncells; ++i)
	{
		vtkIdType* p;
		vtkIdType Npc=0;

		assert(((vtkUnstructuredGrid*)ug)->GetCellType()==VTK_TETRA);
		ug->GetCellPoints(i,Npc,p);
		assert(Npc==4);

		// destination is always i'th element
		for(unsigned j=0;j<4;++j)
			T[i+m_addZeroCell][j] = p[j]+m_addZeroPoint;

		if (regions)
			T_m[i+m_addZeroCell] = regions->GetTuple1(i);
		else
			T_m[i+m_addZeroCell] = 1;
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

	cout << "  Converted to mesh" << endl;
	g->Delete();
	return M;
}

#include <utility>
#include <array>
#include <string>
#include <iostream>
#include <vtkPolyDataReader.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkCell.h>
#include <boost/range/algorithm.hpp>
using namespace std;

/** NOTE: Will prepend an additional point and/or cell if m_addZeroPoint and/or m_addZeroCell are set.
 *
 * Typically FullMonte uses the zero point and zero tetra as special dummy variables.
 * If the mesh was saved from FullMonte, they should already be in place (can set options to false).
 *
 * Coming from other sources, there typically will not be such points added so it should be set true.
 *
 */

pair<vector<array<float,3>>,vector<array<unsigned,3>>> VTKLegacyReader::readSurface(const char* fn)
{
	vtkPolyDataReader* R = vtkPolyDataReader::New();
		R->SetFileName(fn);
		R->Update();

	vtkPolyData* pd = R->GetOutput();

	cout << "Read a surface mesh with " << pd->GetNumberOfPoints() << " points and " << pd->GetNumberOfPolys() << " triangles" << endl;

	bool usePointData=true;

	if (usePointData)
	{
		vtkPointData* pdata = pd->GetPointData();
		vtkFloatArray* ca = vtkFloatArray::SafeDownCast(pdata->GetScalars());

		cout << "  Surface has " << ca->GetNumberOfTuples() << " scalars in its point data array" << endl;
	}

	vector<array<float,3>> P(pd->GetNumberOfPoints()+(m_addZeroPoint?1:0));
	vector<array<unsigned,3>> T(pd->GetNumberOfPolys()+(m_addZeroCell?1:0));

	if (m_addZeroPoint)
		P[0] = array<float,3>{0,0,0};

	if(m_addZeroCell)
		T[0] = array<unsigned,3>{0,0,0};

	for(unsigned i=0;i<pd->GetNumberOfPoints();++i)
	{
		double *p = pd->GetPoint(i);
		std::copy(p, p+3, P[i+m_addZeroPoint].data());
	}

	for(unsigned i=0;i<pd->GetNumberOfPolys();++i)
	{
		vtkCell* c = pd->GetCell(i);
		if (c->GetNumberOfPoints() != 3)
			cout << "ERROR: Expecting 3 points in a cell, received " << c->GetNumberOfPoints() << endl;
		else
			for(unsigned j=0;j<3;++j)
				T[i+m_addZeroCell][j] = c->GetPointId(j)+m_addZeroPoint;
	}

	return make_pair(P,T);
}


