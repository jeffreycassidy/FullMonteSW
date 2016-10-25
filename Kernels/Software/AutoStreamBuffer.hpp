/*
 * LineBufferStream.hpp
 *
 *  Created on: Oct 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_AUTOSTREAMBUFFER_HPP_
#define KERNELS_SOFTWARE_AUTOSTREAMBUFFER_HPP_

#include <iostream>
#include <sstream>

class AutoStreamBuffer;
template<class T>AutoStreamBuffer& operator<<(AutoStreamBuffer& sb,const T& v);

/** AutoStreamBuffer
 *
 * Buffers output to a stringstream, then writes it when the object is destructed.
 * Needed to get sensible output from multi-threaded code using stream inserters (eg. cout << a << ' ' << b << ...)
 */

class AutoStreamBuffer
{
public:
	AutoStreamBuffer(std::ostream& os) : m_os(os){}
	~AutoStreamBuffer()
	{
		m_os << m_ss.str();
	}

private:
	std::ostream&		m_os;
	std::stringstream	m_ss;

	template<class T>friend AutoStreamBuffer& operator<<(AutoStreamBuffer&,const T&);
};

template<class T>AutoStreamBuffer& operator<<(AutoStreamBuffer& sb,const T& v)
{
	sb.m_ss << v;
	return sb;
}

#endif /* KERNELS_SOFTWARE_AUTOSTREAMBUFFER_HPP_ */
