/*
 * DoseSurfaceHistogram.hpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_DOSESURFACEHISTOGRAMGENERATOR_HPP_
#define OUTPUTTYPES_DOSESURFACEHISTOGRAMGENERATOR_HPP_

class TetraMesh;
class OutputData;
template<typename T>class FilterBase;
template<typename T>class SpatialMap;
#include <FullMonteSW/Geometry/Filters/TriFilterRegionBounds.hpp>

class DoseHistogramGenerator
{
public:
	DoseHistogramGenerator();

	/// Get/set the associated mesh
	void 						mesh(const TetraMesh* M);
	const TetraMesh* 			mesh() 						const;

	/// Filter determining which face elements are included
	void						filter(TriFilterRegionBounds* F){ filter(static_cast<FilterBase<int>*>(F)); }
	void 						filter(FilterBase<int>* F);
	const FilterBase<int>*		filter()					const;

	void 						dose(const OutputData* D);
	const SpatialMap<float>*	dose() 								const;

	/// EmpiricalCDF with float value (dose), float weight (area), and standard less-than comparison
	OutputData*					result() const;

private:
	const SpatialMap<float>* 	m_phi=nullptr;
	const TetraMesh*			m_mesh=nullptr;
	const FilterBase<int>*		m_filter=nullptr;
};

#endif /* OUTPUTTYPES_DOSESURFACEHISTOGRAMGENERATOR_HPP_ */
