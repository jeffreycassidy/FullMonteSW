#ifdef SWIG
%module TetraMeshTCL

#include <string>

%include "typemaps.i"

// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
// Needed for VTK commands
%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }

%typemap(in) const std::string& (std::string s){
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
	
	vtkUnstructuredGrid* getVTKTetraMesh(const TetraMeshBase& M);
%}
#endif

#include "TetraMeshBase.hpp"
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>


TetraMeshBase 		 	loadTetraMeshBaseText(const std::string& fn);
void 					saveTetraMeshBaseText(const TetraMeshBase&,const std::string& fn);
vtkUnstructuredGrid* 	getVTKTetraMesh(const TetraMeshBase& M);

// managing database connections
//extern "C" PGConnection* tclConnect();

// loading meshes and results
//extern "C" TetraMesh* loadMesh(PGConnection*,unsigned);

//vtkPolyData* getVTKPolyData(const TriSurf& surf);
//vtkUnstructuredGrid* getVTKTetraData(const TetraMesh& M);