/**
 * FullMonteVTK.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */


#ifndef FULLMONTEVTK_HPP_
#define FULLMONTEVTK_HPP_

#include <vtkProgrammableDataObjectSource.h>
#include <vtkObjectFactory.h>

#include <iostream>

class vtkFloatArray;


/** Converts from a sparse vector of floats to a vtkFloat*, suitable for colouring a rendered volume/surface
 *
 */

template<typename Index,typename Value>class SparseVector;

class SparseVectorVTK : public vtkProgrammableDataObjectSource
{
public:
	static SparseVectorVTK *New();
	vtkTypeMacro(SparseVectorVTK,vtkObject);

	////// Client-side interface
	void setInputSparseVector(const SparseVector<unsigned,float>* sv);
	void setInputSparseVector(const char* sv);
	void update();

	////// VTK interface

	void PrintSelf(std::ostream& os,vtkIndent indent);

	virtual vtkFloatArray* GetOutput();

protected:
	SparseVectorVTK();
	virtual ~SparseVectorVTK();

private:
	const SparseVector<unsigned,float>*	m_sv=nullptr;
	vtkFloatArray*						m_vtkFloat=nullptr;
};

#endif /* FULLMONTEVTK_HPP_ */
