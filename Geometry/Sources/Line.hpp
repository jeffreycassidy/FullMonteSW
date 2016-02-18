/*
 * Line.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_LINE_HPP_
#define GEOMETRY_SOURCES_LINE_HPP_

#include "Base.hpp"

#include <array>
#include <cmath>

namespace Source {

class Line : public Source::detail::cloner<Base,Line>
{

public:
	explicit Line(float w=1.0) : cloner(w){}
	Line(float w,std::array<float,3> ep0,std::array<float,3> ep1) : cloner(w), m_endpoint{ep0,ep1}{}

	std::array<float,3>			endpoint(unsigned i)						const	{ return m_endpoint[i]; }
	void						endpoint(unsigned i,std::array<float,3> p)			{ m_endpoint[i]=p;		}

	float length() const
	{
		std::array<float,3> v{{ m_endpoint[1][0]-m_endpoint[0][0], m_endpoint[1][1]-m_endpoint[0][1], m_endpoint[1][2]-m_endpoint[0][2] }};
		return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	}

private:
	std::array<float,3>		m_endpoint[2];
};

};

#endif /* GEOMETRY_SOURCES_LINE_HPP_ */
