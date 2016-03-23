/*
 * FluenceMapBase.hpp
 *
 *  Created on: Feb 4, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_FLUENCEMAPBASE_HPP_
#define OUTPUTTYPES_FLUENCEMAPBASE_HPP_

#include "OutputData.hpp"
#include "SpatialMapBase.hpp"

#include <memory>
#include <vector>
#include <utility>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>

namespace boost { namespace serialization { class access; }; };

class EnergyUnitsBase
{
public:
	explicit EnergyUnitsBase(float totalEmitted=0.0f,float joulesPerEnergyUnit=std::numeric_limits<float>::quiet_NaN()) :
		m_totalEmitted(totalEmitted),
		m_joulesPerEnergyUnit(joulesPerEnergyUnit){}

	EnergyUnitsBase(const EnergyUnitsBase& E) = default;

	void joulesPerEnergyUnit(float j)					{ m_joulesPerEnergyUnit=j; 		}
	float joulesPerEnergyUnit()					const	{ return m_joulesPerEnergyUnit;	}

	void totalEmitted(float E)							{ m_totalEmitted=E;				}
	float totalEmitted()						const	{ return m_totalEmitted;		}

private:
	float 												m_totalEmitted=0.0f;
	float 												m_joulesPerEnergyUnit=std::numeric_limits<float>::quiet_NaN();
};

class EnergyMapBase : public EnergyUnitsBase
{
public:
	EnergyMapBase(SpatialMapBase<float,unsigned>* m=nullptr) :
		m_map(m)
	{}

	EnergyMapBase(const EnergyMapBase& E) :
		EnergyUnitsBase(E),
		m_map(E->clone())
		{}

	const SpatialMapBase<float,unsigned>* operator->() 	const { return m_map.get();		}
	const SpatialMapBase<float,unsigned>* get() 		const { return m_map.get(); }

	void set(SpatialMapBase<float,unsigned>* m){ m_map.reset(m); }

	float operator[](unsigned i) const { return (*m_map.get())[i]; }

private:
	std::unique_ptr<SpatialMapBase<float,unsigned>>		m_map;
	float 												m_totalEmitted=0.0f;
	float 												m_joulesPerEnergyUnit=std::numeric_limits<float>::quiet_NaN();

	friend class boost::serialization::access;
	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ ar & boost::serialization::base_object<EnergyUnitsBase>(*this) & m_map; }
};

class LengthUnitsBase
{
public:
	LengthUnitsBase(){}

	explicit LengthUnitsBase(float cmPerLengthUnit) : m_cmPerLengthUnit(cmPerLengthUnit){}

	void cmPerLengthUnit(float l)						{ m_cmPerLengthUnit=l;			}
	float cmPerLengthUnit()						const	{ return m_cmPerLengthUnit;		}

private:
	float 												m_cmPerLengthUnit=std::numeric_limits<float>::quiet_NaN();
};

class FluenceMapBase : public EnergyMapBase, public LengthUnitsBase
{
public:
	FluenceMapBase(SpatialMapBase<float,unsigned>* m=nullptr) :
		EnergyMapBase(m)
	{}

private:

	friend class boost::serialization::access;
	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ ar & boost::serialization::base_object<EnergyMapBase>(*this) & m_cmPerLengthUnit; }
};

class SurfaceExitEnergyMap : public EnergyMapBase, public clonable<OutputData,SurfaceExitEnergyMap,OutputData::Visitor>
{
public:
	SurfaceExitEnergyMap(SpatialMapBase<float,unsigned>* m=nullptr) :
		EnergyMapBase(m)
	{
	}

};

struct InternalSurface
{
	InternalSurface(){}
	InternalSurface(float exit_,float enter_) : exit(exit_),enter(enter_){}
	float exit=0.0f;
	float enter=0.0f;

	InternalSurface& operator+=(const InternalSurface rhs){ exit += rhs.exit; enter += rhs.enter; return *this; }
};

inline bool nonzero(const InternalSurface& S){ return S.exit != 0 || S.enter != 0; }

class InternalSurfaceEnergyMap : public EnergyUnitsBase, public clonable<OutputData,InternalSurfaceEnergyMap,OutputData::Visitor>
{
public:
	InternalSurfaceEnergyMap(SpatialMapBase<InternalSurface,unsigned>* m=nullptr) :
		m_map(m)
	{
	}

	const SpatialMapBase<InternalSurface,unsigned>* operator->() const { return m_map; }

	InternalSurface operator[](unsigned i) const { return (*m_map)[i]; }

private:
	const SpatialMapBase<InternalSurface,unsigned>* m_map=nullptr;

	friend class boost::serialization::access;
	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ ar & *m_map; }
};

class InternalSurfaceFluenceMap : public LengthUnitsBase, public EnergyUnitsBase, public clonable<OutputData,InternalSurfaceFluenceMap,OutputData::Visitor>
{
public:
	InternalSurfaceFluenceMap(SpatialMapBase<InternalSurface,unsigned>* m) :
		LengthUnitsBase(1.0f),
		EnergyUnitsBase(0.0f),
		m_map(m)
	{
	}

	boost::any_range<const std::pair<unsigned,InternalSurface>,boost::forward_traversal_tag> 	nonzeros() const { return m_map->nonzeros(); }
	boost::any_range<const std::pair<unsigned,InternalSurface>,boost::forward_traversal_tag> 	dense() const{ return m_map->dense(); }
	boost::any_range<const InternalSurface,boost::forward_traversal_tag>						values() const{ return m_map->values(); }

	unsigned dim() const { return m_map->dim(); }
	unsigned nnz() const { return m_map->nnz(); }

	const SpatialMapBase<InternalSurface,unsigned>* get() const { return m_map; }

	InternalSurface operator[](unsigned i) const { return (*m_map)[i]; }

private:
	const SpatialMapBase<InternalSurface,unsigned>* m_map=nullptr;

	friend class boost::serialization::access;
	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ ar & *m_map; }
};


class VolumeAbsorbedEnergyMap : public FluenceMapBase, public clonable<OutputData,VolumeAbsorbedEnergyMap,OutputData::Visitor>
{
public:
	VolumeAbsorbedEnergyMap(SpatialMapBase<float,unsigned>* m) :
		FluenceMapBase(m)
	{
	}
};

class VolumeFluenceMap : public FluenceMapBase, public clonable<OutputData,VolumeFluenceMap,OutputData::Visitor>
{
public:

	VolumeFluenceMap(SpatialMapBase<float,unsigned>* m) :
		FluenceMapBase(m)
	{}

};

class SurfaceFluenceMap : public FluenceMapBase, public clonable<OutputData,SurfaceFluenceMap,OutputData::Visitor>
{
public:

	SurfaceFluenceMap(SpatialMapBase<float,unsigned>* m=nullptr) :
		FluenceMapBase(m)
	{}

	using FluenceMapBase::cmPerLengthUnit;

};


#endif /* OUTPUTTYPES_FLUENCEMAPBASE_HPP_ */
