/*
 * PlaneCrossingFilter.hpp
 *
 *  Created on: Aug 20, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_FILTERS_PLANECROSSINGFILTER_HPP_
#define GEOMETRY_FILTERS_PLANECROSSINGFILTER_HPP_

#include <FullMonteSW/Geometry/Filters/TetraMeshFilterBase.hpp>
#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>
#include <array>
#include <vector>

class PlaneCrossingFilter : public TetraMeshFilterBase, public FilterBase<unsigned>
{
public:
	PlaneCrossingFilter();
	virtual ~PlaneCrossingFilter();

	void plane(std::array<float,3> normal,float constant);

	virtual bool operator()(unsigned i) const override;

private:
	virtual void postMeshUpdate() override;

	std::array<float,3>		m_normal = std::array<float,3>{{0.0f,0.0f,0.0f}};
	float					m_constant=0;

	std::vector<bool> m_pointAbovePlane;		///< Map for all mesh points, true if above plane
};



#endif /* GEOMETRY_FILTERS_PLANECROSSINGFILTER_HPP_ */
