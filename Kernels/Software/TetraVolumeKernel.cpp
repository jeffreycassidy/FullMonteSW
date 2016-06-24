/*
 * TetraVolumeKernel.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include "TetraVolumeKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <FullMonte/OutputTypes/OutputDataSummarize.hpp>
#include <FullMonte/OutputTypes/FluenceConverter.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <list>

void TetraVolumeKernel::postfinish()
{
	std::list<OutputData*> res = get<0>(m_logger).results();
	res.splice(res.end(), get<1>(m_logger).results());
	res.splice(res.end(), get<2>(m_logger).results());

	dynamic_cast<VolumeAbsorbedEnergyMap&>(*res.back()).totalEmitted(packetCount());

	for(auto r : res)
		addResults(r);
}

VolumeFluenceMap TetraVolumeKernel::getVolumeFluence() const
{
	const VolumeAbsorbedEnergyMap *E = getResultByType<VolumeAbsorbedEnergyMap>();

	FluenceConverter FC;
	FC.mesh(mesh());
	FC.cmPerOutputLengthUnit(1.0f);
	FC.scaleTotalEmittedTo(energy());
	FC.materials(&m_materials);
	VolumeFluenceMap phi = FC.convertToFluence(*E);

	return phi;
}
