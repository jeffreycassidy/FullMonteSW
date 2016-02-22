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

class SurfaceTri : public Source::detail::cloner<Surface,SurfaceTri> {
public:
	SurfaceTri(float w,std::array<unsigned,3> triPointIDs) :
		cloner(w,-1U),
		m_triPointIDs(triPointIDs){}

	void 					triPointIDs(std::array<unsigned,3> ids)			{ m_triPointIDs=ids; 	}
	std::array<unsigned,3>	triPointIDs()							const	{ return m_triPointIDs; }

private:
	std::array<unsigned,3>		m_triPointIDs{{0U,0U,0U}};
};

};




#endif /* GEOMETRY_SOURCES_SURFACETRI_HPP_ */
