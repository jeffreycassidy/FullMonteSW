/*
 * UnitPrefix.hpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_UNITS_UNITPREFIX_HPP_
#define GEOMETRY_UNITS_UNITPREFIX_HPP_

#include <string>

namespace Units {

/** Class holding unit prefixes (eg. milli, micro, etc) with the associated scalar value, full name, and symbol
 */

class UnitPrefix
{
public:
	UnitPrefix(float scale,std::string prefix,std::string sym);

	const std::string& prefix() const;
	const std::string& symbol() const;

	float scale() const;

private:
	float 			m_scale;
	std::string		m_prefix;
	std::string		m_prefixSym;

};

extern const UnitPrefix pico;
extern const UnitPrefix nano;
extern const UnitPrefix micro;
extern const UnitPrefix milli;
extern const UnitPrefix centi;
extern const UnitPrefix kilo;
extern const UnitPrefix mega;
extern const UnitPrefix giga;
extern const UnitPrefix tera;
extern const UnitPrefix peta;
extern const UnitPrefix exa;

};

#endif /* GEOMETRY_UNITS_UNITPREFIX_HPP_ */
