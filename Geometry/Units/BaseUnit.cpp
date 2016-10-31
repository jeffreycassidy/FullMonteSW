/*
 * FundamentalUnit.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#include "BaseUnit.hpp"

namespace Units {

BaseUnit::BaseUnit(std::string name,std::string sym) :
		m_name(name),
		m_textSym(sym)
{
}

BaseUnit::~BaseUnit()
{
}

bool BaseUnit::fundamental() const
{
	return true;
}

std::string BaseUnit::name() const
{
	return m_name;
}

std::string BaseUnit::symbol() const
{
	return m_textSym;
}

DerivedUnit BaseUnit::canonicalize() const
{
	return DerivedUnit{ 1.0, std::vector<DerivedUnit::Component>{
		DerivedUnit::Component{ boost::rational<int>(1), this } }};
}

const BaseUnit metre{"metre","m"};
const BaseUnit second{"second","s"};
const BaseUnit kilogram{"kilogram","kg"};
const BaseUnit ampere{"Ampere","A"};
const BaseUnit kelvin{"Kelvin","K"};
const BaseUnit candela{"candela","cd"};
const BaseUnit mole{"mole","mol"};

const BaseUnit packet{"packet","au"};

};
