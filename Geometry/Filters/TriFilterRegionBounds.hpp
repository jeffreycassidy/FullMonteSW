/*
 * TriFilterRegionBounds.hpp
 *
 *  Created on: Mar 16, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_FILTERS_TRIFILTERREGIONBOUNDS_HPP_
#define GEOMETRY_FILTERS_TRIFILTERREGIONBOUNDS_HPP_

#include "TetraMeshFilter.hpp"
#include "FilterBase.hpp"
#include <vector>

class TetraMesh;

/** Captures fluence passing exiting through faces that bound a region. If bidirectional is set,
 * also captures entering.
 */

class TriFilterRegionBounds : public TetraMeshFilter, public FilterBase<int>
{
public:
	/// Default-construct including all materials
	explicit TriFilterRegionBounds(const TetraMesh* m=nullptr) : TetraMeshFilter(m)
	{
	}

	void includeRegion(unsigned r,bool incl);

	void bidirectional(bool bidir);
	bool bidirectional() const;

	virtual bool operator()(const int x) const override;

	virtual const char* typeStr() const override { return "TriFilterRegionBounds"; }

private:
	bool m_bidirectional=true;
	std::vector<bool> m_regions;		///< Set of regions R whose boundaries to probe
};

#endif /* GEOMETRY_FILTERS_TRIFILTERREGIONBOUNDS_HPP_ */
