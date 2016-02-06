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

template<class Base,class Derived>class clonable : public Base
{
public:
	virtual Base* clone() const override { return new Derived(static_cast<const Derived&>(*this)); }
	template<typename... Args> clonable(Args... args) : Base(args...){}

	virtual const std::string& typeString() const override { return s_typeString; }

private:
	static const std::string s_typeString;

};

//template<class Base,class Derived>class Visitable : virtual pu
//{
//public:
//	typedef typename Base::Visitor Visitor;
//
//protected:
//	Visitable(){}
//
//public:
//	virtual void acceptVisitor(Visitor* v) override
//	{
//		v->visit((Derived*)this);
//	}
//};
//
//template<class VisitorType>class VisitableBase
//{
//public:
//	typedef VisitorType Visitor;
//
//	virtual void acceptVisitor(Visitor* v)=0;
//};

#include <string>

class OutputData;

class MCConservationCountsOutput;
class MCEventCountsOutput;

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
	virtual void doVisit(OutputData* d)=0;
	virtual void doVisit(MCConservationCountsOutput* cc)=0;
	virtual void doVisit(MCEventCountsOutput* cc)=0;

protected:
	Visitor(){}
};



#endif /* OUTPUTTYPES_OUTPUTDATA_HPP_ */
