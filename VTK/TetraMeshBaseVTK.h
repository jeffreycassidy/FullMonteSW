/**
 * FullMonteVTK.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */


#ifndef FULLMONTEVTK_HPP_
#define FULLMONTEVTK_HPP_

#ifdef SWIG

%module FullMonteVTK

%include "std_string.i"
%include "std_vector.i"

%{
#include <FullMonte/Geometry/SourceDescription.hpp>
%}

%template(sourcedescriptionvector) std::vector<SourceDescription*>;
%template(doublevector) std::vector<double>;

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

%typemap(out) std::pair<double,double>
{
	std::stringstream ss;
	ss << $1.first << ' ' << $1.second;
	Tcl_AppendResult(interp,ss.str().c_str(),NULL);
}

%typemap(out) std::array<double,3>
{
	std::array<double,3> a=$1;
	Tcl_Obj* const p[] = {
		Tcl_NewDoubleObj(a[0]),
		Tcl_NewDoubleObj(a[1]),
		Tcl_NewDoubleObj(a[2])};

	Tcl_Obj* obj = Tcl_NewListObj(3,p);
	Tcl_SetObjResult(interp,obj);
}


%typemap(in) std::array<double,3>
{
	std::array<double,3> a;
	std::stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> a[i];
	$1 = a;
}


%typemap(in) Point<3,double>
{
	Point<3,double> p;
	std::stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = p;
}

#define VTK_TYPEMAP(VTK_TYPE) %typemap(out) VTK_TYPE* { vtkTclGetObjectFromPointer(interp,$1,#VTK_TYPE); }

VTK_TYPEMAP(vtkUnstructuredGrid)
VTK_TYPEMAP(vtkPolyData)
VTK_TYPEMAP(vtkDataArray)
VTK_TYPEMAP(vtkLegendBoxActor)
VTK_TYPEMAP(vtkLookupTable)
VTK_TYPEMAP(vtkActor)
VTK_TYPEMAP(vtkScalarBarActor)
VTK_TYPEMAP(vtkPointWidget)
VTK_TYPEMAP(vtkScalarsToColors)

%{

#include "FullMonteVTK.hpp"
#include <sstream>
#include <vtkTclUtil.h>

%}

struct LegendEntry {
	std::string label;
	std::array<float,3> colour;
};

#endif


#ifndef SWIG

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkLegendBoxActor.h>
#include <vtkUnsignedShortArray.h>
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkScalarBarActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointWidget.h>
#include <vtkLookupTable.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkCallbackCommand.h>

#include <FullMonte/Geometry/TetraMeshBase.hpp>

#endif // SWIG


/** VTK interface of TetraMeshBase
 *
 */

class TetraMeshBaseVTK : public vtkObject
{
	// update the points and tetras from the base
	void setInputTetraMeshBase(const TetraMeshBase* M);

protected:

};

#endif /* FULLMONTEVTK_HPP_ */
