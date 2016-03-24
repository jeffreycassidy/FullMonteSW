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

#include "InternalSurface.hpp"

#include <memory>
#include <vector>
#include <utility>

#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/split_member.hpp>

#include "nan_wrapper.hpp"

typedef nan_wrapper<float> nanwrapperF;
BOOST_CLASS_IS_WRAPPER(nanwrapperF);

#include "UnitsBase.hpp"

struct VolumeFluenceMapTraits
{
	typedef float 			Value;
	typedef unsigned 		Index;
	static constexpr bool 	summable=false;
};

struct VolumeAbsorbedEnergyMapTraits
{
	typedef float			Value;
	typedef unsigned 		Index;
	static constexpr bool 	summable=true;
};

struct InternalSurfaceEnergyMapTraits
{
	typedef InternalSurface<float>		Value;
	typedef unsigned					Index;
	static constexpr bool 				summable=true;
};

struct SurfaceExitEnergyMapTraits
{
	typedef float						Value;
	typedef unsigned					Index;
	static constexpr bool 				summable=true;
};

struct SurfaceFluenceMapTraits
{
	typedef float						Value;
	typedef unsigned					Index;
	static constexpr bool 				summable=false;
};

struct InternalSurfaceFluenceMapTraits
{
	typedef InternalSurface<float>		Value;
	typedef unsigned					Index;
	static constexpr bool				summable=false;
};



template<class Traits>class SpatialMapOutputData : public UnitsBase, public clonable<OutputData,SpatialMapOutputData<Traits>,OutputData::Visitor>
{
public:
	typedef SpatialMapBase<typename Traits::Value,typename Traits::Index> SpatialMapType;
	SpatialMapOutputData(SpatialMapType* m=nullptr) :

		m_map(m)
	{}

	virtual ~SpatialMapOutputData(){}


	SpatialMapOutputData(const SpatialMapOutputData& E) :
		UnitsBase(E),
		m_map(E->clone())
		{}

	SpatialMapOutputData(SpatialMapOutputData&& E) :
		UnitsBase(E),
		m_map(std::move(E.m_map))
	{
	}

	SpatialMapOutputData& operator=(SpatialMapOutputData&& E)
	{
		UnitsBase::operator=(E);
		m_map = std::move(E.m_map);
		return *this;
	}

	const SpatialMapType* operator->() 	const 	{ return m_map.get();		}
	const SpatialMapType* get() 		const 	{ return m_map.get(); 	}

	void set(SpatialMapType* m)					{ m_map.reset(m); }

	typename Traits::Value operator[](unsigned i) const { return (*m_map.get())[i]; }

private:
	std::unique_ptr<SpatialMapType>		m_map;

	template<class Archive>void serialize(Archive& ar,const unsigned ver)
	{
		boost::serialization::void_cast_register<SpatialMapOutputData,OutputData>(
			static_cast<SpatialMapOutputData*>(nullptr),
			static_cast<OutputData*>(nullptr));

		ar & boost::serialization::base_object<UnitsBase>(*this);
		ar & boost::serialization::make_nvp("values",m_map);

		}
	friend class boost::serialization::access;
};

BOOST_CLASS_IS_WRAPPER(OutputData)

#endif /* OUTPUTTYPES_FLUENCEMAPBASE_HPP_ */
