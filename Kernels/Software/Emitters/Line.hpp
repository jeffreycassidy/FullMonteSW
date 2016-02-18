/*
 * Line.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_LINE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_LINE_HPP_

namespace Emitter
{

/** Line source emitter (uniform emission along the length) */

template<class RNG>class Line
{
public:
	Line(SSE::Point3 p0,SSE::Point3 p1) :
		m_origin(p0),
		m_displacement(p1-p0){}

	SSE::Point3 position(RNG& rng) const
	{
		return m_origin + m_displacement*SSE::Scalar(*rng.floatU01());
	}

private:
	SSE::Point3		m_origin;				///< Origin of line segment
	SSE::Vector3	m_displacement;			///< Displacement of end point (m_origin + m_displacement = endpoint)
};

}



#endif /* KERNELS_SOFTWARE_EMITTERS_LINE_HPP_ */
