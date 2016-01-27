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

namespace Source
{

class PencilBeam : public Source::detail::cloner<Source::PointSource,Source::PencilBeam>, public detail::Directed
{
public:
	PencilBeam(float w,std::array<float,3> pos,std::array<float,3> dir,unsigned elementHint=-1U) :
		cloner(w,pos,elementHint),
		Directed(dir),
		m_boundarySurfaceHint(-1U){ }

	unsigned				boundaryHint()						const	{ return m_boundarySurfaceHint; 	}
	void					boundaryHint(unsigned b)					{ m_boundarySurfaceHint=b;			}

private:
	unsigned m_boundarySurfaceHint=-1U;
};

};


#endif /* GEOMETRY_SOURCES_PENCILBEAM_HPP_ */
