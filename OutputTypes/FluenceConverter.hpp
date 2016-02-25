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

	/// Rescale input such that a total of E units of energy were emitted
	void scaleTotalEmittedTo(float E)					{ m_scaleToTotalE=E;					}

	void joulesPerOutputEnergyUnit(float j)				{ m_joulesPerOutputEnergyUnit=j;		}
	float joulesPerOutputEnergyUnit() 			const 	{ return m_joulesPerOutputEnergyUnit; 	}

	void cmPerOutputLengthUnit(float l)					{ m_cmPerOutputLengthUnit=l;			}
	float cmPerOutputLengthUnit() 				const 	{ return m_cmPerOutputLengthUnit; 		}

	/// Convert absorbed energy to fluence using phi = E/V/mu_a [J/cm2]
	VolumeFluenceMap 					convertToFluence(const VolumeAbsorbedEnergyMap& E) const;

	/// Convert exiting energy to fluence by dividing by area
	SurfaceFluenceMap					convertToFluence(const SurfaceExitEnergyMap& E) const;

	/// Convert energy transiting a boundary to a per-unit-area fluence map
	InternalSurfaceFluenceMap			convertToFluence(const InternalSurfaceEnergyMap& E) const;

	enum Direction { Enter, Exit, Bidirectional };
	SurfaceFluenceMap					convertToFluence(const InternalSurfaceEnergyMap& E,Direction d) const;

private:
	float energyScale(float) const;		///< E_output / E_input
	float areaScale(float) const;		///< A_output / A_input
	float volumeScale(float) const;		///< V_output / V_input
	float lengthScale(float) const;		///< L_output / L_input

	const TetraMesh*					m_mesh=nullptr;
	const std::vector<SimpleMaterial>*	m_materials=nullptr;

	float m_scaleToTotalE = std::numeric_limits<float>::quiet_NaN();			///< Total output energy post-scaling (nan -> pass through)

	float m_joulesPerOutputEnergyUnit=std::numeric_limits<float>::quiet_NaN();	///< Output energy unit size in Joules (nan -> pass through)
	float m_cmPerOutputLengthUnit=std::numeric_limits<float>::quiet_NaN();		///< Output length unit size in cm (nan -> pass through)
};

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

	cout << "FluenceConverter (E -> vol phi) scaling energy by kE=" << kE << " area by kA=" << kA << endl;

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

	cout << "FluenceConverter (E -> surface phi) scaling energy by kE=" << kE << " area by kA=" << kA << endl;


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




#endif /* OUTPUTTYPES_FLUENCECONVERTER_HPP_ */
