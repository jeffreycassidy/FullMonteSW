#ifdef SWIG
%module TetraMeshTCL

%include "typemaps.i"

// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
// Needed for VTK commands
%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }

%{

	#include "newgeom.hpp"
	#include "graph.hpp"
	#include "fm-postgres/fm-postgres.hpp"
	#include "VTKInterface.hpp"
	
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
#include <tcl.h>



// managing database connections
extern "C" PGConnection* tclConnect();

// loading meshes and results
extern "C" TetraMesh* loadMesh(PGConnection*,unsigned);

extern "C" TriSurf* extractBoundary(const TetraMesh*,unsigned);

void createVTKBoundary(Tcl_Interp* interp,const char *name,const TetraMesh* M, unsigned IDmat);