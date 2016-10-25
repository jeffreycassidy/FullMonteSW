/*
 * Surface.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_SURFACE_HPP_
#define GEOMETRY_SOURCES_SURFACE_HPP_

#include "Abstract.hpp"

namespace Source {

/** Source specified as a surface element */

class Surface : public Abstract
{
public:
	Surface(float w=1.0f,unsigned surfaceElement=-1U) :
		Abstract(w),
		m_surfaceElement(surfaceElement){}

	DERIVED_SOURCE_MACRO(Abstract,Surface)

	unsigned 	surfaceID()				const	{ return m_surfaceElement; 	}
	void 		surfaceID(unsigned s)			{ m_surfaceElement=s; 		}

private:
	unsigned	m_surfaceElement=-1U;
};

};

#endif /* GEOMETRY_SOURCES_SURFACE_HPP_ */
