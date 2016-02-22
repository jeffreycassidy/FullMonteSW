/**
 * FullMonteVTK.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */


#ifndef FULLMONTEVTK_HPP_
#define FULLMONTEVTK_HPP_

#include <vtkObject.h>
#include <vtkObjectFactory.h>

#include <iostream>

class TetraMesh;
class vtkPoints;
class vtkCellArray;
class vtkUnsignedShortArray;
class vtkUnstructuredGrid;

class TetraMeshVTK : public vtkObject
{
public:
	static TetraMeshVTK *New();
	vtkTypeMacro(TetraMeshVTK,vtkObject);

	////// Client-side interface
	void setInputTetraMesh(const TetraMesh* M);
	void setInputTetraMesh(const char* pStr);
	void update();

	////// VTK interface

	void PrintSelf(std::ostream& os,vtkIndent indent);

	vtkUnstructuredGrid* getBlankMesh() const;
	vtkUnstructuredGrid* getRegions() const;

protected:
	TetraMeshVTK();
	virtual ~TetraMeshVTK();

private:
	// The source TetraMesh
	const TetraMesh* 		m_M=nullptr;

	// cached VTK representations of points, tetras, and tetra regions
	vtkPoints*				m_vtkP=nullptr;
	vtkCellArray*			m_vtkT=nullptr;
	vtkUnsignedShortArray*	m_vtkRegions=nullptr;

	friend class TetraMeshRegionsVTK;
};

#endif /* FULLMONTEVTK_HPP_ */
