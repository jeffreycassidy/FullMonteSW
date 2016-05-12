/*
 * TetraMeshFilterBase.hpp
 *
 *  Created on: Feb 25, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_FILTERS_TETRAMESHBASEFILTER_HPP_
#define GEOMETRY_FILTERS_TETRAMESHBASEFILTER_HPP_

class TetraMeshBase;

class TetraMeshBaseFilter
{
public:
	explicit TetraMeshBaseFilter(const TetraMeshBase* m=nullptr);
    virtual ~TetraMeshBaseFilter(){}

	void mesh(const TetraMeshBase* m);			///< Set associated mesh and set to include-all
	const TetraMeshBase* mesh() const;			///< Get associated mesh

private:
	virtual void postMeshUpdate(){}

	const TetraMeshBase* 	m_mesh=nullptr;
};



#endif /* GEOMETRY_FILTERS_TETRAMESHBASEFILTER_HPP_ */
