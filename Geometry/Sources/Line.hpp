/*
 * Line.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_LINE_HPP_
#define GEOMETRY_SOURCES_LINE_HPP_

#include "Abstract.hpp"

#include <FullMonteSW/Geometry/StandardArrayKernel.hpp>

#include <array>
#include <cmath>

namespace Source {

class Line : public Abstract
{

public:
	enum Pattern { Isotropic, Normal };

	explicit Line(float w=1.0) : Abstract(w)
		{}

	Line(float w,std::array<float,3> ep0,std::array<float,3> ep1)
#ifndef SWIG
		:
		Abstract(w),
		m_endpoint{ep0,ep1}{}
#else		// SWIG doesn't like C++11 features like brace initializers
		;
#endif

	DERIVED_SOURCE_MACRO(Abstract,Line)

	std::array<float,3>			endpoint(unsigned i)						const	{ return m_endpoint[i]; }
	void						endpoint(unsigned i,std::array<float,3> p)			{ m_endpoint[i]=p;		}

	std::array<float,3>			direction() const
	{
		return normalize(m_endpoint[1]-m_endpoint[0]);
	}

	float length() const
	{
		std::array<float,3> v{{ m_endpoint[1][0]-m_endpoint[0][0], m_endpoint[1][1]-m_endpoint[0][1], m_endpoint[1][2]-m_endpoint[0][2] }};
		return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	}

	void 	pattern(Pattern p)			{ m_pattern=p; }
	Pattern pattern() 			const 	{ return m_pattern; }

private:
	std::array<float,3>		m_endpoint[2];
	Pattern					m_pattern=Isotropic;
};

};

#endif /* GEOMETRY_SOURCES_LINE_HPP_ */
