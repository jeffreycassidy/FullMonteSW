/*
 * TetraMeshRegionsVTK.hpp
 *
 *  Created on: Nov 7, 2015
 *      Author: jcassidy
 */

#ifndef VTK_TETRAMESHREGIONSVTK_HPP_
#define VTK_TETRAMESHREGIONSVTK_HPP_

#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkObjectFactory.h>

class vtkDataObject;
class vtkInformation;
class vtkInformationVector;

class TetraMeshRegionsVTK : public vtkUnstructuredGridAlgorithm
{

public:

	vtkTypeMacro(TetraMeshRegionsVTK,vtkUnstructuredGridAlgorithm);

	static TetraMeshRegionsVTK* New();

	void PrintSelf(std::ostream& os,vtkIndent indent);

	virtual int RequestData(vtkInformation* request, vtkInformationVector** inputVector,vtkInformationVector* outputVector) override;

protected:
	TetraMeshRegionsVTK();
	virtual ~TetraMeshRegionsVTK();

private:

};




#endif /* VTK_TETRAMESHREGIONSVTK_HPP_ */
