#pragma once

#include "TetraMeshBase.hpp"
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkUnstructuredGrid.h>

vtkUnstructuredGrid* 	getVTKTetraMesh(const TetraMeshBase& M);
vtkPoints* 				getVTKPoints(const TetraMeshBase& M);
vtkCellArray* 			getVTKTetras(const TetraMeshBase& M);
