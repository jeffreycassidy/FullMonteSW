/*
 * FilterBase.hpp
 *
 *  Created on: Feb 25, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_FILTERS_FILTERBASE_HPP_
#define GEOMETRY_FILTERS_FILTERBASE_HPP_

/** Filters geometry elements of type T, returning true if they are included.
 */

template<typename T>class FilterBase
{
public:
	virtual bool operator()(T x) const { return false; }
};

template<typename T>class AlwaysTrue : public FilterBase<T>
{
public:
	AlwaysTrue(){}
	virtual bool operator()(T x) const override { return true; }
};

#endif /* GEOMETRY_FILTERS_FILTERBASE_HPP_ */
