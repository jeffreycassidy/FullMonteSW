/*
 * FluenceConverter.cpp
 *
 *  Created on: Mar 6, 2016
 *      Author: jcassidy
 */


#include "FluenceConverter.hpp"

#include "FluenceMapBase.hpp"

#include <FullMonteSW/Geometry/TetraMesh.hpp>

FluenceConverter::FluenceConverter()
{
}

float FluenceConverter::energyScale(float total) const
{
	float kE = 1.0f;

	if (!isnan(m_scaleToTotalE))
		kE = m_scaleToTotalE / total;

	return kE;
}

float FluenceConverter::lengthScale(float inputCMPerLengthUnit) const
{
	float kL = 1.0f;

	if (!isnan(m_cmPerOutputLengthUnit))
	{
		if (isnan(inputCMPerLengthUnit))
			kL = 1.0f;
		else
			kL = inputCMPerLengthUnit / m_cmPerOutputLengthUnit;
	}

	return kL;
}

float FluenceConverter::areaScale(float inputCMPerLengthUnit) const
{
	float kL = lengthScale(inputCMPerLengthUnit);
	return kL*kL;
}

float FluenceConverter::volumeScale(float inputCMPerLengthUnit) const
{
	float kL = lengthScale(inputCMPerLengthUnit);
	return kL*kL*kL;
}

VolumeFluenceMap FluenceConverter::convertToFluence(const VolumeAbsorbedEnergyMap& E) const
{
	std::vector<float> phi(E->dim(),0.0f);

	float kE = energyScale(E.totalEmitted());
	float kA = areaScale(E.cmPerLengthUnit());

	float kPhi = kE/kA;

	//cout << "FluenceConverter (E -> vol phi) scaling energy by kE=" << kE << " area by kA=" << kA << endl;

	if (!m_mesh)
		throw std::logic_error("FluenceConverter::convertToFluence(const VolumeAbsorbedEnergyMap&) m_mesh is NULL");

	if (!m_materials)
		throw std::logic_error("FluenceConverter::convertToFluence(const VolumeAbsorbedEnergyMap&) m_materials is NULL");

	if (E->dim() != m_mesh->getNt()+1)
		throw std::logic_error("FluenceConverter::convertToFluence(const VolumeAbsorbedEnergyMap&) dimension mismatch (geometry vs. absorption vector)");

	for(const auto e : E->nonzeros())
	{
		float V = m_mesh->getTetraVolume(e.first);
		float muA = (*m_materials)[m_mesh->getMaterial(e.first)].muA();

		if (isnan(e.second) || e.second < 0.0f)
			throw std::logic_error("FluenceConverter::convertToFluence(const VolumeAbsorbedEnergyMap&) invalid (negative/nan) element in absorption vector");

		if (V < 0.0f)
			throw std::logic_error("FluenceConverter::convertToFluence(const VolumeAbsorbedEnergyMap&) negative tetra volume");

		if (V == 0.0f || muA == 0.0f)
		{
			if (e.second > 0.0f)
				throw std::logic_error("FluenceConverter::convertToFluence(const VolumeAbsorbedEnergyMap&) impossible absorption: zero volume or zero mu_a");
			phi[e.first] = 0.0f;
		}
		else
			phi[e.first] = kPhi*e.second / (V * muA);		// V [cm3] * muA [cm-1] => cm2
	}

	SpatialMapBase<float,unsigned>* phiMap = SpatialMapBase<float,unsigned>::newFromVector(phi);

	VolumeFluenceMap o(phiMap);

	// set units: if we don't have a unit specifier (it's NAN) then pass through original units

	o.joulesPerEnergyUnit(
			isnan(m_joulesPerOutputEnergyUnit) ?
					E.joulesPerEnergyUnit() :
					m_joulesPerOutputEnergyUnit);

	o.cmPerLengthUnit(
			isnan(m_cmPerOutputLengthUnit) ?
					E.cmPerLengthUnit() :
					m_cmPerOutputLengthUnit);

	o.totalEmitted(E.totalEmitted()*kE);

	return o;
}


VolumeAbsorbedEnergyMap FluenceConverter::rescale(const VolumeAbsorbedEnergyMap& E) const
{
	std::vector<float> o(E->dim(),0.0f);
	float kE = energyScale(E.totalEmitted());

	for(const auto e : E->nonzeros())
	{
		if (isnan(e.second) || e.second < 0.0f)
			throw std::logic_error("FluenceConverter::rescale(const VolumeAbsorbedEnergyMap&) invalid (negative/nan) element in exit vector");
		else if (e.second > 0.0f)
			o[e.first] = kE * e.second;
	}

	SpatialMapBase<float,unsigned>* eMap = SpatialMapBase<float,unsigned>::newFromVector(o);

	// pass units through
	VolumeAbsorbedEnergyMap OE(eMap);

	OE.joulesPerEnergyUnit(
			isnan(m_joulesPerOutputEnergyUnit) ?
					E.joulesPerEnergyUnit() :
					m_joulesPerOutputEnergyUnit);

	OE.cmPerLengthUnit(E.cmPerLengthUnit());

	OE.totalEmitted(E.totalEmitted()*kE);

	return OE;
}

SurfaceFluenceMap FluenceConverter::convertToFluence(const SurfaceExitEnergyMap& E) const
{
	std::vector<float> phi(E->dim(),0.0f);

	if (!m_mesh)
		throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) m_mesh is NULL");

	if (E->dim() != m_mesh->getNf()+1)
		throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) dimension mismatch (geometry vs. exit vector)");


	float kA = areaScale(m_mesh->cmPerLengthUnit());
	float kE = energyScale(E.totalEmitted());

	float kPhi = kE/kA;

	//cout << "FluenceConverter (E -> surface phi) scaling energy by kE=" << kE << " area by kA=" << kA << endl;


	for(const auto e : E->nonzeros())
	{
		float A = m_mesh->getFaceArea(e.first);

		if (isnan(e.second) || e.second < 0.0f)
			throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) invalid (negative/nan) element in exit vector");

		if (A < 0.0f)
			throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) negative face area");

		if (A == 0.0f)
		{
			if (e.second > 0.0f)
				throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) impossible exit: zero area");
			phi[e.first] = 0.0f;
		}
		else
			phi[e.first] = kPhi * e.second / A;
	}

	SpatialMapBase<float,unsigned>* phiMap = SpatialMapBase<float,unsigned>::newFromVector(phi);

	// pass units through
	SurfaceFluenceMap o(phiMap);

	o.joulesPerEnergyUnit(
			isnan(m_joulesPerOutputEnergyUnit) ?
					E.joulesPerEnergyUnit() :
					m_joulesPerOutputEnergyUnit);

	o.cmPerLengthUnit(
			isnan(m_cmPerOutputLengthUnit) ?
					m_mesh->cmPerLengthUnit() :
					m_cmPerOutputLengthUnit);

	o.totalEmitted(E.totalEmitted()*kE);

	return o;
}


InternalSurfaceFluenceMap FluenceConverter::convertToFluence(const InternalSurfaceEnergyMap& E) const
{
	std::vector<InternalSurface<float>> phi(E->dim(),InternalSurface<float>());

	if (!m_mesh)
		throw std::logic_error("FluenceConverter::convertToFluence(const InternalSurfaceExitEnergyMap&) m_mesh is NULL");

	if (E->dim() != m_mesh->getNf()+1)
		throw std::logic_error("FluenceConverter::convertToFluence(const InternalSurfaceExitEnergyMap&) dimension mismatch (geometry vs. exit vector)");


	float kA = areaScale(m_mesh->cmPerLengthUnit());
	float kE = energyScale(E.totalEmitted());

	float kPhi = kE/kA;

	//cout << "FluenceConverter (E -> internal surface phi) scaling energy by kE=" << kE << " area by kA=" << kA << endl;

	for(const auto e : E->nonzeros())
	{
		float A = m_mesh->getFaceArea(e.first);

		if (isnan(e.second.exit) || e.second.exit < 0.0f || isnan(e.second.enter) || e.second.enter < 0.0f)
			throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) invalid (negative/nan) element in face-crossing vector");

		if (A < 0.0f)
			throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) negative face area");

		if (A == 0.0f)
		{
			if (e.second.exit > 0.0f || e.second.enter > 0.0f)
				throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) impossible exit: zero area");
			phi[e.first] = InternalSurface<float>(0.0f,0.0f);
		}
		else
			phi[e.first] = InternalSurface<float>(
								kPhi * e.second.exit  / A,
								kPhi * e.second.enter / A);
	}

	SpatialMapBase<InternalSurface<float>,unsigned>* phiMap = SpatialMapBase<InternalSurface<float>,unsigned>::newFromVector(phi);

	// pass units through
	InternalSurfaceFluenceMap o(phiMap);

	return o;
}
