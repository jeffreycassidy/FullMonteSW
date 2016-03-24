/*
 * nan_wrapper.hpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_NAN_WRAPPER_HPP_
#define OUTPUTTYPES_NAN_WRAPPER_HPP_

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/access.hpp>

template<typename T>struct nan_wrapper
{
	nan_wrapper(T& x,T defaultValue,const char *s=nullptr) : m_reference(x),m_defaultValue(defaultValue),m_key(s) {}
	operator T() const { return isnan(m_reference) ? m_defaultValue : m_reference; }

	nan_wrapper& operator=(T i){ m_reference = (i==m_defaultValue) ? std::numeric_limits<T>::quiet_NaN() : i; return *this; }

private:
	T& m_reference;
	T m_defaultValue;
	const char* m_key;

	template<class Archive>void load(Archive& ar,const unsigned ver)
	{
		T t;
		ar & boost::serialization::make_nvp(m_key,t);
		operator=(t);
	}
	template<class Archive>void save(Archive& ar,const unsigned ver) const
	{
		T t = *this;
		ar & boost::serialization::make_nvp(m_key,t);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

	friend class boost::serialization::access;

};




#endif /* OUTPUTTYPES_NAN_WRAPPER_HPP_ */
