#pragma once
#include "TriSurf.hpp"
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

vtkPolyData* getVTKPolyData(const TriSurf& surf);
void getVTKPolyData(const TriSurf& surf,vtkPolyData *polydata);

vtkCellArray* getVTKTriangleCells(const TriSurf& surf);
template<class InputIterator>vtkCellArray* getVTKTriangleCells(InputIterator begin,InputIterator end,unsigned long size_hint=0);
vtkPoints* getVTKPoints(const vector<Point<3,double>>& pts);
template<class InputIterator>vtkPoints* getVTKPoints(InputIterator begin,InputIterator end,unsigned long size_hint=0);
