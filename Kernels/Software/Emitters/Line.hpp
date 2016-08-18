/*
 * Line.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_LINE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_LINE_HPP_

#include <FullMonteSW/Geometry/StandardArrayKernel.hpp>

#include <iostream>
#include <vector>

namespace Emitter
{

/** Line source emitter (uniform emission along the length) */

template<class RNG>class Line
{
public:
	/// Elements are a list of (segment-end-distance, tetID)
	Line(SSE::Point3 p0,SSE::Point3 p1,const std::vector<std::pair<float,unsigned>>& elements) :
		m_origin(p0),
		m_displacement(p1-p0),
		m_elements(elements)
	{
		// normalize element distances by length to put into [0,1)
		float l = float(norm(m_displacement));
		for(auto& e : m_elements)
			e.first /= l;
	}

	/// Generate the position
	std::pair<unsigned,SSE::Point3> position(RNG& rng) const
	{
		float d = *rng.floatU01();
		return std::make_pair(tetra(d),m_origin + m_displacement*SSE::Scalar(d));
	}

	/// Determine which tetra emits the photon for dimensionless w [0,1)
	unsigned		tetra(float w) const;

	/// Query source properties
	SSE::Point3		origin()		const { return m_origin; 				}
	SSE::Vector3 	displacement() 	const { return m_displacement; 			}
	float			length()		const { return norm(m_displacement); 	}

	bool static compareLength(const std::pair<float,unsigned>& p) { return p.first; }

private:
	SSE::Point3		m_origin;				///< Origin of line segment
	SSE::Vector3	m_displacement;			///< Displacement of end point (m_origin + m_displacement = endpoint)

	std::vector<std::pair<float,unsigned>>	m_elements;
};

template<class RNG>unsigned Line<RNG>::tetra(float d) const
{
	if (d < m_elements.front().first)
		return m_elements.front().second;

	const auto it = boost::lower_bound(m_elements, d, [](const std::pair<float,unsigned>& p,float d){ return p.first < d;  });
	if (it != m_elements.end())
		return it->second;
	else
		std::cout << "WARNING: Emitting from a position (d=" << d << ") beyond the end of a line source of length " << float(norm(m_displacement)) << std::endl;
	return m_elements.back().second;
}

}



#endif /* KERNELS_SOFTWARE_EMITTERS_LINE_HPP_ */
