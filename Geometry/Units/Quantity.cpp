/*
 * Quantity.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jcassidy
 */

#include "Quantity.hpp"

namespace Units {

const Quantity mass			{Quantity::Scalar,"mass",	"m"};
const Quantity length		{Quantity::Scalar,"length",	"l"};
const Quantity area			{Quantity::Scalar,"area",	"A"};
const Quantity volume		{Quantity::Scalar,"volume",	"V"};
const Quantity time			{Quantity::Scalar,"time",	"t"};

const Quantity fluence		{Quantity::Scalar,"fluence",		"phi"};
const Quantity fluenceRate	{Quantity::Scalar,"fluence rate",	"phi"};
const Quantity irradiance	{Quantity::Scalar,"irradiance",		"I"};

//const Quantity surfaceEnergyDensity {Quantity::Scalar,"Energy Density","E_s"};
//const Quantity volumeEnergyDensity	{Quantity::Scalar,"Energy Density","E_v"};
const Quantity energy		{Quantity::Scalar,"energy",			"E"};


//////// Derived units
//
Quantity::Quantity(Quantity::Type type,std::string name,std::string sym) :
		m_type(type),
		m_name(name),
		m_textSym(sym)
{

}

Quantity::~Quantity()
{

}

std::string Quantity::name() const
{
	return m_name;
}

std::string Quantity::symbol() const
{
	return m_textSym;
}


};
