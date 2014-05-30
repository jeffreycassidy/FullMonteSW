#include "PinnacleCurve.hpp"

#include <vtkPolygon.h>
#include <vtkSmartPointer.h>

pair<vtkSmartPointer<vtkCellArray>,vtkSmartPointer<vtkPoints>> vtkPolygonFromCurve(const Pinnacle::Curve* C);
