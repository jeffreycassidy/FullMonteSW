/*
 * PlacementBase.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_PLACEMENT_PLACEMENTBASE_HPP_
#define GEOMETRY_PLACEMENT_PLACEMENTBASE_HPP_

#include <vector>
#include <boost/range.hpp>

class PlacementMediatorBase;

/** PlacementBase is a base class for a source placement. It holds a collection of PlacementMediatorBase*, which defines the
 * relationship between the placement and an underlying source type.
 *
 */

namespace Source { class Abstract; }

class PlacementBase
{
public:
	virtual ~PlacementBase();

	PlacementBase();

	/// Add/remove sources from the placement
	void addSource(PlacementMediatorBase* pmb);
	void removeSource(PlacementMediatorBase* pmb);

	/// Add/remove detectors from the placement
	void addDetector(PlacementMediatorBase* pmb);
	void removeDetector(PlacementMediatorBase* pmb);

	/// Update all underlying source & detector reps by calling their update() method
	void update();

	boost::iterator_range<std::vector<PlacementMediatorBase*>::const_iterator> sources()
		{ return m_sources; }

	Source::Abstract*		source();

private:
	std::vector<PlacementMediatorBase*>		m_sources;
	std::vector<PlacementMediatorBase*>		m_detectors;
};

#endif /* GEOMETRY_PLACEMENT_PLACEMENTBASE_HPP_ */
