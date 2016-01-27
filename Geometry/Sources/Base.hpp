/*
 * Base.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_BASE_HPP_
#define GEOMETRY_SOURCES_BASE_HPP_

#include "Visitor.hpp"

namespace Source {
namespace detail {

/** Base class providing covariant clone method and delete */
template<typename Base,typename Derived>struct cloner : public Base
{
	void deleteSelf()
	{
		delete this;
	}

	typedef typename Base::Top Top;

	// provide covariant clone method
	virtual Base* cloneSelf() const override { return new Derived(static_cast<const Derived&>(*this)); }

	// provide forwarding constructor
	template<typename... Args>explicit cloner(Args... args) : Base(args...){}

	virtual void acceptVisitor(Source::Visitor* v) override
	{
		v->visit((Derived*)this);
	}

	// make sure destructor is virtual
	virtual ~cloner(){}
};
};



class Base
{
public:

	typedef Base Top;

	Base* base() { return this; }

	virtual ~Base(){}

	float 		power() 		const 	{ return m_power; 	}
	void 		power(float p)			{ m_power=p; 		}

	virtual Base*		cloneSelf() const=0;

	virtual void acceptVisitor(Visitor* v)=0;

	Base(float p) : m_power(p){}

private:
	float m_power=1.0f;
};

};


#endif /* GEOMETRY_SOURCES_BASE_HPP_ */
