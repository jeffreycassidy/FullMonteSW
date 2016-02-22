/*
 * VTKLegacyReader.hpp
 *
 *  Created on: May 27, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_VTK_VTKLEGACYREADER_HPP_
#define STORAGE_VTK_VTKLEGACYREADER_HPP_

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <string>

class vtkUnstructuredGrid;

class VTKLegacyReader {

public:

	virtual ~VTKLegacyReader();

	class NotImplemented : public std::exception { virtual const char* what(){ return "Not supported"; } };

	virtual TetraMeshBase mesh() const;
	//virtual std::vector<Material> materials() const;
	//virtual std::vector<SourceDescription*> sources() const;

	void setFileName(const std::string fn){ meshFn_ = fn; }

	bool renumberZero() const { return renumberZero_; }
	void renumberZero(bool b){ renumberZero_=b; }

private:
	TetraMeshBase convertToMesh(vtkUnstructuredGrid* ug) const;

	bool renumberZero_=false;

	std::string meshFn_;

};



#endif /* STORAGE_VTK_VTKLEGACYREADER_HPP_ */
