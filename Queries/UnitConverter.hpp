/*
 * UnitConverter.hpp
 *
 *  Created on: Oct 11, 2016
 *      Author: jcassidy
 */

#ifndef QUERIES_UNITCONVERTER_HPP_
#define QUERIES_UNITCONVERTER_HPP_

class UnitConverter
{
public:

private:
};

/// Rescale input such that a total of E units of energy were emitted
void scaleTotalEmittedTo(float E)					{ m_scaleToTotalE=E;					}

void joulesPerOutputEnergyUnit(float j)				{ m_joulesPerOutputEnergyUnit=j;		}
float joulesPerOutputEnergyUnit() 			const 	{ return m_joulesPerOutputEnergyUnit; 	}

void cmPerOutputLengthUnit(float l)					{ m_cmPerOutputLengthUnit=l;			}
float cmPerOutputLengthUnit() 				const 	{ return m_cmPerOutputLengthUnit; 		}


//	VolumeAbsorbedEnergyMap				rescale(const VolumeAbsorbedEnergyMap& E) const;
//
//	/// Convert absorbed energy to fluence using phi = E/V/mu_a [J/cm2]
//	VolumeFluenceMap 					convertToFluence(const VolumeAbsorbedEnergyMap& E) const;
//
//	/// Convert exiting energy to fluence by dividing by area
//	SurfaceFluenceMap					convertToFluence(const SurfaceExitEnergyMap& E) const;
//
//	/// Convert energy transiting a boundary to a per-unit-area fluence map
//	InternalSurfaceFluenceMap			convertToFluence(const InternalSurfaceEnergyMap& E) const;
//
//
//	/// Convert absorbed energy in an element to an energy density
//	VolumeAbsorbedEnergyDensityMap		convertToEnergyDensity(const VolumeAbsorbedEnergyMap& E) const;


float m_scaleToTotalE = std::numeric_limits<float>::quiet_NaN();			///< Total output energy post-scaling (nan -> pass through)

float m_joulesPerOutputEnergyUnit=std::numeric_limits<float>::quiet_NaN();	///< Output energy unit size in Joules (nan -> pass through)
float m_cmPerOutputLengthUnit=std::numeric_limits<float>::quiet_NaN();		///< Output length unit size in cm (nan -> pass through)

float energyScale(float) const;		///< E_output / E_input
	float areaScale(float) const;		///< A_output / A_input
	float volumeScale(float) const;		///< V_output / V_input
	float lengthScale(float) const;		///< L_output / L_input


	float EnergyToFluence::energyScale(float total) const
	{
		float kE = 1.0f;

		if (!isnan(m_scaleToTotalE))
			kE = m_scaleToTotalE / total;

		return kE;
	}

	float EnergyToFluence::lengthScale(float inputCMPerLengthUnit) const
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

	float EnergyToFluence::areaScale(float inputCMPerLengthUnit) const
	{
		float kL = lengthScale(inputCMPerLengthUnit);
		return kL*kL;
	}

	float EnergyToFluence::volumeScale(float inputCMPerLengthUnit) const
	{
		float kL = lengthScale(inputCMPerLengthUnit);
		return kL*kL*kL;
	}


	VolumeFluenceMap EnergyToFluence::convertToFluence(const VolumeAbsorbedEnergyMap& E) const
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


	VolumeAbsorbedEnergyDensityMap EnergyToFluence::convertToEnergyDensity(const VolumeAbsorbedEnergyMap& E) const
	{
		std::vector<float> ef(E->dim(),0.0f);

		float kE = energyScale(E.totalEmitted());
		float kV = volumeScale(E.cmPerLengthUnit());

		float k = kE/kV;

		if (!m_mesh)
			throw std::logic_error("FluenceConverter::convertToEnergyDensity(const VolumeAbsorbedEnergyMap&) m_mesh is NULL");

		if (E->dim() != m_mesh->getNt()+1)
			throw std::logic_error("FluenceConverter::convertToEnergyDensity(const VolumeAbsorbedEnergyMap&) dimension mismatch (geometry vs. absorption vector)");

		for(const auto e : E->nonzeros())
		{
			float V = m_mesh->getTetraVolume(e.first);

			if (isnan(e.second) || e.second < 0.0f)
				throw std::logic_error("FluenceConverter::convertToEnergyDensity(const VolumeAbsorbedEnergyMap&) invalid (negative/nan) element in absorption vector");

			if (V < 0.0f)
				throw std::logic_error("FluenceConverter::convertToEnergyDensity(const VolumeAbsorbedEnergyMap&) negative tetra volume");

			if (V == 0.0f)
			{
				if (e.second > 0.0f)
					throw std::logic_error("FluenceConverter::::convertToEnergyDensity(const VolumeAbsorbedEnergyMap&) impossible absorption: zero volume");
				ef[e.first] = 0.0f;
			}
			else
				ef[e.first] = k*e.second / V;		// V [cm3] * muA [cm-1] => cm2
		}

		SpatialMapBase<float,unsigned>* EDMap = SpatialMapBase<float,unsigned>::newFromVector(ef);

		VolumeAbsorbedEnergyDensityMap o(EDMap);

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


	VolumeAbsorbedEnergyMap EnergyToFluence::rescale(const VolumeAbsorbedEnergyMap& E) const
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

	//
	//SurfaceFluenceMap EnergyToFluence::convertToFluence(const SurfaceExitEnergyMap& E) const
	//{
	//	std::vector<float> phi(E->dim(),0.0f);
	//
	//	if (!m_mesh)
	//		throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) m_mesh is NULL");
	//
	//	if (E->dim() != m_mesh->getNf()+1)
	//		throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) dimension mismatch (geometry vs. exit vector)");
	//
	//
	//	float kA = areaScale(m_mesh->cmPerLengthUnit());
	//	float kE = energyScale(E.totalEmitted());
	//
	//	float kPhi = kE/kA;
	//
	//	//cout << "FluenceConverter (E -> surface phi) scaling energy by kE=" << kE << " area by kA=" << kA << endl;
	//
	//
	//	for(const auto e : E->nonzeros())
	//	{
	//		float A = m_mesh->getFaceArea(e.first);
	//
	//		if (isnan(e.second) || e.second < 0.0f)
	//			throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) invalid (negative/nan) element in exit vector");
	//
	//		if (A < 0.0f)
	//			throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) negative face area");
	//
	//		if (A == 0.0f)
	//		{
	//			if (e.second > 0.0f)
	//				throw std::logic_error("FluenceConverter::convertToFluence(const SurfaceExitEnergyMap&) impossible exit: zero area");
	//			phi[e.first] = 0.0f;
	//		}
	//		else
	//			phi[e.first] = kPhi * e.second / A;
	//	}
	//
	//	SpatialMapBase<float,unsigned>* phiMap = SpatialMapBase<float,unsigned>::newFromVector(phi);
	//
	//	// pass units through
	//	SurfaceFluenceMap o(phiMap);
	//
	//	o.joulesPerEnergyUnit(
	//			isnan(m_joulesPerOutputEnergyUnit) ?
	//					E.joulesPerEnergyUnit() :
	//					m_joulesPerOutputEnergyUnit);
	//
	//	o.cmPerLengthUnit(
	//			isnan(m_cmPerOutputLengthUnit) ?
	//					m_mesh->cmPerLengthUnit() :
	//					m_cmPerOutputLengthUnit);
	//
	//	o.totalEmitted(E.totalEmitted()*kE);
	//
	//	return o;
	//}


	InternalSurfaceFluenceMap EnergyToFluence::convertToFluence(const InternalSurfaceEnergyMap& E) const
	{
		std::vector<DirectedSurfaceElement<float>> phi(E->dim(),InternalSurface<float>());

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

		SpatialMapBase<DirectedSurfaceElement<float>,unsigned>* phiMap = SpatialMapBase<DirectedSurfaceElement<float>,unsigned>::newFromVector(phi);

		// pass units through
		InternalSurfaceFluenceMap o(phiMap);

		return o;
	}

#endif /* QUERIES_UNITCONVERTER_HPP_ */
