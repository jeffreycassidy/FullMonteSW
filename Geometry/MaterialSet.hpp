/*
 * MaterialSet.hpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_MATERIALSET_HPP_
#define GEOMETRY_MATERIALSET_HPP_

#include <vector>

class Material;


class MaterialSet
{
public:
	/// Create a new set of materials from scratch (empty)
	MaterialSet();

	/// Create a new material set, copying all materials from the original
	MaterialSet*	clone() const;


	/// Alias for material(0) which represents the exterior of the mesh
	Material*		exterior();

	Material*		material(unsigned i);

private:
	std::vector<Material*>		m_materials;
};

#endif /* GEOMETRY_MATERIALSET_HPP_ */
