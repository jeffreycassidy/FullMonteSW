/*
 * TetraVolumeKernel.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include "TetraVolumeKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <FullMonteSW/OutputTypes/OutputDataSummarize.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <list>

// TODO: This doesn't belong here
template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

void TetraVolumeKernel::postfinish()
{
	std::list<OutputData*> res = get<0>(m_logger).results();
	res.splice(res.end(), get<1>(m_logger).results());
	res.splice(res.end(), get<2>(m_logger).results());

	dynamic_cast<VolumeAbsorbedEnergyMap&>(*res.back()).totalEmitted(packetCount());

	for(auto r : res)
		addResults(r);
}
