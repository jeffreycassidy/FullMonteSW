/*
 * TetraSurfaceKernel.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include <FullMonte/Kernels/Software/TetraSurfaceKernel.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerTuple.hpp>
#include "TetraMCKernelThread.hpp"

#include <FullMonte/OutputTypes/FluenceConverter.hpp>

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

std::vector<double> TetraSurfaceKernel::getSurfaceFluence() const
{
	const SurfaceExitEnergyMap *E = getResultByType<SurfaceExitEnergyMap>();

	FluenceConverter FC;
	FC.mesh(mesh());
	FC.cmPerOutputLengthUnit(1.0f);
	FC.scaleTotalEmittedTo(energy());
	SurfaceFluenceMap phi = FC.convertToFluence(*E);

	std::vector<double> o(phi->dim(),0.0);

	for(const auto t : phi->nonzeros())
		o[t.first] = t.second;

	return o;
}
