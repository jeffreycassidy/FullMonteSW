/*
 * PrefixedUnit.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#include "UnitPrefix.hpp"
#include "PrefixedUnit.hpp"

namespace Units {

PrefixedUnit::PrefixedUnit(const UnitPrefix* pfx,const Unit* unit) :
		m_prefix(pfx),
		m_unit(unit)
{
}

PrefixedUnit::~PrefixedUnit()
{
}

const Unit* PrefixedUnit::base() const
{
	return m_unit;
}

float PrefixedUnit::scale() const
{
	return m_prefix->scale();
}

std::string PrefixedUnit::name() const
{
	return m_prefix->prefix()+m_unit->name();
}

std::string PrefixedUnit::symbol() const
{
	return m_prefix->symbol()+m_unit->symbol();
}

DerivedUnit PrefixedUnit::canonicalize() const
{
	DerivedUnit D = m_unit->canonicalize();
	D.scale *= m_prefix->scale();
	return D;
}
};
