/*
 * Directed.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_DIRECTED_HPP_
#define GEOMETRY_SOURCES_DIRECTED_HPP_

#include <array>

namespace Source
{
namespace detail
{

/** Mix-in class for directed sources */

class Directed
{
public:
	Directed(){}
	Directed(std::array<float,3> dir) : m_dir(dir){}

	std::array<float,3>		direction() 						const	{ return m_dir; }
	void					direction(std::array<float,3> dir)			{ m_dir=dir; 	}

private:
	std::array<float,3>		m_dir{{0.0f,0.0f,0.0f}};
};

};
};



#endif /* GEOMETRY_SOURCES_DIRECTED_HPP_ */
