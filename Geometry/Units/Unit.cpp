/*
 * Units.cpp
 *
 *  Created on: Sep 22, 2016
 *      Author: jcassidy
 */

#include "Unit.hpp"
#include <cmath>
#include <map>

#include <sstream>
#include <iostream>
#include "BaseUnit.hpp"

namespace Units {


Unit::Unit()
{
}

Unit::~Unit()
{
}

bool Unit::fundamental() const
{
	return false;
}

DerivedUnit::DerivedUnit()
{
}

DerivedUnit::DerivedUnit(const Unit* u,boost::rational<int> exponent) :
		components{Component{exponent,u}}
{
}

DerivedUnit::DerivedUnit(float sc,std::vector<Component>&& il_) : scale(sc),components(il_)
{
}

DerivedUnit::DerivedUnit(const BaseUnit& u) :
		components{ DerivedUnit::Component { 1, &u } }
{
}

std::string DerivedUnit::name() const
{
	return "*** Undef ***";
}

std::string DerivedUnit::symbol() const
{
	std::stringstream ss;
	if (scale != 1.0f)
		ss << "*" << scale << ' ';
	for(const auto c : components)
	{
		ss << c.unit->symbol();
		if (c.exponent != boost::rational<int>(1))
		{
			ss << "^";
			if (c.exponent.denominator()==1)
				ss << c.exponent.numerator();
			else
				ss << c.exponent;
		}
		ss << ' ';
	}
	return ss.str();
}

DerivedUnit DerivedUnit::canonicalize() const
{
	// map of base unit (by pointer) to its exponent
	std::map<const Unit*,boost::rational<int>> baseUnits;

	// grab current unit scale
	float k=scale;

	for(const auto& c : components)
	{
		DerivedUnit u = c.unit->canonicalize();

		// update scale with sub-unit scale
		k *= std::pow(u.scale,boost::rational_cast<float>(c.exponent));

		// update base-unit exponents
		for(const auto& subcomp : u.components)
			baseUnits[subcomp.unit] += subcomp.exponent*c.exponent;
	}

	DerivedUnit D;
	D.scale=k;
	for(const auto& p : baseUnits)
		D.components.push_back(DerivedUnit::Component { p.second, p.first });

	return D;
}

bool DerivedUnit::convertible(const DerivedUnit& lhs,const DerivedUnit& rhs)
{
	DerivedUnit lhsc = lhs.canonicalize();
	DerivedUnit rhsc = rhs.canonicalize();

	if (lhsc.components.size() != rhsc.components.size())
		return false;

	for(unsigned i=0;i<lhsc.components.size();++i)
		if (lhsc.components[i].exponent != rhsc.components[i].exponent)
			return false;

	return true;
}

float DerivedUnit::convert(const DerivedUnit& from,const DerivedUnit& to,float qlhs)
{
	DerivedUnit lhsc = from.canonicalize();
	DerivedUnit rhsc = to.canonicalize();

	if (!convertible(lhsc,rhsc))
		throw std::logic_error("invalid unit conversion");

	return lhsc.scale / rhsc.scale;
}

DerivedUnit DerivedUnit::operator*(const DerivedUnit& rhs) const
{
	DerivedUnit D = *this;
	D.scale *= rhs.scale;
	std::copy(rhs.components.begin(), rhs.components.end(), std::back_inserter(D.components));
	return D;
}

};
