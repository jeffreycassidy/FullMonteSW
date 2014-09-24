#ifdef SWIG
%module TetraMeshTCL

#include <string>

%include "typemaps.i"

// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
// Needed for VTK commands
%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }

%typemap(in) std::string& (std::string s){
	s = string(Tcl_GetString($input));
	$1 = &s;
}

//void vtkTclGetObjectFromPointer(Tcl_Interp *interp, void *temp, const char *targetType);
                           
// For vtkPolyData* returns, wrap it in a Tcl object and return the object name
%typemap(out) vtkPolyData* {
	vtkTclGetObjectFromPointer(interp,$1,"vtkPolyData");	
}

#define VTK_TYPEMAP(VTK_TYPE) %typemap(out) VTK_TYPE* { vtkTclGetObjectFromPointer(interp,$1,#VTK_TYPE); }

VTK_TYPEMAP(vtkUnstructuredGrid)
VTK_TYPEMAP(vtkPolyData)
VTK_TYPEMAP(vtkDataArray)

%{
	#include "graph.hpp"
	#include <vtk/vtkTclUtil.h>
	#include <vtkUnstructuredGrid.h>
	#include <vtkPolyData.h>
	#include "TetraMeshTCL.i"
	#include "TetraMeshBaseVTK.hpp"
	
	#include "Parallelepiped.hpp"
	
	#include <string>
	
	vtkUnstructuredGrid* getVTKTetraMesh(const TetraMeshBase& M);
	vtkPolyData* getVTKPolyData(const TriSurf& ts);
	vtkDoubleArray* getScalars(const vector<double>& V);
	
TetraMeshBase* loadMesh(PGConnection*,unsigned);
TetraMeshBase* loadMeshFile(const std::string&);
vtkDataArray* getVTKDataArray(const vector<double>& v);
vector<double> loadVectorDouble(const std::string& fn);
	
	vtkPolyData* getVTKRegion(const TetraMesh& M,const vector<unsigned>& tetIDs);
	vector<unsigned> loadVector(const std::string& fn);
%}
#endif

#include <boost/smart_ptr/shared_ptr.hpp>
#include "fm-postgres/fm-postgres.hpp"

#include "Parallelepiped.hpp"
#include "TetraMeshBase.hpp"
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <string>


TetraMeshBase 		 	loadTetraMeshBaseText(const std::string& fn);
void 					saveTetraMeshBaseText(const TetraMeshBase&,const std::string& fn);
vtkUnstructuredGrid* 	getVTKTetraMesh(const TetraMeshBase& M);

// managing database connections
extern "C" PGConnection* tclConnect();

// loading meshes and results
TetraMeshBase*	loadMesh(PGConnection*,unsigned);
TetraMeshBase* 	loadMeshFile(const std::string&);
vtkPolyData* createVTKBoundary(const TetraMesh& M,unsigned matID);

vtkPolyData* getVTKRegion(const TetraMesh& M,const vector<unsigned>& tetIDs);
vector<unsigned> loadVector(const std::string& fn);
vector<double> loadVectorDouble(const std::string& fn);
vtkDataArray* getVTKDataArray(const vector<double>& v);

TetraMesh* buildMesh(const TetraMeshBase& M);

Parallelepiped readParallelepiped(const std::string&);
TetraMeshBase clipToRegion(const TetraMeshBase& M,const Parallelepiped& pp);
vector<double> exportResultVector(PGConnection* conn,unsigned IDr,unsigned dType);