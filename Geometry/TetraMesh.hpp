#ifndef TETRAMESH_HPP_INCLUDED_
#define TETRAMESH_HPP_INCLUDED_

#ifndef SWIG
#include <vector>
#include <unordered_map>

#include "Face.hpp"
#include "Tetra.hpp"
#include "TetraMeshBase.hpp"

#include <boost/functional/hash.hpp>

#include "newgeom.hpp"

template<typename T>class FilterBase;

/** In lexicographical order, gives the point indices of all four faces */

constexpr std::array<std::array<unsigned char,3>,4> tetra_face_indices {
	std::array<unsigned char,3>{{ 0, 1, 2 }},
	std::array<unsigned char,3>{{ 0, 1, 3 }},
	std::array<unsigned char,3>{{ 0, 2, 3 }},
	std::array<unsigned char,3>{{ 1, 2, 3 }}
};



/** In lexicographical order, gives the point indices of all four faces, and the point opposite them */

struct face_opposite {
	std::array<unsigned char,3> faceidx;
	unsigned oppidx;
};

constexpr std::array<face_opposite,4> tetra_face_opposite_point_indices {
	face_opposite{std::array<unsigned char,3>{{ 0, 1, 2 }},3},
	face_opposite{std::array<unsigned char,3>{{ 0, 1, 3 }},2},
	face_opposite{std::array<unsigned char,3>{{ 0, 2, 3 }},1},
	face_opposite{std::array<unsigned char,3>{{ 1, 2, 3 }},0}
};



/** In lexicographical order, gives the point indices of all six edge pairs of a tetra */

constexpr std::array<std::array<unsigned char,2>,6> tetra_edge_indices {
	std::array<unsigned char,2>{{ 0, 1}},
	std::array<unsigned char,2>{{ 0, 2}},
	std::array<unsigned char,2>{{ 0, 3}},
	std::array<unsigned char,2>{{ 1, 2}},
	std::array<unsigned char,2>{{ 1, 3}},
	std::array<unsigned char,2>{{ 2, 3}}
};

using namespace std;

#endif // SWIG



/** Holds a tetrahedral mesh including face and tetra information.
 *
 */

class TetraMesh : public TetraMeshBase
{
public:
	TetraMesh(){};
	TetraMesh(const TetraMeshBase& Mb) : TetraMeshBase(Mb)
		{ buildTetrasAndFaces(); }

	TetraMesh(TetraMeshBase&& Mb) : TetraMeshBase(Mb)
		{ buildTetrasAndFaces(); }

	TetraMesh(const vector<Point<3,double> >& P_,const vector<TetraByPointID>& T_p_,const vector<unsigned>& T_m_)
	: TetraMeshBase(P_,T_p_,T_m_) { buildTetrasAndFaces(); }

	TetraMesh(const vector<std::array<float,3>>& P_,const vector<TetraByPointID>& T_p_,const vector<unsigned>& T_m_)
	{
		std::vector<Point<3,double>> Pd(P_.size());
		std::vector<TetraByPointID> T_p(T_p_.size());

		for(unsigned i=0;i<Pd.size();++i)
			Pd[i] = Point<3,double>{ P_[i][0], P_[i][1], P_[i][2] };

		*this = TetraMesh(TetraMeshBase(Pd,T_p_,T_m_));
	}

	~TetraMesh();
	virtual void Delete(){ delete this; }

    vector<unsigned> facesBoundingRegion(unsigned i) const;

	// query size of mesh
	unsigned getNf() const { return m_faces.size()-1; }

	// Accessors for various point/face constructs
    Face                    getFace(int id)                 const { Face f = m_faces[abs(id)]; if(id<0){ f.flip(); } return f; }
    const FaceByPointID&    getFacePointIDs(unsigned id)    const { assert(id < m_facePoints.size()); return m_facePoints[id]; }
	const TetraByFaceID&    getTetraByFaceID(unsigned id)   const { return m_tetraFaces[id]; }
    unsigned                getTetraFromFace(int IDf)       const;

    unsigned                getTetraID(TetraByPointID IDt) const {
        auto it = m_pointIDsToTetraMap.find(IDt);
        return (it == m_pointIDsToTetraMap.end() ? 0 : it->second);
    }

    Tetra                   getTetra(unsigned IDt) const { return m_tetras[IDt]; }

    double                  getFaceArea(const FaceByPointID&)   const;
    double                  getFaceArea(int IDf)                const { return getFaceArea(m_facePoints[abs(IDf)]); }
    double                  getFaceArea(unsigned IDf)           const { return getFaceArea(m_facePoints[IDf]); }

	// find nearest point or enclosing tetra
	unsigned findEnclosingTetra(const Point<3,double>&) const;
	unsigned findNearestPoint  (const Point<3,double>&) const;

    // checks if a point is within a given tetra by expressing as a linear combination of the corner points
    bool isWithinByPoints(int,const Point<3,double>&) const;

	// checks if faces are oriented correctly
	bool checkFaces() const;

	std::vector<unsigned> tetras_close_to(Point<3,double> p0,float) const;

	bool faceBoundsRegion(unsigned region,int IDf) const
	{
		array<unsigned,2> p = m_faceTetras[abs(IDf)];		// Get the two incident m_tetras
		unsigned r0 = m_tetraMaterials[p[0]], r1 = m_tetraMaterials[p[1]];				// check material types
		return r0!=r1 && (r1==region || r0==region);
	}

	void setFacesForFluenceCounting(const FilterBase<int>* TF);		/// Predicate specifying if a given face should have its fluence counted

    std::vector<unsigned> getRegionBoundaryTris(unsigned r) const;

    /// Extracts the face IDs and tetra IDs corresponding to a given surface
    vector<pair<unsigned,unsigned>> getRegionBoundaryTrisAndTetras(unsigned r0,unsigned r1=-1U) const;

    // returns the intersection result and face ID entered (res, IDf) for the next face crossed by ray (p,dir)
    std::tuple<PointIntersectionResult,int> findNextFaceAlongRay(Point<3,double> p,UnitVector<3,double> dir,int IDf_exclude=0) const;

private:

    void printTetra(unsigned IDt) const;		// Debug method

	vector<TetraByFaceID>	    	m_tetraFaces;       // tetra -> 4 face IDs
    vector<FaceByPointID>       	m_facePoints;       // face ID -> 3 point IDs
	vector<Face>			    	m_faces;          	// faces (with normals and constants)

	/** For each positive face ID f, m_faceTetras[f] gives the tetra IDs adjacent to the face.
	 * The normal of m_face[f] points into tetra m_faceTetras[f][0] and out of m_faceTetras[f][1].
	 */
	vector<std::array<unsigned,2>>	m_faceTetras;

	/** Packed and aligned tetra representation for the kernel, holding face normals, constants, adjacent tetras,
	 * bounding faces, material ID, and face flags (for logging).
	 */
    vector<Tetra>               	m_tetras;

    std::unordered_map<TetraByPointID,unsigned,boost::hash<std::array<unsigned,4>>> m_pointIDsToTetraMap;
    std::unordered_map<FaceByPointID, unsigned,boost::hash<std::array<unsigned,3>>> m_pointIDsToFaceMap;

    /// Build tetras and faces from the point & connectivity lists
	void buildTetrasAndFaces();

	/// Build the kernel tetra representation from the point, connectivity, and face lists
	void makeKernelTetras();

    template<class Archive>void serialize(Archive& ar,const unsigned ver)
    {
    	ar & boost::serialization::base_object<TetraMeshBase>(*this);

    	if(Archive::is_loading::value)		// reconstruct from the TetraMeshBase
    		buildTetrasAndFaces();
    }

    friend boost::serialization::access;
};

#endif
