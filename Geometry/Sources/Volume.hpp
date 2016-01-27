/*
 * Volume.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_VOLUME_HPP_
#define GEOMETRY_SOURCES_VOLUME_HPP_

#include "Base.hpp"

namespace Source {

class Volume : public Source::detail::cloner<Base,Volume>
{
public:
	Volume(float w,unsigned elementID) : cloner(w),m_element(elementID){}

	unsigned 	elementID()				const	{ return m_element; }
	void 		elementID(unsigned e)			{ m_element=e; 		}

private:
	unsigned m_element=-1U;
};

};

#endif /* GEOMETRY_SOURCES_VOLUME_HPP_ */
