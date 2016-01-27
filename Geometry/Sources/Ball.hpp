/*
 * Ball.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_BALL_HPP_
#define GEOMETRY_SOURCES_BALL_HPP_

#include <array>
#include "PointSource.hpp"

namespace Source
{

class Ball : public Source::detail::cloner<Source::PointSource,Ball>
{
public:
	Ball(float w,std::array<float,3> pos,float r) :
		cloner(w,pos),
		m_radius(r){}

	std::array<float,3>		centre() 						const 	{ return pos(); 	}
	void					centre(std::array<float,3> p)			{ pos(p); 			}

	float 					radius()						const 	{ return m_radius; 	}
	void 					radius(float r)							{ m_radius=r; 		}

private:
	float m_radius=0.0f;
};

};


#endif /* GEOMETRY_SOURCES_BALL_HPP_ */
