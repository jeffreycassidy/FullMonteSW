/*
 * TetraTraceKernel.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: jcassidy
 */

#include "TetraTraceKernel.hpp"
#include <FullMonteSW/OutputTypes/SpatialMap.hpp>

#include <FullMonteSW/OutputTypes/PacketPositionTraceSet.hpp>

void TetraTraceKernel::postfinish()
{
	const OutputData* d = getResultByIndex(3);
	const SpatialMap<float>* surf = dynamic_cast<const SpatialMap<float>*>(d);

	if (!surf)
	{
		cout << "ERROR: Cannot cast result to SpatialMap<float>, actual type is " << d->typeString() << endl;
		throw std::logic_error("Invalid result cast");
	}

	cout << "Surface exit scored with " << surfaceScorer().accumulator().retryCount() << " retries over " << surfaceScorer().accumulator().accumulationCount() << " accumulations"<< endl;
	cout << "  Event scorer recorded " << eventScorer().state().Nexit << " packets exiting" << endl;
	cout << "  Conservation shows total exiting weight of " << conservationScorer().state().w_exit << " compared to exit score of " << surf->sum() << endl;

	const PacketPositionTraceSet* P = getResultByType<PacketPositionTraceSet>();

	const auto& E = eventScorer().state();

	cout << "Path tracer recorded " << P->nTraces() << " traces with a total of " << P->nPoints() << " points, expecting " << E.Nlaunch << " and " << E.Nlaunch+E.Nrefr+E.Nfresnel+E.Ntir+E.Nscatter+E.Nexit+E.Ndie << endl;
}

void TetraTraceKernel::prepareScorer()
{
	get<2>(m_scorer).dim(mesh()->getNt()+1);
	get<3>(m_scorer).dim(mesh()->getNf()+1);

	cout << "Resized for " << get<2>(m_scorer).dim() << " tetras and " << get<3>(m_scorer).dim() << " faces" << endl;
	cout << "  Face scorer has address " << &get<3>(m_scorer) << " and accumulator buffer size "<< get<3>(m_scorer).accumulator().size() << std::endl;
}

void TetraTraceKernel::prestart()
{
}

