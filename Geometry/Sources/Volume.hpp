/*
 * Volume.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_VOLUME_HPP_
#define GEOMETRY_SOURCES_VOLUME_HPP_

#include "Abstract.hpp"

namespace Source {

/** Source launching from within the volume of a single element, specified by ID */

class Volume : public Abstract
{
public:
	Volume(float w,unsigned elementID) : Abstract(w),m_element(elementID){}

	DERIVED_SOURCE_MACRO(Abstract,Volume)

	unsigned 	elementID()				const	{ return m_element; }
	void 		elementID(unsigned e)			{ m_element=e; 		}

private:
	unsigned m_element=-1U;
};

};

#endif /* GEOMETRY_SOURCES_VOLUME_HPP_ */
