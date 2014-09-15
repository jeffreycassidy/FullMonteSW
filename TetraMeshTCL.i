#ifdef SWIG
%module TetraMeshTCL

%include "typemaps.i"

// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
// Needed for VTK commands
%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }

//void vtkTclGetObjectFromPointer(Tcl_Interp *interp, void *temp, const char *targetType);
                           
// For vtkPolyData* returns, wrap it in a Tcl object and return the object name
%typemap(out) vtkPolyData* {
	vtkTclGetObjectFromPointer(interp,$1,"vtkPolyData");	
}

%typemap(out) vtkUnstructuredGrid* {
	vtkTclGetObjectFromPointer(interp,$1,"vtkUnstructuredGrid");	
}


%{

	#include "newgeom.hpp"
	#include "graph.hpp"
	#include "fm-postgres/fm-postgres.hpp"
	#include "VTKInterface.hpp"

	#include <vtk/vtkTclUtil.h>
	
	class TriSurf;
	class vtkPolyData;
	
	#include "TetraMeshTCL.i"
	

%}
#endif

#include "TriSurf.hpp"
#include "graph.hpp"
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnstructuredGrid.h>
#include <tcl.h>



// managing database connections
extern "C" PGConnection* tclConnect();

// loading meshes and results
extern "C" TetraMesh* loadMesh(PGConnection*,unsigned);

vtkPolyData* getVTKPolyData(const TriSurf& surf);
vtkUnstructuredGrid* getVTKTetraData(const TetraMesh& M);