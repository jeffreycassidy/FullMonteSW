/**
 * TetraMeshBaseVTK.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */

#ifndef VTK_TETRAMESHVTK_HPP_
#define VTK_TETRAMESHVTK_HPP_

#include <vtkObject.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>

#include "vtkFullMonteTetraMeshBaseWrapper.h"

class TetraMesh;

template<class T>class FilterBase;

class vtkCellArray;
class vtkPolyData;


void getVTKTriangleCells(const TetraMesh& M,vtkCellArray* ca,const FilterBase<int>* F=nullptr);

/** Non-owning wrapper for a FullMonte TetraMesh object.
 */

class vtkFullMonteTetraMeshWrapper : public vtkFullMonteTetraMeshBaseWrapper
{
public:
	vtkTypeMacro(vtkFullMonteTetraMeshWrapper,vtkObject);
	static vtkFullMonteTetraMeshWrapper* New();
	virtual ~vtkFullMonteTetraMeshWrapper();

	/// Get/set the underlying mesh
	void 					mesh(const TetraMesh* m);
	void					mesh(const char* mptr);
	const TetraMesh* 		mesh() 							const;

	/// Update the VTK arrays from the mesh
	void update();

	vtkPolyData*			faces() const;

	void faceFilter(FilterBase<int>* f);
	void faceFilter(const char* fptr);

protected:
	vtkFullMonteTetraMeshWrapper();

private:
	vtkCellArray*		m_faces=nullptr;
	FilterBase<int>*	m_faceFilter=nullptr;
};

#endif /* VTK_TETRAMESHVTK_HPP_ */
