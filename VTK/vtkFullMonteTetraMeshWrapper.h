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

#include "vtkFullMonteTetraMeshBaseWrapper.h"

class TetraMesh;

class vtkCellArray;
class vtkPolyData;

void getVTKTriangleCells(const TetraMesh& M,vtkCellArray* ca);

/** Non-owning wrapper for a FullMonte TetraMesh object.
 */

class vtkFullMonteTetraMeshWrapper : public vtkFullMonteTetraMeshBaseWrapper
{
public:
	static vtkFullMonteTetraMeshWrapper* New();
	virtual ~vtkFullMonteTetraMeshWrapper();

	/// Get/set the underlying mesh
	void 					mesh(const TetraMesh* m);
	const TetraMesh* 		mesh() 							const;

	/// Update the VTK arrays from the mesh
	void update();

	vtkPolyData*			faces() const;

protected:
	vtkFullMonteTetraMeshWrapper();

private:
	vtkCellArray*		m_faces=nullptr;
};

#endif /* VTK_TETRAMESHVTK_HPP_ */
