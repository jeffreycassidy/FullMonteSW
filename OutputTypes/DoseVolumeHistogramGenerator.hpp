/*
 * DoseVolumeHistogram.hpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_DOSEVolumeHISTOGRAMGENERATOR_HPP_
#define OUTPUTTYPES_DOSEVolumeHISTOGRAMGENERATOR_HPP_

class TetraMesh;
template<typename T>class FilterBase;

#include "FluenceMapBase.hpp"


#include "EmpiricalCDF.hpp"

class TriFilterRegionBounds;

class DoseVolumeHistogramGenerator
{
public:
	DoseVolumeHistogramGenerator();

	/// Get/set the associated mesh
	void 						mesh(const TetraMesh* M);
	const TetraMesh* 			mesh() 						const;

	/// Filter determining which tetra elements are included
	void 						filter(FilterBase<unsigned>* F);
	const FilterBase<unsigned>*	filter()					const;

	void						fluence(const VolumeFluenceMap* phi);
	const VolumeFluenceMap*	fluence() 								const;

	/// EmpiricalCDF with float value (dose), float weight (area), and standard less-than comparison
	EmpiricalCDF<float,float>	result() const;

private:
	const VolumeFluenceMap* 	m_phi=nullptr;
	const TetraMesh*			m_mesh=nullptr;
	const FilterBase<unsigned>*	m_filter=nullptr;
};



#endif /* OUTPUTTYPES_DOSEVolumeHISTOGRAMGENERATOR_HPP_ */
