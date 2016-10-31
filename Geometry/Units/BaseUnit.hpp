/*
 * FundamentalUnit.hpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_UNITS_BASEUNIT_HPP_
#define GEOMETRY_UNITS_BASEUNIT_HPP_

#include <string>

#include "Unit.hpp"

/** Represents a base unit in the SI sense (a unit which is the foundation for other units, not expressed
 * in terms of any other unit).
 */

namespace Units {

class BaseUnit : public Unit
{
public:
	BaseUnit(std::string name,std::string sym);
	virtual ~BaseUnit();

	virtual bool fundamental() const override;

	virtual std::string name() const override;
	virtual std::string symbol() const override;

	virtual DerivedUnit canonicalize() const override;

private:
	std::string					m_name;
	std::string					m_textSym;
};

// The SI base units
extern const BaseUnit metre;
extern const BaseUnit second;
extern const BaseUnit kilogram;
extern const BaseUnit ampere;
extern const BaseUnit kelvin;
extern const BaseUnit candela;
extern const BaseUnit mole;

extern const BaseUnit packet;

};

#endif /* GEOMETRY_UNITS_BASEUNIT_HPP_ */
