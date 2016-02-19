/*
 * BlockRandomDistribution.hpp
 *
 *  Created on: Feb 12, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_BLOCKRANDOMDISTRIBUTION_HPP_
#define KERNELS_SOFTWARE_BLOCKRANDOMDISTRIBUTION_HPP_

#include <cstddef>
#include <boost/align/is_aligned.hpp>

namespace RandomDistribution
{

template<class RNG,class Distribution,typename T>void generate(RNG& rng,Distribution& d,T* o)
{
	d.calculate(rng,o);
}

};


/** Buffers a random-number distribution that calculates its results in blocks.
 *
 * @tparam		Distribution		The distribution class
 * @tparam		OutputBlocks		Number of output blocks to buffer
 */

/**
 * @tparam		RNG					Random number generator which functions with the chosen Distribution through
 * 										BlockDistribution::generate(rng, distribution, Nblocks)
 */

template<class Distribution,std::size_t OutputBlocks=1,std::size_t Align=Distribution::OutputElementSize*sizeof(typename Distribution::result_type)>class alignas(Align) BlockRandomDistribution
{
public:
	typedef typename Distribution::result_type	result_type;
	static constexpr std::size_t				outputPitch = Distribution::OutputElementSize;
	static constexpr std::size_t				outputBlockSize = Distribution::OutputElementSize*Distribution::OutputsPerInputBlock;
	static constexpr std::size_t				outputBufferSize = outputBlockSize*OutputBlocks;

	BlockRandomDistribution();

	/// Return a pointer to a result, _possibly_ invoking the RNG if needed to refill the buffer
	template<class RNG>const result_type* draw(RNG& rng);
	template<class RNG>const result_type* operator()(RNG& rng){ return draw(rng); }

	template<class RNG>void generate(RNG& rng,std::size_t N)
	{
		for(unsigned i=0;i<N;i += outputBlockSize)
			m_distribution.calculate(rng,m_outputBuffer+i);
	}

	/// Return a reference to the generating distribution
	Distribution& distribution(){ return m_distribution; }

private:
	result_type		m_outputBuffer[outputBufferSize];
	unsigned		m_pos=0;
	Distribution	m_distribution;
};

template<class Distribution,std::size_t OutputBlocks,std::size_t Align>template<class RNG>const typename Distribution::result_type* BlockRandomDistribution<Distribution,OutputBlocks,Align>::draw(RNG& rng)
{
	if (m_pos == 0)
		for(unsigned i=0;i<OutputBlocks*outputBlockSize; i += outputBlockSize)
			RandomDistribution::generate(rng,m_distribution,m_outputBuffer+i);

	const typename Distribution::result_type* p = m_outputBuffer + m_pos;

	m_pos = (m_pos + outputPitch) % BlockRandomDistribution<Distribution,OutputBlocks,Align>::outputBufferSize;

	return p;
}

template<class Distribution,std::size_t OutputBlocks,std::size_t Align>BlockRandomDistribution<Distribution,OutputBlocks,Align>::BlockRandomDistribution()
{
	// Check that the buffer is aligned at least to the granularity of output elements
	if (!boost::alignment::is_aligned(Align,m_outputBuffer))
		throw std::bad_alloc();
}

#endif /* KERNELS_SOFTWARE_BLOCKRANDOMDISTRIBUTION_HPP_ */
