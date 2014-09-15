#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkIdTypeArray.h>
#include <vtkCellArray.h>
#include "TriSurf.hpp"
#include "VTKInterface.hpp"

vtkPolyData* getVTKPolyData(const TriSurf& surf)
{
	vtkPolyData* pd = vtkPolyData::New();
	getVTKPolyData(surf,pd);
	return pd;
}

void getVTKPolyData(const TriSurf& surf,vtkPolyData *vtkpoly)
{
	// Cells
	vtkCellArray *vtkcells = getVTKTriangleCells(surf.F.begin(),surf.F.end(),surf.F.size());
	vtkpoly->SetPolys(vtkcells);

	// Points
	vtkPoints* vtkpoints=getVTKPoints(surf.P);
	vtkpoly->SetPoints(vtkpoints);
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



/**
 * Requirements:
 * InputIterator must dereference to something which has a data() member giving a float* or double*
 *
 * If size_hint is given, the size will be preallocated (faster), otherwise inserted as needed
 * NOTE: size_hint must be exact!! no range checking performed
 */

template<class InputIterator>vtkPoints* getVTKPoints(InputIterator begin,InputIterator end,unsigned long size_hint=0)
{
	vtkPoints* vtkp = vtkPoints::New();

	if (size_hint == 0)
		for(; begin != end; ++begin)
			vtkp->InsertNextPoint(begin->data());
	else
	{
		vtkp->SetNumberOfPoints(size_hint);
		for(auto el : make_pair(begin,end) | boost::adaptors::indexed(0))
			vtkp->SetPoint(el.index(), el.value().data());
	}
	return vtkp;
}
