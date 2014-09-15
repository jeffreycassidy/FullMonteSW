#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkIdTypeArray.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedShortArray.h>
#include "TriSurf.hpp"
#include "VTKInterface.hpp"

#include "graph.hpp"


vtkPolyData* getVTKPolyData(const TriSurf& surf)
{
	vtkPolyData* pd = vtkPolyData::New();

	// Cells
	vtkCellArray *vtkcells = getVTKTriangleCells(surf.F.begin(),surf.F.end(),surf.F.size());
	pd->SetPolys(vtkcells);

	// Points
	vtkPoints* vtkpoints=getVTKPoints(surf.P);
	pd->SetPoints(vtkpoints);

	return pd;
}

template<class InputIterator>vtkCellArray* getVTKTriangleCells(InputIterator begin,InputIterator end,unsigned long size_hint=0)
{
	vtkCellArray *vtkca = vtkCellArray::New();
	if (!size_hint)
		for(; begin != end; ++begin)
		{
			array<vtkIdType,3> tmp;
			copy(begin->begin(),begin->end(),tmp.begin());
			vtkca->InsertNextCell(3,tmp.data());
		}
	else
	{
		vtkIdTypeArray *cells = vtkIdTypeArray::New();
		cells->SetNumberOfComponents(1);
		cells->SetNumberOfTuples(4*size_hint);
		for(auto el : make_pair(begin,end) | boost::adaptors::indexed(0))
		{
			cells->SetTuple1(el.index()<<2,3);
			cells->SetTuple1((el.index()<<2)+1,el.value()[0]);
			cells->SetTuple1((el.index()<<2)+2,el.value()[1]);
			cells->SetTuple1((el.index()<<2)+3,el.value()[2]);
		}
		vtkca->SetCells(size_hint,cells);
	}
	return vtkca;
}


template<class InputIterator>vtkCellArray* getVTKTetraCells(InputIterator begin,InputIterator end,unsigned long size_hint=0)
{
	vtkCellArray *vtkca = vtkCellArray::New();
	if(!size_hint)
		for(;begin != end; ++begin)
		{
			array<vtkIdType,4> tmp;
			copy(begin->begin(),begin->end(),tmp.begin());
			vtkca->InsertNextCell(4,tmp.data());
		}
	else
	{
		vtkIdTypeArray *cells = vtkIdTypeArray::New();
		cells->SetNumberOfComponents(1);
		cells->SetNumberOfTuples(5*size_hint);
		for(auto el : make_pair(begin,end) | boost::adaptors::indexed(0))
		{
			cells->SetTuple1(el.index()*5,4);
			cells->SetTuple1(el.index()*5+1,el.value()[0]);
			cells->SetTuple1(el.index()*5+2,el.value()[1]);
			cells->SetTuple1(el.index()*5+3,el.value()[2]);
			cells->SetTuple1(el.index()*5+4,el.value()[3]);
		}
		vtkca->SetCells(size_hint,cells);
	}
	return vtkca;
}

vtkCellArray* getVTKTetraCells(const vector<TetraByPointID>& v)
{
	return getVTKTetraCells(v.begin(),v.end(),v.size());
}


vtkUnstructuredGrid* getVTKTetraData(const TetraMesh& M)
{
	vtkUnstructuredGrid* ds = vtkUnstructuredGrid::New();

	// Cells
	vtkCellArray *vtkcells = getVTKTetraCells(M.tetraIDBegin(),M.tetraIDEnd(),M.getNt());
	ds->SetCells(VTK_TETRA,vtkcells);

	// Points
	vtkPoints* vtkpoints = getVTKPoints(M.pointBegin()-1,M.pointEnd(),M.getNp()+1);
	ds->SetPoints(vtkpoints);

	// Data
	vtkUnsignedShortArray* vtkregions = vtkUnsignedShortArray::New();
	vtkregions->SetNumberOfComponents(1);
	vtkregions->SetNumberOfTuples(M.getNt());

	for(unsigned i=0;i<M.getNt();++i)
		vtkregions->SetTuple1(i,M.getMaterial(i+1));

	ds->GetCellData()->SetScalars(vtkregions);

	return ds;
}



vtkPoints* getVTKPoints(const vector<Point<3,double>>& pts)
{
	/*vtkPoints* vtkp = vtkPoints::New();

	vtkp->SetNumberOfPoints((long long int)pts.size());

	for(auto el : pts | boost
			::adaptors::indexed(0))
		vtkp->SetPoint(el.index(), el.value().data());

	return vtkp;*/
	return getVTKPoints(pts.begin(),pts.end(),pts.size());
}


