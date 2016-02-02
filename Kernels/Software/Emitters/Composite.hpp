/*
 * Composite.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_COMPOSITE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_COMPOSITE_HPP_

#include <vector>
#include <boost/random/discrete_distribution.hpp>

namespace Emitter
{

/** Emitter composed of a vector of sub-emitters */

template<class RNG>class Composite : public EmitterBase<RNG>
{
public:

	template<class ConstIterator>SourceMultiEmitter(ConstIterator begin,ConstIterator end) :
		SourceEmitter<RNG>(mesh_),
		m_sourceDistribution(
			boost::make_transform_iterator(begin,	std::mem_fn(&SourceEmitter<RNG>::power)),
			boost::make_transform_iterator(end,		std::mem_fn(&SourceEmitter<RNG>::power)))
	{
		for(; begin != end; ++begin)
			//emitters.push_back(SourceEmitterFactory<RNG>(mesh_,**begin));
			m_emitters.push_back(*begin);
	}

	LaunchPacket emit(RNG& rng) const
	{
	    return m_emitters[m_sourceDistribution(rng)]->emit(rng);
	}

private:
	vector<EmitterBase<RNG>*> 						m_emitters;
	boost::random::discrete_distribution<unsigned> 	m_sourceDistribution;
};

};


#endif /* KERNELS_SOFTWARE_EMITTERS_COMPOSITE_HPP_ */
