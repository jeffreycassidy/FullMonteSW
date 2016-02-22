/*
 * VolumeFluenceMap.cpp
 *
 *  Created on: Feb 4, 2016
 *      Author: jcassidy
 */

#include "FluenceMapBase.hpp"
#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/SimpleMaterial.hpp>

#include <vector>

VolumeFluenceMap convertAbsorbedEnergyToFluence(const VolumeAbsorbedEnergyMap& E, const TetraMesh& M, const std::vector<SimpleMaterial>& mat)
{
	std::vector<Value> phi(E->dim(), 0.0f);

	unsigned idx;
	float e;

	// TODO: Incorporate length units for mu_a

	for(const auto p : E.nonzeros())
	{
		std::tie(idx,e) = p;

		float V = M.getTetraVolume(idx);
		unsigned matID = M.getMaterial(idx);
		float mu_a = mat[matID].mu_a;

		phi[idx] = e/(V*mu_a);
	}

	SpatialMapBase<float,unsigned>* m = SpatialMapBase<float,unsigned>::newFromVector(phi);

	VolumeFluenceMap VM(m);
	VM.joulesPerEnergyUnit(E.joulesPerEnergyUnit());
	VM.cmPerLengthUnit(M.cmPerLengthUnit());
	VM.totalEmitted(E.totalEmitted());

	return VM;
}
