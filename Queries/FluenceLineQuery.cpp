/*
 * LineQuery.cpp
 *
 *  Created on: Mar 5, 2016
 *      Author: jcassidy
 */

#include "FluenceLineQuery.hpp"

FluenceLineQuery::FluenceLineQuery()
{
}
//
//void FluenceLineQuery::update(boost::any_range<const WalkSegment,boost::forward_traversal_tag,const WalkSegment&,std::ptrdiff_t> R,
//		const VolumeFluenceMap& phi)
//{
//	for(const auto seg : R)
//	{
//		TetraInfo i;
//		i.p0 = seg.f0.p;
//		i.IDt = seg.IDt;
//		i.lSeg = seg.lSeg;
//		i.matID = seg.matID;
//		i.phi = phi[seg.IDt];
//		i.dToOrigin = seg.dToOrigin;
//		m_depthTetraFluence.push_back(i);
//	}
//}
