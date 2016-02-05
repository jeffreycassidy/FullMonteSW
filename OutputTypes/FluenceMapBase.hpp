/*
 * FluenceMapBase.hpp
 *
 *  Created on: Feb 4, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_FLUENCEMAPBASE_HPP_
#define OUTPUTTYPES_FLUENCEMAPBASE_HPP_

#include "SpatialMapBase.hpp"

#include <memory>
#include <vector>
#include <utility>

class EnergyMapBase
{
public:
	EnergyMapBase(SpatialMapBase<float,unsigned>* m) :
		m_map(m)
	{}

	void joulesPerEnergyUnit(float j)					{ m_joulesPerEnergyUnit=j; 		}
	float joulesPerEnergyUnit()					const	{ return m_joulesPerEnergyUnit;	}

	void totalEmitted(float E)							{ m_totalEmitted=E;				}
	float totalEmitted()						const	{ return m_totalEmitted;		}

	const SpatialMapBase<float,unsigned>* operator->() const { return m_map.get();		}

	void set(SpatialMapBase<float,unsigned>* m){ m_map.reset(m); }

private:
	std::unique_ptr<SpatialMapBase<float,unsigned>>		m_map;
	float 												m_totalEmitted=0.0f;
	float 												m_joulesPerEnergyUnit=0.0f;
};

class FluenceMapBase : public EnergyMapBase
{
public:
	FluenceMapBase(SpatialMapBase<float,unsigned>* m) :
		EnergyMapBase(m)
	{}

	void cmPerLengthUnit(float l)						{ m_cmPerLengthUnit=l;			}
	float cmPerLengthUnit()						const	{ return m_cmPerLengthUnit;		}

private:
	float 												m_cmPerLengthUnit=0.0f;
};

class SurfaceExitEnergyMap : public EnergyMapBase
{
public:
	SurfaceExitEnergyMap(SpatialMapBase<float,unsigned>* m) :
		EnergyMapBase(m)
	{
	}

};


class VolumeAbsorbedEnergyMap : public FluenceMapBase
{
public:
	VolumeAbsorbedEnergyMap(SpatialMapBase<float,unsigned>* m) :
		FluenceMapBase(m)
	{
	}

};

class VolumeFluenceMap : public FluenceMapBase
{
public:

	VolumeFluenceMap(SpatialMapBase<float,unsigned>* m) :
		FluenceMapBase(m)
	{}

};

#endif /* OUTPUTTYPES_FLUENCEMAPBASE_HPP_ */
