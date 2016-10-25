/*
 * SurfaceTri.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_SURFACETRI_HPP_
#define GEOMETRY_SOURCES_SURFACETRI_HPP_

#include "Surface.hpp"
#include <array>

namespace Source {

/** Source specified as a triangle of three point IDs */

class SurfaceTri : public Surface
{
public:
	SurfaceTri(float w,std::array<unsigned,3> triPointIDs) :
		Surface(w,-1U),
		m_triPointIDs(triPointIDs){}

	DERIVED_SOURCE_MACRO(Abstract,SurfaceTri);

	void 					triPointIDs(std::array<unsigned,3> ids)			{ m_triPointIDs=ids; 	}
	std::array<unsigned,3>	triPointIDs()							const	{ return m_triPointIDs; }

private:
	std::array<unsigned,3>		m_triPointIDs{{0U,0U,0U}};
};

};




#endif /* GEOMETRY_SOURCES_SURFACETRI_HPP_ */
