/*
 * TetraSurfaceKernel.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Kernels/Software/TetraSurfaceKernel.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerTuple.hpp>
#include "TetraMCKernelThread.hpp"

#include <FullMonteSW/OutputTypes/FluenceConverter.hpp>

void TetraSurfaceKernel::postfinish()
{
	std::list<OutputData*> res = get<0>(m_logger).results();
	res.splice(res.end(), get<1>(m_logger).results());
	res.splice(res.end(), get<2>(m_logger).results());

	dynamic_cast<SurfaceExitEnergyMap&>(*res.back()).totalEmitted(packetCount());

	for(auto r : res)
		addResults(r);
}

void TetraSurfaceKernel::prestart()
{
	get<0>(m_logger).clear();
	get<1>(m_logger).clear();
	get<2>(m_logger).clear();
}

SurfaceFluenceMap TetraSurfaceKernel::getSurfaceFluenceMap() const
{
	const SurfaceExitEnergyMap *E = getResultByType<SurfaceExitEnergyMap>();

	FluenceConverter FC;
	FC.mesh(mesh());
	FC.cmPerOutputLengthUnit(1.0f);
	FC.scaleTotalEmittedTo(energy());
	SurfaceFluenceMap phi = FC.convertToFluence(*E);

	return phi;
}

