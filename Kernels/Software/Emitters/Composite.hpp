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

/** Emitter composed of a vector of sub-emitters (owned by pointer) */

template<class RNG>class Composite : public EmitterBase<RNG>
{
public:

	virtual ~Composite()
	{
		// delete the emitters
		for(const auto e : m_emitters)
			delete e;
	}


	/** Provided a range that dereferences to pair<float,EmitterBase*>, constructs a Composite emitter.
	 * The Composite owns the pointers provided to it, deleting them when it is deleted.
	 */

	template<class ConstRange>Composite(ConstRange R)
	{
		std::vector<float> power;

		for(const std::pair<float,EmitterBase<RNG>*> p : R)
		{
			power.push_back(p.first);
			m_emitters.push_back(p.second);
		}

		m_sourceDistribution = boost::random::discrete_distribution<unsigned>(power);

		// std::cout << "  New Emitter::Composite with " << m_emitters.size() << " elements" << std::endl;
	}

	LaunchPacket emit(RNG& rng) const
	{
	    return m_emitters[m_sourceDistribution(rng)]->emit(rng);
	}

private:
	std::vector<const EmitterBase<RNG>*> 			m_emitters;				///< Owning pointer to sub-emitters
	boost::random::discrete_distribution<unsigned> 	m_sourceDistribution;	///< Distribution for power
};

};


#endif /* KERNELS_SOFTWARE_EMITTERS_COMPOSITE_HPP_ */
