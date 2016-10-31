/*
 * PrefixedUnit.hpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_UNITS_PREFIXEDUNIT_HPP_
#define GEOMETRY_UNITS_PREFIXEDUNIT_HPP_

#include <string>

#include "Unit.hpp"

namespace Units {

class UnitPrefix;

class PrefixedUnit : public Unit
{
public:
	PrefixedUnit(const UnitPrefix* pfx,const Unit* unit);
	virtual ~PrefixedUnit();

	virtual std::string name() const override;
	virtual std::string symbol() const override;

	const Unit* base() const;
	float scale() const;

	virtual DerivedUnit canonicalize() const override;

private:
	const UnitPrefix* 	m_prefix;
	const Unit*			m_unit;
};

};

#endif /* GEOMETRY_UNITS_PREFIXEDUNIT_HPP_ */
