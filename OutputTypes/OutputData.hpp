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

#include "named_type.hpp"

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
//	virtual void doVisit(void* v)
//	{
//		std::cout << "doVisit(void*)" << std::endl;
//	}
};

inline void OutputData::acceptVisitor(Visitor* v)
{
	v->doVisit(this);
}

//class OutputData::Visitor
//{
//public:
//	virtual ~Visitor(){}
//
//	void visit(OutputData* d){ if(d) d->acceptVisitor(this); }
//
//	// visitor implementations
//	virtual void doVisit(OutputData* d)
//		{ throw std::logic_error("OutputData::Visitor method not implemented: generic case doVisit(OutputData*)"); }
//
//	virtual void doVisit(MCConservationCountsOutput* cc)
//		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(MCConservationCountsOutput*)"); }
//
//	virtual void doVisit(MCEventCountsOutput* cc)
//		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(MCEventCountsOutput*)"); }
//
//	virtual void doVisit(AbstractSpatialMap* M)
//		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(AbstractSpatialMap*)"); }
//
////	virtual void doVisit(VolumeAbsorbedEnergyMap* em)
////		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(VolumeAbsorbedEnergyMap*)"); }
////
////	virtual void doVisit(SurfaceExitEnergyMap* sm)
////		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(SurfaceExitEnergyMap*)"); }
////
////	virtual void doVisit(SurfaceFluenceMap* sm)
////		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(SurfaceFluenceMap*)"); }
////
////	virtual void doVisit(VolumeFluenceMap* vm)
////		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(VolumeFluenceMap*)"); }
////
////	virtual void doVisit(FluenceLineQuery* flq)
////		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(FluenceLineQuery*)"); }
////
////	virtual void doVisit(InternalSurfaceEnergyMap* flq)
////		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(InternalSurfaceEnergyMap*)"); }
////
////	virtual void doVisit(InternalSurfaceFluenceMap* flq)
////		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(InternalSurfaceFluenceMap*)"); }
//
//protected:
//	Visitor(){}
//};



#endif /* OUTPUTTYPES_OUTPUTDATA_HPP_ */
