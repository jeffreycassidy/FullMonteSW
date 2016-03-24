/*
 * DirectionalSurface.hpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_DIRECTIONALSURFACE_HPP_
#define OUTPUTTYPES_DIRECTIONALSURFACE_HPP_

#include <FullMonte/Geometry/TetraMesh.hpp>

#include "InternalSurface.hpp"
#include "SpatialMapBase.hpp"
#include "FluenceMapBase.hpp"

/** Returns the directional surface fluence (entering/exiting) from a region specified by tetra inclusion criteria (true->included).
 * For two tetras Ta, Tb joined by face F_ab with normal pointing into Ta, if pred(T_a) && !pred(T_b) then we are exiting the face F_ab.
 *
 * Since TetraMesh faces have their enter/exit directions arbitrarily oriented, the filter needs to figure out the correct orientation
 * with respect to the region defined.
 *
 * The face orientations can be queried with faces() and facesU(). Where faces() value is negative, enter/exit need to be inverted.
 */

enum FaceDirection { Enter, Exit, Bidirectional };

class DirectionalSurface
{
public:
	DirectionalSurface();

	/// Get/set the TetraMesh (requires face info, so no TetraMeshBase)
	const TetraMesh*					mesh()										const	{ return m_mesh; 	}
	void								mesh(const TetraMesh* m)							{ m_mesh=m;			}

	/// Get/set the filter operating on tetra IDs
	FilterBase<unsigned>*				tetraFilter()								const 	{ return m_filter; 	}
	void								tetraFilter(FilterBase<unsigned>* f)				{ m_filter=f; 		}

	/// Get/set the fluence data
	void 								data(const InternalSurfaceFluenceMap* m)			{ m_data = m;		}
	const InternalSurfaceFluenceMap*	data()										const 	{ return m_data; 	}


	/// Updates the face indices from the filter
	void 								update();

	/// Pulls the specified fluence direction
	SurfaceFluenceMap					result(FaceDirection dir) const;

	/// Pulls the bidirectional fluence
	InternalSurfaceFluenceMap			result() const;

	/// Check which faces are included
	vector<int>							faces() const;

	/// Check which faces are included (return unsigned, ie. |IDf|)
	vector<unsigned>					facesU() const;


private:
	const InternalSurfaceFluenceMap*m_data=nullptr;			///< Fluence data
	const TetraMesh*				m_mesh=nullptr;			///< The mesh (required for topology)
	FilterBase<unsigned>*			m_filter=nullptr;		///< The tetra filter

	std::vector<int>				m_faces;				///< The face list (<0 -> need to invert enter/exit)
};

#endif /* OUTPUTTYPES_DIRECTIONALSURFACE_HPP_ */
