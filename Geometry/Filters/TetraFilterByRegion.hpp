/*
 * TetraFilterByRegion.hpp
 *
 *  Created on: Feb 25, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_FILTERS_TETRAFILTERBYREGION_HPP_
#define GEOMETRY_FILTERS_TETRAFILTERBYREGION_HPP_

#include <FullMonteHW/Geometry/Filters/TetraMeshBaseFilter.hpp>
#include "FilterBase.hpp"
#include <vector>

class TetraFilterByRegion : public FilterBase<unsigned>, public TetraMeshBaseFilter
{
public:
	/// Default-construct including all materials
	explicit TetraFilterByRegion(const TetraMeshBase* m=nullptr);

	void include(unsigned region,bool incl);	///< Set inclusion of region
	bool include(unsigned region) const;		///< Query inclusion of region

	void includeAll();
	void excludeAll();

	virtual bool operator()(const unsigned x) const override;

private:
	virtual void postMeshUpdate() override;
	std::vector<bool> 	m_includeFlags;
};




#endif /* GEOMETRY_FILTERS_TETRAFILTERBYREGION_HPP_ */
