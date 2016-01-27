/*
 * Point.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_POINT_HPP_
#define GEOMETRY_SOURCES_POINT_HPP_

#include "Base.hpp"
#include <array>

namespace Source
{

class Point : public Source::detail::cloner<Base,Point>
{
public:
	Point(float w,std::array<float,3> p,unsigned regionHint=-1U) :
		cloner(w),
		m_pos(p),
		m_regionHint(regionHint){}

	std::array<float,3> position() 						const	{ return m_pos; }
	void 				position(std::array<float,3> p)			{ m_pos=p; }

	unsigned			regionHint()					const 	{ return m_regionHint; 	}
	void				regionHint(unsigned r)					{ m_regionHint=r;		}

private:
	std::array<float,3>		m_pos;
	unsigned				m_regionHint=-1U;
};

};


#endif /* GEOMETRY_SOURCES_POINT_HPP_ */
