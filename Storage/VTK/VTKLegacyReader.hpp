/*
 * VTKLegacyReader.hpp
 *
 *  Created on: May 27, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_VTK_VTKLEGACYREADER_HPP_
#define STORAGE_VTK_VTKLEGACYREADER_HPP_

#include <FullMonte/Geometry/TetraMesh.hpp>

#include <array>
#include <vector>
#include <utility>

class vtkUnstructuredGrid;

/** Class for loading .vtk files (ASCII or binary)
 *
 * NOTE: Use caution when loading ASCII files, as they have finite precision that may behave badly in some
 * situations, eg. causing ill-formed tetras. Binary files are preferred when storing generated/refined meshes
 * for simulation.
 */

class VTKLegacyReader
{

public:

	virtual ~VTKLegacyReader();

	class NotImplemented : public std::exception {
		virtual const char* what() const noexcept override { return "Not supported"; }
	};

	virtual TetraMeshBase mesh() const;
	//virtual std::vector<Material> materials() const;
	//virtual std::vector<SourceDescription*> sources() const;

	void setFileName(const std::string fn){ meshFn_ = fn; }

	/// Control whether a dummy point/tetra is added at the zero position or not
	/// (convention is that point and tetra zero are special "not-a-point" or
	/// "not-a-tetra" values

	void addZeroPoint(bool z)	{ m_addZeroPoint=z; }
	void addZeroCell(bool z)	{ m_addZeroCell=z;	}
	void renumberZero(bool z)	{ m_addZeroCell=m_addZeroPoint=z; }

	/// Load a surface, returning the points and triangles
	std::pair<
		std::vector<std::array<float,3>>,
		std::vector<std::array<unsigned,3>>> readSurface(const char*);

private:
	TetraMeshBase convertToMesh(vtkUnstructuredGrid* ug) const;

	bool m_addZeroPoint=true;
	bool m_addZeroCell=true;

	std::string meshFn_;
};

#endif /* STORAGE_VTK_VTKLEGACYREADER_HPP_ */
