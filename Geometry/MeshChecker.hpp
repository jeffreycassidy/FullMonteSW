/*
 * MeshChecker.hpp
 *
 *  Created on: Apr 4, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_MESHCHECKER_HPP_
#define GEOMETRY_MESHCHECKER_HPP_

struct Tetra;

class MeshChecker
{
public:

	MeshChecker(const TetraMesh* M){ mesh(M); }

	void mesh(const TetraMesh* M){ m_mesh = M; }

	/// Check that tetra (face-normal definition) is closed, ie. all pairs of normals have negative dot product
	bool dihedrals(unsigned IDt) const;

	/// Check that point orientations are correct (height of each point over opposite face is positive)
	bool pointHeights(unsigned IDt) const;

private:
	const TetraMesh* m_mesh=nullptr;

	float m_pointHeightTolerance=2e-5f;
};




#endif /* GEOMETRY_MESHCHECKER_HPP_ */
