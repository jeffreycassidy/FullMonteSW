/*
 * VTKLegacyReader.hpp
 *
 *  Created on: May 27, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_VTK_VTKLEGACYWRITER_HPP_
#define STORAGE_VTK_VTKLEGACYWRITER_HPP_

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <string>

#include <vtkUnstructuredGrid.h>

/** Write a TetraMesh
 *
 */

class VTKLegacyWriter
{
public:
	class NotImplemented : public std::exception { virtual const char* what(){ return "Not supported"; } };

	//virtual TetraMeshBase mesh() const;
	//virtual std::vector<Material> materials() const;
	//virtual std::vector<SourceDescription*> sources() const;

	void setFileName(const std::string fn){ meshFn_ = fn; }

	void writeMeshRegions(std::string fn,const TetraMeshBase&) const;

private:
	vtkUnstructuredGrid* convertFromMesh(const TetraMeshBase& ug) const;

	std::string meshFn_;

};



#endif /* STORAGE_VTK_VTKLEGACYWRITER_HPP_ */
