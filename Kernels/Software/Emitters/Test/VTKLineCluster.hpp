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

/** Creates a point cloud for testing 3D statistical distributions
 * Typical usage: illustrating rays emerging from a source
 **/

class VTKLineCluster
{
public:
	VTKLineCluster();
	void write(const std::string& fn);
	void clear();

	void add(std::array<float,3> p,std::array<float,3> d);
	void add(std::array<double,3> p,std::array<double,3> d);


private:
	vtkPoints*		m_vtkP=nullptr;
	vtkFloatArray*	m_vtkD=nullptr;
};

#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_VTKPOINTCLOUD_HPP_ */
