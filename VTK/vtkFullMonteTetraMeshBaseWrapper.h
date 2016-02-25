/**
 * TetraMeshBaseVTK.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */

#ifndef VTK_TETRAMESHBASEVTK_HPP_
#define VTK_TETRAMESHBASEVTK_HPP_

#include <vtkObject.h>
#include <vtkIdTypeArray.h>

class TetraMeshBase;

class vtkPoints;
class vtkCellArray;
class vtkUnstructuredGrid;
class vtkUnsignedShortArray;

void getVTKPoints(const TetraMeshBase& M,vtkPoints* P);
void getVTKTetraCells(const TetraMeshBase& M,vtkCellArray* ca);
void getVTKTetraRegions(const TetraMeshBase& M,vtkUnsignedShortArray* R);


/** Non-owning wrapper for a FullMonte TetraMeshBase object.
 */

class vtkFullMonteTetraMeshBaseWrapper : public vtkObject
{
public:
	static vtkFullMonteTetraMeshBaseWrapper* New();
	virtual ~vtkFullMonteTetraMeshBaseWrapper();

	/// Get/set the underlying mesh
	void 					mesh(const TetraMeshBase* m);
	const TetraMeshBase* 	mesh() 							const;

	/// Update the VTK arrays from the mesh
	void update();

	vtkUnstructuredGrid* blankMesh() const;
	vtkUnstructuredGrid* regionMesh() const;

	vtkPoints*				points() const;
	vtkUnsignedShortArray* regions() const;

protected:
	vtkFullMonteTetraMeshBaseWrapper();

	vtkCellArray*			tetras() const;

private:
	const TetraMeshBase* 	m_mesh=nullptr;

	vtkPoints*				m_points=nullptr;
	vtkUnsignedShortArray*	m_regions=nullptr;
	vtkCellArray*			m_tetras=nullptr;
};

//// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
//// Needed for VTK commands
//%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }

//%typemap(out) std::array<double,3>
//{
//	std::array<double,3> a=$1;
//	Tcl_Obj* const p[] = {
//		Tcl_NewDoubleObj(a[0]),
//		Tcl_NewDoubleObj(a[1]),
//		Tcl_NewDoubleObj(a[2])};
//
//	Tcl_Obj* obj = Tcl_NewListObj(3,p);
//	Tcl_SetObjResult(interp,obj);
//}

//#define VTK_TYPEMAP(VTK_TYPE) %typemap(out) VTK_TYPE* { vtkTclGetObjectFromPointer(interp,$1,#VTK_TYPE); }
//
//VTK_TYPEMAP(vtkUnstructuredGrid)
//#include <vtkTclUtil.h>
#endif /* VTK_TETRAMESHBASEVTK_HPP_ */
