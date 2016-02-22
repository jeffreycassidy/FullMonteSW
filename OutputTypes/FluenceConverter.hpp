/*
 * FluenceConverter.hpp
 *
 *  Created on: Feb 20, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_FLUENCECONVERTER_HPP_
#define OUTPUTTYPES_FLUENCECONVERTER_HPP_

#include <FullMonte/OutputTypes/FluenceMapBase.hpp>

class FluenceConverter
{
public:
	FluenceConverter();

	const TetraMesh* 					mesh() 											const 	{ return m_mesh; 		}
	void 								mesh(const TetraMesh* m)								{ m_mesh=m;				}

	const std::vector<SimpleMaterial>*	materials()										const	{ return m_materials;	}
	void 								materials(const std::vector<SimpleMaterial>* m)			{ m_materials=m;		}


	/// Convert absorbed energy to fluence using phi = E/V/mu_a [J/cm2]
	VolumeFluenceMap 					convertToFluence(const VolumeAbsorbedEnergyMap& E) const;

	/// Convert exiting energy to fluence by dividing by area
	SurfaceFluenceMap					convertToFluence(const SurfaceExitEnergyMap& E) const;

private:
	const TetraMesh*					m_mesh=nullptr;
	const std::vector<SimpleMaterial>*	m_materials=nullptr;
};

FluenceConverter::FluenceConverter()
{
}

VolumeFluenceMap FluenceConverter::convertToFluence(const VolumeAbsorbedEnergyMap& E) const
{
	std::vector<float> phi(E->dim(),0.0f);

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
			phi[e.first] = e.second / V / muA;
	}

	SpatialMapBase<float,unsigned>* phiMap = SpatialMapBase<float,unsigned>::newFromVector(phi);

	// pass units through
	VolumeFluenceMap o(phiMap);
	o.joulesPerEnergyUnit(E.joulesPerEnergyUnit());
	o.cmPerLengthUnit(E.cmPerLengthUnit());
	o.totalEmitted(E.totalEmitted());

	return o;
}


SurfaceFluenceMap FluenceConverter::convertToFluence(const SurfaceExitEnergyMap& E) const
{
	std::vector<float> phi(E->dim(),0.0f);

	if (!m_mesh)
		throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) m_mesh is NULL");

	if (E->dim() != m_mesh->getNf()+1)
		throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) dimension mismatch (geometry vs. exit vector)");

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
			phi[e.first] = e.second / A;
	}

	SpatialMapBase<float,unsigned>* phiMap = SpatialMapBase<float,unsigned>::newFromVector(phi);

	// pass units through
	SurfaceFluenceMap o(phiMap);
	o.joulesPerEnergyUnit(E.joulesPerEnergyUnit());
	o.cmPerLengthUnit(m_mesh->cmPerLengthUnit());
	o.totalEmitted(E.totalEmitted());

	return o;
}




#endif /* OUTPUTTYPES_FLUENCECONVERTER_HPP_ */
