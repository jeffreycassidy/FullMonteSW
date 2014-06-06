#include "PinnacleCurve.hpp"

#include <vtkPolygon.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

pair<vtkSmartPointer<vtkCellArray>,vtkSmartPointer<vtkPoints>> vtkPolygonFromCurve(const Pinnacle::Curve* C);
vtkSmartPointer<vtkPolyData> vtkRibbonFromCurves(const Pinnacle::File& f);
