#include "PinnacleCurve.hpp"
#include "PinnacleROI.hpp"
#include "PinnacleFile.hpp"

#include "PinnacleGraph.hpp"

#include <vtkPolygon.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkIdList.h>

#include <vector>
#include <algorithm>

using namespace std;

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



class PointRecord {
public:
	unsigned sliceID;
	unsigned roiID;
	unsigned curveID;
	PointGraph::vertex_descriptor pointID;
	array<double,3> coords;

	PointRecord(unsigned sliceID_=0,unsigned roiID_=0,unsigned curveID_=0,PointGraph::vertex_descriptor pointID_=0,const array<double,3>& coords_=array<double,3>{0.0,0.0,0.0}) :
		sliceID(sliceID_),roiID(roiID_),curveID(curveID_),pointID(pointID_),coords(coords_){}

	static PointRecord begin(unsigned sliceID_=0,unsigned roiID_=0){   return PointRecord(sliceID_,roiID_,0); }
	static PointRecord   end(unsigned sliceID_=-1,unsigned roiID_=-1){ return PointRecord(sliceID_,roiID_,-1); }

	static bool OrderBySlice(const PointRecord& a,const PointRecord& b){
		if (a.sliceID < b.sliceID)
			return true;
		if (a.sliceID > b.sliceID)
			return false;

		if (a.roiID < b.roiID)
			return true;
		if (a.roiID > b.roiID)
			return false;

		if (a.curveID < b.curveID)
			return true;
		if (a.curveID > b.curveID)
			return false;

		return a.pointID < b.pointID;
	}


	static bool OrderByROI(const PointRecord& a,const PointRecord& b){
		if (a.roiID < b.roiID)
			return true;
		if (a.roiID > b.roiID)
			return false;

		if (a.sliceID < b.sliceID)
			return true;
		if (a.sliceID > b.sliceID)
			return false;

		if (a.curveID < b.curveID)
			return true;
		if (a.curveID > b.curveID)
			return false;

		return a.pointID < b.pointID;
	}
};

ostream& operator<<(ostream& os,const PointRecord& p)
{
	os << "ROI " << p.roiID << "  slice " << p.sliceID << "  curve " << p.curveID << "  point " << p.pointID << "  coords " << p.coords;
	return os;
}


