/*
 * SpatialMap2D2D.hpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_SPATIALMAP2D_HPP_
#define OUTPUTTYPES_SPATIALMAP2D_HPP_

#include "OutputData.hpp"

#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>

using namespace std;

/** Represents a spatial map of some property over elements in a geometry */

template<typename Value>class SpatialMap2D : public AbstractSpatialMap
{
public:
	explicit SpatialMap2D(std::size_t w,std::size_t h);

	SpatialMap2D(const std::vector<Value>&,SpaceType stype=UnknownSpaceType,ValueType vtype=Scalar);
	SpatialMap2D(std::vector<Value>&&,SpaceType stype=UnknownSpaceType,ValueType vtype=Scalar);

	virtual ~SpatialMap2D();

#ifndef SWIG
	CLONE_METHOD(OutputData,SpatialMap2D)
	ACCEPT_VISITOR_METHOD(OutputData,SpatialMap2D)
#endif

	virtual void			dim(std::size_t N) 	override;
	virtual std::size_t		dim() 				const override;

	const Value& 	operator[](std::size_t i) const;
	Value& 			operator[](std::size_t i);

	const Value& 	operator[](std::size_t x,std::size_t y) const;
	Value& 			operator[](std::size_t x,std::size_t y);

	Value			sum() const;

	virtual const char* typeString() const override { return "SpatialMap2D"; }

	boost::iterator_range<typename std::vector<Value>::const_iterator> values() const { return m_values; }

private:
	std::size_t				m_w=0,m_h=0;
	std::vector<Value>		m_values;
};

template<typename Value>SpatialMap2D<Value>::SpatialMap2D
	(std::vector<Value>&& src,SpaceType stype,ValueType vtype) :
		AbstractSpatialMap2D(stype,vtype),
		m_values(std::move(src))
{
}

template<typename Value>SpatialMap2D<Value>::SpatialMap2D
	(const std::vector<Value>& src,SpaceType stype,ValueType vtype) :
		AbstractSpatialMap2D(stype,vtype),
		m_values(src)
{
}

template<typename Value>SpatialMap2D<Value>::SpatialMap2D
	(std::size_t N,SpaceType stype,ValueType vtype) :
		AbstractSpatialMap2D(stype,vtype),
		m_values(N,Value())
{
}

template<typename Value>SpatialMap2D<Value>::~SpatialMap2D()
{
}

template<typename Value>void SpatialMap2D<Value>::dim(std::size_t N)
{
	m_values.resize(N,Value());
}

template<typename Value>std::size_t SpatialMap2D<Value>::dim() const
{
	return m_values.size();
}

template<typename Value>const Value& SpatialMap2D<Value>::operator[](std::size_t i) const
{
	return m_values[i];
}

template<typename Value>Value& SpatialMap2D<Value>::operator[](std::size_t i)
{
	return m_values[i];
}

template<typename Value>Value SpatialMap2D<Value>::sum() const
{
	Value s();
	for(const auto v : m_values)
		s += v;
	return s;
}


#endif /* OUTPUTTYPES_SpatialMap2D2D_HPP_ */
