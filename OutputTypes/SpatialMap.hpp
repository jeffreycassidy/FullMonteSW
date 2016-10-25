#ifndef SPATIALMAP_INCLUDED_
#define SPATIALMAP_INCLUDED_

#include "OutputData.hpp"
#include "AbstractSpatialMap.hpp"

#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>

using namespace std;

/** Represents a spatial map of some property over elements in a geometry */


template<typename Value>class SpatialMap :
		public AbstractSpatialMap
{
public:
	explicit SpatialMap(std::size_t N=0,SpaceType stype=UnknownSpaceType,ValueType vtype=Scalar);

	SpatialMap(const std::vector<Value>&,SpaceType stype=UnknownSpaceType,ValueType vtype=Scalar);
	SpatialMap(std::vector<Value>&&,SpaceType stype=UnknownSpaceType,ValueType vtype=Scalar);

	virtual ~SpatialMap();

#ifndef SWIG
	CLONE_METHOD(OutputData,SpatialMap)
	ACCEPT_VISITOR_METHOD(OutputData,SpatialMap)
#endif

	virtual void			dim(std::size_t N) 	override;
	virtual std::size_t		dim() 				const override;

	const Value& 	operator[](std::size_t i) const;
	Value& 			operator[](std::size_t i);

	Value			sum() const;

	virtual const char* typeString() const override { return "SpatialMap"; }

	boost::iterator_range<typename std::vector<Value>::const_iterator> values() const { return m_values; }

private:
	std::vector<Value>		m_values;
};

template<typename Value>SpatialMap<Value>::SpatialMap
	(std::vector<Value>&& src,SpaceType stype,ValueType vtype) :
		AbstractSpatialMap(stype,vtype),
		m_values(std::move(src))
{
}

template<typename Value>SpatialMap<Value>::SpatialMap
	(const std::vector<Value>& src,SpaceType stype,ValueType vtype) :
		AbstractSpatialMap(stype,vtype),
		m_values(src)
{
}

template<typename Value>SpatialMap<Value>::SpatialMap
	(std::size_t N,SpaceType stype,ValueType vtype) :
		AbstractSpatialMap(stype,vtype),
		m_values(N,Value())
{
}

template<typename Value>SpatialMap<Value>::~SpatialMap()
{
}

template<typename Value>void SpatialMap<Value>::dim(std::size_t N)
{
	m_values.resize(N,Value());
}

template<typename Value>std::size_t SpatialMap<Value>::dim() const
{
	return m_values.size();
}

template<typename Value>const Value& SpatialMap<Value>::operator[](std::size_t i) const
{
	return m_values[i];
}

template<typename Value>Value& SpatialMap<Value>::operator[](std::size_t i)
{
	return m_values[i];
}

template<typename Value>Value SpatialMap<Value>::sum() const
{
	Value s();
	for(const auto v : m_values)
		s += v;
	return s;
}

#endif
