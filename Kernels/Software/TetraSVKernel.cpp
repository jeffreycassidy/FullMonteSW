/*
 * TetraSVKernel.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: jcassidy
 */

#include "TetraSVKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <list>

void TetraSVKernel::postfinish()
{
	std::list<OutputData*> res = get<0>(m_logger).results();
	res.splice(res.end(), get<1>(m_logger).results());

	res.splice(res.end(), get<2>(m_logger).results());
	dynamic_cast<VolumeAbsorbedEnergyMap&>(*res.back()).totalEmitted(packetCount());

	res.splice(res.end(), get<3>(m_logger).results());
	dynamic_cast<SurfaceExitEnergyMap&>(*res.back()).totalEmitted(packetCount());

	res.splice(res.end(), get<4>(m_logger).results());
	dynamic_cast<InternalSurfaceEnergyMap&>(*res.back()).totalEmitted(packetCount());

	for(auto r : res)
		addResults(r);
}
