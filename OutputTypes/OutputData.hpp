/*
 * OutputData.hpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_OUTPUTDATA_HPP_
#define OUTPUTTYPES_OUTPUTDATA_HPP_

#include <string>

template<class Base>class clonable_base
{
public:
	virtual ~clonable_base(){}

	virtual Base* clone() const=0;

	virtual const std::string& typeString() const { return s_typeString; }

protected:
	clonable_base(){}

	static const std::string s_typeString;
};

template<class Base,class Derived,class Visitor>class clonable : public Base
{
public:
	virtual Base* clone() const override { return new Derived(static_cast<const Derived&>(*this)); }
	template<typename... Args> clonable(Args... args) : Base(args...){}

	virtual const std::string& typeString() const override { return s_typeString; }

	virtual void acceptVisitor(Visitor* v) override{ v->doVisit((Derived*)this); }

private:
	static const std::string s_typeString;

};

#include <string>
#include <stdexcept>

class OutputData;

class MCConservationCountsOutput;
class MCEventCountsOutput;
class VolumeAbsorbedEnergyMap;
class SurfaceFluenceMap;
class VolumeFluenceMap;
class SurfaceExitEnergyMap;
class FluenceLineQuery;
class InternalSurfaceEnergyMap;
class InternalSurfaceFluenceMap;

class OutputData : public clonable_base<OutputData>
{
public:
	class Visitor;

private:
	virtual void acceptVisitor(Visitor* v)=0;
};

class OutputData::Visitor
{
public:
	virtual ~Visitor(){}

	void visit(OutputData* d){ d->acceptVisitor(this); }

	// visitor implementations
	virtual void doVisit(OutputData* d)
		{ throw std::logic_error("OutputData::Visitor method not implemented: generic case doVisit(OutputData*)"); }

	virtual void doVisit(MCConservationCountsOutput* cc)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(MCConservationCountsOutput*)"); }

	virtual void doVisit(MCEventCountsOutput* cc)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(MCEventCountsOutput*)"); }

	virtual void doVisit(VolumeAbsorbedEnergyMap* em)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(VolumeAbsorbedEnergyMap*)"); }

	virtual void doVisit(SurfaceExitEnergyMap* sm)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(SurfaceExitEnergyMap*)"); }

	virtual void doVisit(SurfaceFluenceMap* sm)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(SurfaceFluenceMap*)"); }

	virtual void doVisit(VolumeFluenceMap* vm)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(VolumeFluenceMap*)"); }

	virtual void doVisit(FluenceLineQuery* flq)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(FluenceLineQuery*)"); }

	virtual void doVisit(InternalSurfaceEnergyMap* flq)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(InternalSurfaceEnergyMap*)"); }

	virtual void doVisit(InternalSurfaceFluenceMap* flq)
		{ throw std::logic_error("OutputData::Visitor method not implemented: doVisit(InternalSurfaceFluenceMap*)"); }

protected:
	Visitor(){}
};



#endif /* OUTPUTTYPES_OUTPUTDATA_HPP_ */
