/*
 * VTKPointCloud.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_VTKPOINTCLOUD_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_VTKPOINTCLOUD_HPP_

class vtkPoints;
class vtkPolyData;

#include <string>
#include <array>

/** Creates a point cloud for testing 3D statistical distributions
 */

class VTKPointCloud
{
public:
	VTKPointCloud();
	void write(const std::string& fn);		///< Write points to a .vtk polydata file as verts
	void clear();

	void add(std::array<float,3> p);		///< Insert a point
	void add(std::array<double,3> p);		///< Insert a point


private:
	vtkPoints*		m_vtkP=nullptr;
};



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_VTKPOINTCLOUD_HPP_ */
