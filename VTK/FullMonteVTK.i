%module FullMonteVTK

//%include "typemaps.i"

%include "std_string.i"


// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
// Needed for VTK commands
%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }

%typemap(in) std::array<float,2>
{
	std::array<float,2> a;
	std::stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<2;++i)
		ss >> a[i];
	$1 = a;
}
              
// For vtkPolyData* returns, wrap it in a Tcl object and return the object name
//%typemap(out) vtkPolyData* {
//	vtkTclGetObjectFromPointer(interp,$1,"vtkPolyData");	
//}

#define VTK_TYPEMAP(VTK_TYPE) %typemap(out) VTK_TYPE* { vtkTclGetObjectFromPointer(interp,$1,#VTK_TYPE); }

VTK_TYPEMAP(vtkUnstructuredGrid)
VTK_TYPEMAP(vtkPolyData)
VTK_TYPEMAP(vtkDataArray)
VTK_TYPEMAP(vtkLegendBoxActor)
VTK_TYPEMAP(vtkLookupTable)

%{

#include "FullMonteVTK.hpp"
#include <sstream>
#include <vtk/vtkTclUtil.h>

%}


class VTKMeshRep {
public:
	VTKMeshRep(const TetraMesh* M);
	~VTKMeshRep();

	vtkUnstructuredGrid*	getMeshWithRegions() const;
	vtkPolyData*			getSubsetFaces(const std::vector<unsigned>& idx) const;
	vtkUnstructuredGrid*	getSubsetMesh(const std::vector<unsigned>& idx) const;

	// deal with legend
	void addLegendEntry(const LegendEntry& le);
	
	vtkLegendBoxActor*		getLegendActor(
			const std::array<float,2> ll,
			const std::array<float,2> ur) const;
	void setLegend(const std::vector<LegendEntry>& legend){ legend_=legend; }
	
	vtkLookupTable* getRegionMapLUT() const;
};

struct LegendEntry {
	std::string label;
	std::array<float,3> colour;
};

