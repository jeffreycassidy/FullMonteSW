#include "PinnacleCurve.hpp"
#include "PinnacleROI.hpp"
#include "PinnacleFile.hpp"

#include "PinnacleGraph.hpp"

#include <vtkPolygon.h>
#include <vtkCellArray.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>

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




#include <vtk/vtkTclUtil.h>

// Support code that has some constants needed for creation of VTK TCL references

template<class vtkObjectType>struct vtkObjectTraits;

#define VTK_OBJECT_TCL_WRAPPER(VTK_OBJECT_TYPE) \
	template<>struct vtkObjectTraits<VTK_OBJECT_TYPE>{ \
		static constexpr const char *typestr=#VTK_OBJECT_TYPE;\
		static vtkTclCommandStruct cs; \
	}; \
	ClientData VTK_OBJECT_TYPE##NewCommand(); \
	int VTK_OBJECT_TYPE##Command(ClientData cd, Tcl_Interp *interp,int argc,char **argv); \
	vtkTclCommandStruct vtkObjectTraits<VTK_OBJECT_TYPE>::cs = { VTK_OBJECT_TYPE##NewCommand, VTK_OBJECT_TYPE##Command };

//VTK_OBJECT_TCL_WRAPPER(vtkActor)
VTK_OBJECT_TCL_WRAPPER(vtkPolyData)

// not part of the exposed API - need to use with care
// This depends on VTK 6.1.0
vtkTclInterpStruct *vtkGetInterpStruct(Tcl_Interp *interp);

template<class vtkObjectType>vtkObjectType* createVTKTCLObject(Tcl_Interp* interp,const char* name)
{
	char *argv[2] = { (char*)vtkObjectTraits<vtkObjectType>::typestr, (char*)name };
	vtkTclNewInstanceCommand((ClientData*)&vtkObjectTraits<vtkObjectType>::cs,interp,2,argv);

	// look up the resulting object
	vtkTclInterpStruct *is = vtkGetInterpStruct(interp);

	if (Tcl_HashEntry *entry = Tcl_FindHashEntry(&is->InstanceLookup,name))
	{
		return static_cast<vtkObjectType*>(Tcl_GetHashValue(entry));
	}
	else {
		cerr << "Error creating object! " << endl;
		return NULL;
	}
}


void vtkROIAsPolygons(const Pinnacle::ROI& roi,vtkPolyData* vtkpoly)
{
	vtkPoints* vtkp = vtkPoints::New();
	vtkIdTypeArray *vtkid = vtkIdTypeArray::New();
	vtkCellArray* vtkcells = vtkCellArray::New();

	unsigned Npoints=0,Ncells=0;
	for(const Pinnacle::Curve& c : roi.getCurves())
	{
		const vector<array<double,3>>& pts = c.getPoints();
		vtkid->InsertNextTuple1(pts.size());
		++Ncells;
		for(const array<double,3>& p : pts)
		{
			vtkp->InsertNextPoint(p.data());
			vtkid->InsertNextTuple1(Npoints++);
		}
	}

	vtkcells->SetCells(Ncells,vtkid);

	vtkpoly->SetPoints(vtkp);
	vtkpoly->SetPolys(vtkcells);
}


void makeROIPolys(Tcl_Interp* interp,const Pinnacle::File* pf,unsigned roi,const char* name)
{
	vtkPolyData* vtkpoly = createVTKTCLObject<vtkPolyData>(interp,name);
	vtkROIAsPolygons(pf->getROI(roi),vtkpoly);

}


void printStr(const string& str_)
{
	cout << "Printing string: " << str_ << endl;
}


Pinnacle::File* readPinnacleFile(const string& fn)
{
	Pinnacle::File* pf = new Pinnacle::File(fn);
	pf->read();
	return pf;
}
