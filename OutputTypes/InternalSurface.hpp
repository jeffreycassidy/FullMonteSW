/*
 * InternalSurface.hpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_INTERNALSURFACE_HPP_
#define OUTPUTTYPES_INTERNALSURFACE_HPP_

#include <boost/serialization/access.hpp>

template<typename T>class InternalSurface
{
public:
	InternalSurface(){}
	InternalSurface(T exit_,T enter_) : exit(exit_),enter(enter_){}
	T exit=0.0f;
	T enter=0.0f;

	T bidirectional() const { return exit+enter; }

	InternalSurface& operator+=(const InternalSurface rhs)
	{
		exit += rhs.exit;
		enter += rhs.enter;
		return *this;
	}

private:
	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ ar & BOOST_SERIALIZATION_NVP(exit) & BOOST_SERIALIZATION_NVP(enter); }
	friend boost::serialization::access;
};

inline bool nonzero(float i){ return i != 0.0; }
template<typename T>inline bool nonzero(const InternalSurface<T>& S)
	{ return nonzero(S.exit) || nonzero(S.enter); }

inline InternalSurface<float> flip(const InternalSurface<float> i)
{
	InternalSurface<float> o = i;
	std::swap(o.enter,o.exit);
	return o;
}


#endif /* OUTPUTTYPES_INTERNALSURFACE_HPP_ */
