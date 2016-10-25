/*
 * clonable.hpp
 *
 *  Created on: Oct 20, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_CLONABLE_HPP_
#define OUTPUTTYPES_CLONABLE_HPP_

#ifndef SWIG
#define CLONE_METHOD(Base,Derived) virtual Base* clone() const override { return new Derived (static_cast < const Derived& >(*this)); }
#else
#define CLONE_METHOD(Base,Derived) virtual Base* clone() const override;
#endif

/** Serves as the base class of a hierarchy of clonable types.
 *
 */

template<class Base>class clonable_base
{
public:
	virtual ~clonable_base(){}
	virtual Base* clone() const=0;

protected:
	clonable_base(){}
};


/** Adds cloning functionality to the Derived class, which must be derived from Base and copyable.
 *
 */

template<class Base,class Derived>class clonable
{
public:
	clonable(){}
	virtual Base* clone() const { return new Derived(static_cast<const Derived&>(*this)); }
};

#endif /* OUTPUTTYPES_CLONABLE_HPP_ */
