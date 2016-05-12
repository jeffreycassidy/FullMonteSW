/*
 * TetraMeshFilter.hpp
 *
 *  Created on: Mar 16, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_FILTERS_TETRAMESHFILTER_HPP_
#define GEOMETRY_FILTERS_TETRAMESHFILTER_HPP_

class TetraMesh;

#include <FullMonte/Geometry/Filters/FilterBase.hpp>

class TetraMeshFilter : public FilterBase<unsigned>
{
public:
	TetraMeshFilter(const TetraMesh* m);
    virtual ~TetraMeshFilter(){}

	void mesh(const TetraMesh* m);			///< Set associated mesh and set to include-all
	const TetraMesh* mesh() const;			///< Get associated mesh

private:
	virtual void postMeshUpdate(){}

	const TetraMesh* 	m_mesh=nullptr;
};




#endif /* GEOMETRY_FILTERS_TETRAMESHFILTER_HPP_ */
