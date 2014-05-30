#include "PinnacleCurve.hpp"

#include <vtkPolygon.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkIdList.h>

pair<vtkSmartPointer<vtkCellArray>,vtkSmartPointer<vtkPoints>> vtkPolygonFromCurve(const Pinnacle::Curve* C)
{
	vtkSmartPointer<vtkPoints> pts = vtkPoints::New();

	vtkSmartPointer<vtkCellArray> cellarray = vtkCellArray::New();

	unsigned Np=C->getNPoints();

	if (Np < 2)
		return make_pair(vtkSmartPointer<vtkCellArray>(),vtkSmartPointer<vtkPoints>());
	else
		Np--;

	// drop a point because curve contains duplicate start/end point
	//if (!pts->SetNumberOfPoints(Np))
	pts->SetDataTypeToDouble();
	pts->SetNumberOfPoints(Np);

	// copy to array
	vtkIdType i=0;
	cellarray->InsertNextCell(Np);
	for(const array<double,3>& p : C->getPoints())
	{
		if (i < Np)
		{
			cout << "Adding point " << i << endl;
			cellarray->InsertCellPoint(i);
			pts->SetPoint(i,p[0],p[1],p[2]);
			++i;
		}
	}

	return make_pair(cellarray,pts);
}
