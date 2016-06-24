/*
 * Point.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_POINT_HPP_
#define KERNELS_SOFTWARE_EMITTERS_POINT_HPP_

#include "Base.hpp"

namespace Emitter
{

/** Point source emitter */

class Point
{
public:
	Point(unsigned element,SSE::Vector3 p) : m_pos(p),m_element(element){}

	template<class RNG>std::pair<unsigned,SSE::Point3> position(RNG& rng) const
	{
		return std::make_pair(m_element,m_pos);
	}

private:
	SSE::Vector3	m_pos;				///< The position
	unsigned 		m_element=-1U;
};

};



#endif /* KERNELS_SOFTWARE_EMITTERS_POINT_HPP_ */
