/*
 * TetraSVKernel.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: jcassidy
 */

#include "TetraSVKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <list>

#include <FullMonteSW/OutputTypes/FluenceConverter.hpp>

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

// TODO: Deduplicate this from TetraVolumeKernel
VolumeFluenceMap TetraSVKernel::getVolumeFluenceMap() const
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

VolumeAbsorbedEnergyMap TetraSVKernel::getVolumeAbsorbedEnergyMap() const
{
	const VolumeAbsorbedEnergyMap *E = getResultByType<VolumeAbsorbedEnergyMap>();

	FluenceConverter FC;
	FC.cmPerOutputLengthUnit(1.0f);
	FC.scaleTotalEmittedTo(energy());

	return FC.rescale(*E);
}

// TODO: Deduplicate this from TetraSurfaceKernel
SurfaceFluenceMap TetraSVKernel::getSurfaceFluenceMap() const
{
	const SurfaceExitEnergyMap *E = getResultByType<SurfaceExitEnergyMap>();

	FluenceConverter FC;
	FC.mesh(mesh());
	FC.cmPerOutputLengthUnit(1.0f);
	FC.scaleTotalEmittedTo(energy());
	SurfaceFluenceMap phi = FC.convertToFluence(*E);

	return phi;
}

InternalSurfaceFluenceMap TetraSVKernel::getInternalSurfaceFluenceMap() const
{
	const InternalSurfaceEnergyMap *E = getResultByType<InternalSurfaceEnergyMap>();

	FluenceConverter FC;
	FC.mesh(mesh());
	FC.cmPerOutputLengthUnit(1.0f);
	FC.scaleTotalEmittedTo(energy());
	InternalSurfaceFluenceMap phi = FC.convertToFluence(*E);

	return phi;
}


