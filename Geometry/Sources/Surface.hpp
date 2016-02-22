/*
 * Surface.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_SURFACE_HPP_
#define GEOMETRY_SOURCES_SURFACE_HPP_

#include "Base.hpp"

namespace Source {

/** Source specified as a surface element */

class Surface : public Source::detail::cloner<Base,Surface> {
public:
	Surface(float w,unsigned surfaceElement) :
		cloner(w),
		m_surfaceElement(surfaceElement){}

	unsigned 	surfaceID()				const	{ return m_surfaceElement; 	}
	void 		surfaceID(unsigned s)			{ m_surfaceElement=s; 		}

private:
	unsigned	m_surfaceElement=-1U;
};

};

#endif /* GEOMETRY_SOURCES_SURFACE_HPP_ */
