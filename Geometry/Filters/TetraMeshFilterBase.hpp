/*
 * TetraMeshFilter.hpp
 *
 *  Created on: Mar 16, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_FILTERS_TETRAMESHFILTERBASE_HPP_
#define GEOMETRY_FILTERS_TETRAMESHFILTERBASE_HPP_

class TetraMesh;

class TetraMeshFilterBase
{
public:
	TetraMeshFilterBase(const TetraMesh* m=nullptr);
    virtual ~TetraMeshFilterBase();

	void mesh(const TetraMesh* m);			///< Set associated mesh and set to include-all
	const TetraMesh* mesh() const;			///< Get associated mesh

	void updateMesh();

private:
	virtual void postMeshUpdate();

	const TetraMesh* 	m_mesh=nullptr;
};

#endif /* GEOMETRY_FILTERS_TETRAMESHFILTERBASE_HPP_ */
