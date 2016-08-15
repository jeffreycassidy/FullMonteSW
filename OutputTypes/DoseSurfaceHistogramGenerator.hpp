/*
 * DoseSurfaceHistogram.hpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_DOSESURFACEHISTOGRAMGENERATOR_HPP_
#define OUTPUTTYPES_DOSESURFACEHISTOGRAMGENERATOR_HPP_

class TetraMesh;
template<typename T>class FilterBase;

#include "FluenceMapBase.hpp"


#include "EmpiricalCDF.hpp"

class TriFilterRegionBounds;

class DoseSurfaceHistogramGenerator
{
public:
	DoseSurfaceHistogramGenerator();

	/// Get/set the associated mesh
	void 						mesh(const TetraMesh* M);
	const TetraMesh* 			mesh() 						const;

	/// Filter determining which face elements are included
	void 						filter(FilterBase<int>* F);
	const FilterBase<int>*		filter()					const;

	void						fluence(const SurfaceFluenceMap* phi);
	const SurfaceFluenceMap*	fluence() 								const;

	/// EmpiricalCDF with float value (dose), float weight (area), and standard less-than comparison
	EmpiricalCDF<float,float>	result() const;

private:
	const SurfaceFluenceMap* 	m_phi=nullptr;
	const TetraMesh*			m_mesh=nullptr;
	const FilterBase<int>*		m_filter=nullptr;
};



#endif /* OUTPUTTYPES_DOSESURFACEHISTOGRAMGENERATOR_HPP_ */
