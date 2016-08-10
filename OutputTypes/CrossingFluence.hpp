/*
 * CrossingFluence.hpp
 *
 *  Created on: Jul 22, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_CROSSINGFLUENCE_HPP_
#define OUTPUTTYPES_CROSSINGFLUENCE_HPP_

#include "FluenceMapBase.hpp"

class BidirectionalFluence
{
public:
	/// Get/set the dataset being operated on
	const InternalSurfaceFluenceMap*	source() const;
	void								source(const InternalSurfaceFluenceMap* s);

	/// Returns whatever result type (enter, exit, bidir) is currently selected
	SurfaceFluenceMap				result() const;

private:
	const InternalSurfaceFluenceMap* m_source=nullptr;
};

SurfaceFluenceMap BidirectionalFluence::result() const
{
	vector<float> phi((*m_source)->dim(),0.0f);

	for(unsigned i=0;i<(*m_source)->dim();++i)
		phi[i] = (*m_source)[i].enter + (*m_source)[i].exit;

	SurfaceFluenceMap S(SpatialMapBase<float,unsigned>::newFromVector(phi));

	return S;
}

const InternalSurfaceFluenceMap* BidirectionalFluence::source() const
{
	return m_source;
}

void BidirectionalFluence::source(const InternalSurfaceFluenceMap* s)
{
	m_source=s;
}



#endif /* OUTPUTTYPES_CROSSINGFLUENCE_HPP_ */
