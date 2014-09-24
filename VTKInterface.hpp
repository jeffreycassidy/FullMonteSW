#pragma once
#include "TriSurf.hpp"
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

#include <boost/range/adaptor/indexed.hpp>

vtkPolyData* getVTKPolyData(const TriSurf& surf);
void getVTKPolyData(const TriSurf& surf,vtkPolyData *polydata);

vtkCellArray* getVTKTriangleCells(const TriSurf& surf);
template<class InputIterator>vtkCellArray* getVTKTriangleCells(InputIterator begin,InputIterator end,unsigned long size_hint=0);
vtkPoints* getVTKPoints(const vector<Point<3,double>>& pts);


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

template<class ArrayType,class InputIterator>ArrayType* getVTKScalarArray(InputIterator begin,InputIterator end,unsigned long size_hint=0)
{
	ArrayType* vtka = ArrayType::New();

	vtka->SetNumberOfComponents(1);

	if(size_hint==0)
		for(; begin != end; ++begin)
			vtka->InsertNextTuple1(*begin);
	else {
		vtka->SetNumberOfTuples(size_hint);
		for(auto el : make_pair(begin,end) | boost::adaptors::indexed(0))
			vtka->SetTuple1(el.index(),el.value());
	}
	return vtka;
}
