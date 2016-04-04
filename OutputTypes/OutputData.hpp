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

	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ ar & boost::serialization::base_object<Base>(*this); }

	friend class boost::serialization::access;
};

#include <string>
#include <stdexcept>

class OutputData;

struct VolumeFluenceMapTraits;
struct VolumeAbsorbedEnergyMapTraits;
struct InternalSurfaceFluenceMapTraits;
struct InternalSurfaceEnergyMapTraits;
struct SurfaceFluenceMapTraits;
struct SurfaceExitEnergyMapTraits;

template<typename Traits>class SpatialMapOutputData;
using VolumeFluenceMap = SpatialMapOutputData<VolumeFluenceMapTraits>;
using SurfaceExitEnergyMap = SpatialMapOutputData<SurfaceExitEnergyMapTraits>;
using SurfaceFluenceMap = SpatialMapOutputData<SurfaceFluenceMapTraits>;
using VolumeAbsorbedEnergyMap = SpatialMapOutputData<VolumeAbsorbedEnergyMapTraits>;
using InternalSurfaceFluenceMap = SpatialMapOutputData<InternalSurfaceFluenceMapTraits>;
using InternalSurfaceEnergyMap = SpatialMapOutputData<InternalSurfaceEnergyMapTraits>;

class MCConservationCountsOutput;
class MCEventCountsOutput;
class FluenceLineQuery;

#include <boost/serialization/access.hpp>
#include <boost/serialization/void_cast.hpp>

#include <iostream>

class OutputData : public clonable_base<OutputData>
{
public:
	class Visitor;
	virtual ~OutputData(){}

private:
	virtual void acceptVisitor(Visitor* v)=0;

	template<class Archive>void serialize(Archive& ar,const unsigned)
		{ }
	friend class boost::serialization::access;
};


class OutputData::Visitor
{
public:
	virtual ~Visitor(){}

	void visit(OutputData* d){ if(d) d->acceptVisitor(this); }

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
