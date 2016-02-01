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
	Point(SSE::Vector3 p) : m_pos(p){}

	template<class RNG>SSE::Vector3 position(RNG& rng) const
	{
		return m_pos;
	}

private:
	SSE::Vector3	m_pos;
};

};



#endif /* KERNELS_SOFTWARE_EMITTERS_POINT_HPP_ */
