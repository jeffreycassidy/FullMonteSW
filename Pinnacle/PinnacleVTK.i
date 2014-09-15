%module PinnacleVTK
%{
	#include <string>
	#include "PinnacleFile.hpp"
	#include "PinnacleCurve.hpp"
	#include "PinnacleROI.hpp"
	
	void printStr(const std::string& str);
	Pinnacle::File* readPinnacleFile(const std::string& fn);
	void makeROIPolys(Tcl_Interp* interp,const Pinnacle::File* pf,unsigned roi,const char* name);
%}

// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
// Needed for VTK commands
%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }

%typemap(in) const std::string& (std::string tempstr) {
	const char *s = Tcl_GetString($input);
	tempstr.assign(s,strlen(s));
	$1 = &tempstr;
}


//extern VTKTCL_EXPORT void
//vtkTclGetObjectFromPointer(Tcl_Interp *interp, void *temp,
//                           const char *targetType);


//extern VTKTCL_EXPORT void *
//vtkTclGetPointerFromObject(const char *name, const char *result_type,
                           //Tcl_Interp *interp, int &error);

%typemap(out) vtkPolyData* {
	const char *tclname;
	vtkTclGetObjectFromPointer(interp,$1,"vtkPolyData");
}

void printStr(const std::string& str);

Pinnacle::File* readPinnacleFile(const std::string& fn);

void makeROIPolys(Tcl_Interp* interp,const Pinnacle::File* pf,unsigned roi,const char* name);