%module FullMonteVTKTCL

%{
	#include "TetraMeshVTK.hpp"
	#include "TetraMeshRegionsVTK.hpp"
	#include <vtkTclUtil.h>
%}


// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
// Needed for VTK commands
%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }


#define VTK_TYPEMAP(VTK_TYPE) %typemap(out) VTK_TYPE* { vtkTclGetObjectFromPointer(interp,$1,#VTK_TYPE); }

VTK_TYPEMAP(TetraMeshVTK)
VTK_TYPEMAP(TetraMeshRegionsVTK)

%include "TetraMeshVTK.hpp"
%include "TetraMeshRegionsVTK.hpp"
