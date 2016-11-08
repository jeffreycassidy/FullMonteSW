/*
 * PencilBeam.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_PENCILBEAM_HPP_
#define GEOMETRY_SOURCES_PENCILBEAM_HPP_

#include "Abstract.hpp"
#include "Directed.hpp"
#include "Point.hpp"

#include <limits>

namespace Source
{

class PencilBeam : public Point, public detail::Directed
{
public:
#ifdef SWIG
	PencilBeam() :
		Point(1.0f,std::array<float,3>{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN()},-1U),
		Directed(std::array<float,3>{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN()})
		{}
#else
	PencilBeam(){}
#endif

	PencilBeam(float w,std::array<float,3> pos,std::array<float,3> dir,unsigned elementHint=-1U) :
		Point(w,pos,elementHint),
		Directed(dir)
		{ }

	DERIVED_SOURCE_MACRO(Abstract,PencilBeam)

	/// Nonzero radius means a top-hat beam
	float 		radius() 			const	{ return m_radius; 	}
	void 		radius(float r)				{ m_radius=r; 		}

private:
	unsigned 	m_faceHint=-1U;
	float 		m_radius=0.0f;
};

};


#endif /* GEOMETRY_SOURCES_PENCILBEAM_HPP_ */
