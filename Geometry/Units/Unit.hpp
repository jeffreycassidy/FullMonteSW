/*
 * Units.hpp
 *
 *  Created on: Sep 22, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_UNITS_UNIT_HPP_
#define GEOMETRY_UNITS_UNIT_HPP_

#include <vector>
#include <string>

#include <boost/rational.hpp>

namespace Units {

class Unit;
class BaseUnit;

struct DerivedUnit
{
	/** A component of the product */
	struct Component
	{
		boost::rational<int>	exponent;
		const Unit*				unit;
	};

	DerivedUnit();
	DerivedUnit(const Unit*,boost::rational<int> exponent=1);
	DerivedUnit(float sc,std::vector<Component>&&);
	DerivedUnit(const BaseUnit& u);

	float						scale=1.0f;
	std::vector<Component>		components;

	std::string name() const;
	std::string symbol() const;

	DerivedUnit canonicalize() const;

	static bool convertible(const DerivedUnit& lhs,const DerivedUnit& rhs);
	static float convert(const DerivedUnit& from,const DerivedUnit& to,float qfrom=1.0f);

	// Simple concatenation of vectors, not even simplifying where duplicates exist
	DerivedUnit operator*(const DerivedUnit& rhs) const;
	DerivedUnit operator/(const DerivedUnit& rhs) const;
};


/** Unit is an abstract base class for any unit that could be attached to a physical quantity
 *
 */

class Unit
{
public:
	/// Constructor/destructor
	Unit();
	virtual ~Unit();

	/// True if unit is fundamental
	virtual bool fundamental() const;

	/// Query unit name & symbol
	virtual std::string	name() const=0;
	virtual std::string symbol() const=0;

	/// Convert to canonical representation (in terms of product of fundamental units)
	virtual DerivedUnit canonicalize() const=0;
};

};

#endif /* GEOMETRY_UNITS_UNIT_HPP_ */
