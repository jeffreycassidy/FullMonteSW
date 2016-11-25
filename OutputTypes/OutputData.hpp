/*
 * OutputData.hpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_OUTPUTDATA_HPP_
#define OUTPUTTYPES_OUTPUTDATA_HPP_

#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

#include <string>
#include <stdexcept>

class OutputData;

class AbstractSpatialMap;
class MCConservationCountsOutput;
class MCEventCountsOutput;

#include <boost/serialization/access.hpp>
#include <boost/serialization/void_cast.hpp>

#include <iostream>

#include "clonable.hpp"
#include "visitable.hpp"

class MCConservationCountsOutput;
class MCEventCountsOutput;
template<typename Value>class SpatialMap;

class OutputData
{
public:
	virtual ~OutputData(){}

	class Visitor;
	virtual void acceptVisitor(Visitor* v);

	virtual OutputData* clone() const=0;

	virtual const char* typeString() const { return "(generic-output-data)"; }
};

#ifndef SWIG
class OutputData::Visitor
{
public:
	Visitor(){}
	virtual ~Visitor(){}

	virtual void doVisit(MCEventCountsOutput* C)
	{
		std::cout << "doVisit(MCEventCountsData*)" << std::endl;
	}
	virtual void doVisit(MCConservationCountsOutput* C)
	{
		std::cout << "doVisit(MCConservationCountsData*)" << std::endl;
	}
	virtual void doVisit(SpatialMap<float>* M)
	{
		std::cout << "doVisit(SpatialMap<float>*)" << std::endl;
	}
	virtual void doVisit(OutputData* D)
	{
		std::cout << "doVisit(OutputData*)" << std::endl;
	}
};

#endif

#endif /* OUTPUTTYPES_OUTPUTDATA_HPP_ */
