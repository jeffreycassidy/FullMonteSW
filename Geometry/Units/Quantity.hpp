/*
 * Quantity.hpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_UNITS_QUANTITY_HPP_
#define GEOMETRY_UNITS_QUANTITY_HPP_

#include <string>

namespace Units {

class Unit;



/** A physical quantity in a unit system - no units attached.
 */

class Quantity
{
public:
	enum Type { Scalar, Vector };

	Quantity(Type t,std::string name,std::string sym);
	~Quantity();

	virtual std::string name() 		const;
	virtual std::string symbol() 	const;

private:
	Type										m_type;
	std::string									m_name;
	std::string									m_textSym;
};







////// Fundamental units
extern const Quantity length;
extern const Quantity mass;
extern const Quantity current;
//extern const Quantity intensity;	// not needed
//extern Quantity time;				// name conflict
//extern const Quantity mole;
//extern const Quantity kelvin;

////// Derived quantities
extern const Quantity fluence;
extern const Quantity energy;

};
#endif /* GEOMETRY_UNITS_QUANTITY_HPP_ */
