/*
 * FluenceConverter.hpp
 *
 *  Created on: Feb 20, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_FLUENCECONVERTER_HPP_
#define OUTPUTTYPES_FLUENCECONVERTER_HPP_

#include <FullMonteSW/OutputTypes/FluenceMapBase.hpp>

class TetraMesh;

#include <FullMonteSW/Geometry/SimpleMaterial.hpp>

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

	VolumeAbsorbedEnergyMap				rescale(const VolumeAbsorbedEnergyMap& E) const;

	/// Convert absorbed energy to fluence using phi = E/V/mu_a [J/cm2]
	VolumeFluenceMap 					convertToFluence(const VolumeAbsorbedEnergyMap& E) const;

	/// Convert exiting energy to fluence by dividing by area
	SurfaceFluenceMap					convertToFluence(const SurfaceExitEnergyMap& E) const;

	/// Convert energy transiting a boundary to a per-unit-area fluence map
	InternalSurfaceFluenceMap			convertToFluence(const InternalSurfaceEnergyMap& E) const;

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

#endif /* OUTPUTTYPES_FLUENCECONVERTER_HPP_ */

