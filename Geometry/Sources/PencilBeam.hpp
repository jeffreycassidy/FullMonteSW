/*
 * PencilBeam.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_PENCILBEAM_HPP_
#define GEOMETRY_SOURCES_PENCILBEAM_HPP_

#include "Base.hpp"
#include "Directed.hpp"
#include "PointSource.hpp"

#include <limits>

namespace Source
{

class PencilBeam : public Source::detail::cloner<Source::PointSource,Source::PencilBeam>, public detail::Directed
{
public:
	PencilBeam() :
		cloner(1.0f,std::array<float,3>{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN()},-1U),
		Directed(std::array<float,3>{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN()})
		{}

	PencilBeam(float w,std::array<float,3> pos,std::array<float,3> dir,unsigned elementHint=-1U) :
		cloner(w,pos,elementHint),
		Directed(dir)
		{ }

	/// Nonzero radius means a top-hat beam
	float 		radius() 			const	{ return m_radius; 	}
	void 		radius(float r)				{ m_radius=r; 		}

private:
	unsigned 	m_faceHint=-1U;
	float 		m_radius=0.0f;
};

};


#endif /* GEOMETRY_SOURCES_PENCILBEAM_HPP_ */
