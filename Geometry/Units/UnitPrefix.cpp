/*
 * UnitPrefix.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#include "UnitPrefix.hpp"

#include <string>

using namespace std;

namespace Units {

const UnitPrefix pico{1e-12f,"pico","p"};
const UnitPrefix nano{1e-9f,"nano","n"};
const UnitPrefix micro{1e-6f,"micro","u"};
const UnitPrefix milli{1e-3f,"milli","m"};
const UnitPrefix centi{1e-2f,"centi","c"};
const UnitPrefix kilo{1e3f,"kilo","k"};
const UnitPrefix mega{1e6f,"mega","M"};
const UnitPrefix giga{1e9f,"giga","G"};
const UnitPrefix tera{1e12f,"tera","T"};
const UnitPrefix peta{1e15f,"peta","P"};
const UnitPrefix exa{1e18f,"exa","E"};

UnitPrefix::UnitPrefix(float scale,std::string prefix,std::string sym) :
		m_scale(scale),
		m_prefix(prefix),
		m_prefixSym(sym)
{
}

float UnitPrefix::scale() const
{
	return m_scale;
}

const std::string& UnitPrefix::symbol() const
{
	return m_prefixSym;
}

const std::string& UnitPrefix::prefix() const
{
	return m_prefix;
}

};
