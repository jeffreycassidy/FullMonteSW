/*
 * Ball.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_BALL_HPP_
#define GEOMETRY_SOURCES_BALL_HPP_

#include <array>
#include "Point.hpp"

namespace Source
{

class Ball :
		public Point
{
public:
	Ball(){}
	Ball(float w,std::array<float,3> pos,float r) :
		Point(w,pos),
		m_radius(r){}

	DERIVED_SOURCE_MACRO(Abstract,Ball)

	std::array<float,3>		centre() 						const 	{ return position(); 	}
	void					centre(std::array<float,3> p)			{ position(p); 			}

	float 					radius()						const 	{ return m_radius; 	}
	void 					radius(float r)							{ m_radius=r; 		}

private:
	float m_radius=0.0f;
};

};


#endif /* GEOMETRY_SOURCES_BALL_HPP_ */
