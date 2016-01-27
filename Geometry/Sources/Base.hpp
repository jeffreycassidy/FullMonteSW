/*
 * Base.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_BASE_HPP_
#define GEOMETRY_SOURCES_BASE_HPP_

namespace Source {
namespace detail {

/** Base class providing covariant clone method and delete */
template<typename Base,typename Derived>struct cloner : public Base {

	void deleteSelf()
	{
		delete this;
	}

	// provide covariant clone method
	virtual Base* cloneSelf() const { return new Derived(static_cast<Derived const&>(*this)); }

	// provide forwarding constructor
	template<typename... Args>explicit cloner(Args... args) : Base(args...){}

	// make sure destructor is virtual
	virtual ~cloner(){}
};
};



class Base
{

public:

	float 		power() 		const 	{ return m_power; 	}
	void 		power(float p)			{ m_power=p; 		}

	virtual Base*		cloneSelf() const=0;

protected:
	Base(float p) : m_power(p){}

private:
	float m_power=1.0f;
};

};


#endif /* GEOMETRY_SOURCES_BASE_HPP_ */
