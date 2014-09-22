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

%{
	#include "graph.hpp"
	#include <vtk/vtkTclUtil.h>
	#include <vtkUnstructuredGrid.h>
	#include <vtkPolyData.h>
	#include "TetraMeshTCL.i"
	#include "TetraMeshBaseVTK.hpp"
	
	#include <string>
	
	vtkUnstructuredGrid* getVTKTetraMesh(const TetraMeshBase& M);
	vtkPolyData* getVTKPolyData(const TriSurf& ts);
	//TetraMesh* loadMesh(PGConnection*,unsigned);
	TetraMesh* loadMesh(const std::string&);
	
	vtkPolyData* getVTKRegion(const TetraMesh& M,const vector<unsigned>& tetIDs);
	vector<unsigned> loadVector(const std::string& fn);
%}
#endif

#include <boost/smart_ptr/shared_ptr.hpp>
#include "fm-postgres/fm-postgres.hpp"

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
//TetraMesh* loadMesh(PGConnection*,unsigned);
TetraMesh* loadMesh(const std::string&);
//TetraMesh* loadMesh(const char*);
vtkPolyData* createVTKBoundary(const TetraMesh& M,unsigned matID);

vtkPolyData* getVTKRegion(const TetraMesh& M,const vector<unsigned>& tetIDs);
vector<unsigned> loadVector(const std::string& fn);