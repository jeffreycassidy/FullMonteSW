/*
 * VTKLineCluster.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_VTKLINECLUSTER_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_VTKLINECLUSTER_HPP_

class vtkPoints;
class vtkPolyData;
class vtkFloatArray;

#include <string>
#include <array>

/** Creates a set of rays as VTK polydata vertices at the points with normals pointing in the direction of the ray.
 */

class VTKLineCluster
{
public:
	VTKLineCluster();
	void write(const std::string& fn);							///< Write to vtk polydata as lines
	void clear();

	void add(std::array<float,3> p,std::array<float,3> d);		///< Insert ray (position,direction) into the cluster
	void add(std::array<double,3> p,std::array<double,3> d);


private:
	vtkPoints*		m_vtkP=nullptr;
	vtkFloatArray*	m_vtkD=nullptr;
};

#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_VTKPOINTCLOUD_HPP_ */
