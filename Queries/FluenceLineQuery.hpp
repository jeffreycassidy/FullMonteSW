/*
 * LineQuery.hpp
 *
 *  Created on: Mar 5, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_FLUENCELINEQUERY_HPP_
#define OUTPUTTYPES_FLUENCELINEQUERY_HPP_

#include <array>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/range/any_range.hpp>
#include <FullMonteSW/Geometry/RayWalk.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>

#include <FullMonteSW/OutputTypes/SpatialMap.hpp>

#include <FullMonteSW/OutputTypes/OutputData.hpp>

class TetraMesh;

class FluenceLineQuery
{
public:
	FluenceLineQuery();

//	void update(boost::any_range<const WalkSegment,boost::forward_traversal_tag,const WalkSegment&,std::ptrdiff_t> R,
//			const VolumeFluenceMap& phi);

	struct TetraInfo
		{
			std::array<float,3>		p0;			///< Point of entry
			float					dToOrigin;	///< Distance along path
			float					lSeg;		///< Segment length
			unsigned				IDt;		///< Tetra ID
			float					phi;		///< Fluence
			unsigned				matID;		///< Material ID
		};

	boost::iterator_range<std::vector<TetraInfo>::const_iterator> result() const
		{
			return m_depthTetraFluence;
		}

private:
	std::vector<TetraInfo> 		m_depthTetraFluence;

	// serialization support
	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ ar & m_depthTetraFluence; }
	friend class boost::serialization::access;
};

#endif /* OUTPUTTYPES_FLUENCELINEQUERY_HPP_ */
