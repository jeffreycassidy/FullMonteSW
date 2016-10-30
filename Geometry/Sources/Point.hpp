/*
 * Point.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_POINT_HPP_
#define GEOMETRY_SOURCES_POINT_HPP_

#include "Abstract.hpp"
#include <array>

namespace Source
{

class Point : public Abstract
{
public:
	Point(float w=1.0,std::array<float,3> p={{0.0,0.0,0.0}},unsigned elementHint=-1U) :
		Abstract(w),
		m_pos(p),
		m_elementHint(elementHint){}

	DERIVED_SOURCE_MACRO(Abstract,Point)

	std::array<float,3> position() 						const	{ return m_pos; }
	void 				position(std::array<float,3> p)			{ m_pos=p; }

	unsigned			elementHint()					const 	{ return m_elementHint; 	}
	void				elementHint(unsigned r)					{ m_elementHint=r;		}

private:
	std::array<float,3>		m_pos;
	unsigned				m_elementHint=-1U;
};

};


#endif /* GEOMETRY_SOURCES_POINT_HPP_ */
