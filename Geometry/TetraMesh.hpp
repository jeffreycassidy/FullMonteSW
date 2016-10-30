#ifndef TETRAMESH_HPP_INCLUDED_
#define TETRAMESH_HPP_INCLUDED_

#ifndef SWIG
#include <vector>
#include <unordered_map>

#include "Face.hpp"
#include "Tetra.hpp"
#include "TetraMeshBase.hpp"

#include <boost/functional/hash.hpp>

#include <boost/iterator/counting_iterator.hpp>

#include <iterator>

#include <boost/timer/timer.hpp>

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


struct RTIntersection
{
	std::array<float,3>		q;
	float					d;
	int						IDf;
	unsigned				IDt;
};



/** Holds a tetrahedral mesh including face and tetra information.
 *
 */

#ifdef SWIG
%nodefaultctor TetraMesh;
#endif


typedef struct {} point_above_face_tag;
typedef struct {} point_below_face_tag;
typedef struct {} tetra_above_face_tag;
typedef struct {} tetra_below_face_tag;
typedef struct {} face_tag;
typedef struct {} faces_tag;

class TetraMesh : public TetraMeshBase
{
public:
	struct FaceHint
		{
			FaceByPointID fIDps;
			array<unsigned,2> IDts;
		};

	TetraMesh(){}
	TetraMesh(const TetraMeshBase& Mb,const std::vector<FaceHint>& H=std::vector<FaceHint>()) : TetraMeshBase(Mb)
		{ buildTetrasAndFaces(H); }

#ifndef SWIG
	TetraMesh(TetraMeshBase&& Mb,const std::vector<FaceHint>& H=std::vector<FaceHint>()) : TetraMeshBase(Mb)
		{ buildTetrasAndFaces(H); }
#endif


	TetraMesh(const vector<Point<3,double> >& P_,const vector<TetraByPointID>& T_p_,const vector<unsigned>& T_m_,const vector<FaceHint>& hint=vector<FaceHint>())
	: TetraMeshBase(P_,T_p_,T_m_) { buildTetrasAndFaces(hint); }

	TetraMesh(const vector<std::array<float,3>>& P_,const vector<TetraByPointID>& T_p_,const vector<unsigned>& T_m_,const vector<FaceHint>& hint=vector<FaceHint>())
	{
		std::vector<Point<3,double>> Pd(P_.size());
		std::vector<TetraByPointID> T_p(T_p_.size());

		for(unsigned i=0;i<Pd.size();++i)
			Pd[i] = Point<3,double>{ P_[i][0], P_[i][1], P_[i][2] };

		*this = TetraMesh(Pd,T_p_,T_m_,hint);
	}

	~TetraMesh();
	virtual void Delete(){ delete this; }

	// query size of mesh
	unsigned getNf() const { return m_faces.size()-1; }

	// Accessors for various point/face constructs
    Face                    getFace(int id)                 const { Face f = m_faces[abs(id)]; if(id<0){ f.flip(); } return f; }
    const FaceByPointID&    getFacePointIDs(unsigned id)    const { assert(id < m_facePoints.size()); return m_facePoints[id]; }

	const TetraByFaceID&    getTetraByFaceID(unsigned id)   const { return m_tetraFaces[id]; }
    unsigned                getTetraFromFace(int IDf)       const;

    Tetra                   getTetra(unsigned IDt) const { return m_tetras[IDt]; }

    double                  getFaceArea(const FaceByPointID&)   const;
    double                  getFaceArea(int IDf)                const { return getFaceArea(m_facePoints[abs(IDf)]); }
    double                  getFaceArea(unsigned IDf)           const { return getFaceArea(m_facePoints[IDf]); }

    bool faceChecksEnabled() const 	{ return m_enableFaceChecks; }
    void enableFaceChecks(bool st)	{ m_enableFaceChecks=st; }

	// find nearest point or enclosing tetra
	unsigned findEnclosingTetra(const Point<3,double>&) const;
	unsigned findNearestPoint  (const Point<3,double>&) const;

	/// Find the nearest surface face along a ray, with optional filtering
	RTIntersection findSurfaceFace(std::array<float,3> p,std::array<float,3> d,const FilterBase<int>* F=nullptr) const;

    // checks if a point is within a given tetra by expressing as a linear combination of the corner points
    bool isWithinByPoints(int,const Point<3,double>&) const;

	// checks if faces are oriented correctly
	bool checkFaces() const;

	void setFacesForFluenceCounting(const FilterBase<int>* TF);		/// Predicate specifying if a given face should have its fluence counted

    /// Extracts the face IDs and tetra IDs corresponding to a given surface
    vector<pair<unsigned,unsigned>> getRegionBoundaryTrisAndTetras(unsigned r0,unsigned r1=-1U) const;

    // returns the intersection result and face ID entered (res, IDf) for the next face crossed by ray (p,dir)
    std::tuple<PointIntersectionResult,int> findNextFaceAlongRay(Point<3,double> p,UnitVector<3,double> dir,int IDf_exclude=0) const;

    TetraMesh* self() { return this; }


    class 				DirectedFaceDescriptor;

    typedef boost::counting_iterator<
			DirectedFaceDescriptor,
			std::random_access_iterator_tag,
			std::ptrdiff_t> DirectedFaceIterator;

    typedef boost::iterator_range<DirectedFaceIterator> DirectedFaceRange;

    class 				FaceDescriptor;

    typedef boost::counting_iterator<
			FaceDescriptor,
			std::random_access_iterator_tag,
			std::ptrdiff_t> FaceIterator;

    typedef  boost::iterator_range<FaceIterator> FaceRange;


    TetraRange tetras() const;

    FaceRange faces() const;




private:
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


    /// Enable checking of the face construct after creation
    bool m_enableFaceChecks=false;

    /// If true, output lots of extra information as the kernel tetras are being constructed
    bool m_enableVerboseConstruction=false;

    /// Tolerance to allow when checking for correct face orientation; if dot(p,n)-C < -m_pointHeightTolerance then flag an issue
    double m_pointHeightTolerance=2e-5;

    std::unordered_map<FaceByPointID, unsigned,boost::hash<std::array<unsigned,3>>> m_pointIDsToFaceMap;


    /** For each tetra, create tetra <-> face mappings based solely on ID:
     *
     * 		m_pointIDsToFaceMap			3 points -> face
     * 		m_tetraFaces				tetra -> 4 faces
     * 		m_facePoints				face -> point ID
     * 		m_faceTetras				face -> 2 tetras
     */
    void mapTetrasToFaces();


    /** Fix orientation of tetras so they are all counterclockwise and in minimal lexicographical order
     *
     */

    void orientTetras();


    /** For each existing face, build the plane (normal/constant) representation. Orientation for face (A,B,C)
     * is right-handed (ie. normal is ABxAC)
     *
     */

	void createFaceNormals();


    /** Check that face orientation is correct (requires that face normals already be constructed)
     *
     */

    void orientFaces();



    /// Build tetras and faces from the point & connectivity lists
	void buildTetrasAndFaces(const std::vector<FaceHint>& hint=std::vector<FaceHint>());

	/// Build the kernel tetra representation from the point, connectivity, and face lists
	void makeKernelTetras();

    boost::timer::cpu_timer m_timer;

    //template<typename Result,typename Prop,typename...Args>friend Result get(Prop,const TetraMesh&,Args...);

    friend FaceByPointID 	get(points_tag,const TetraMesh& M,FaceDescriptor);

    friend TetraDescriptor	get(tetra_above_face_tag,const TetraMesh& M,DirectedFaceDescriptor);
    friend TetraDescriptor	get(tetra_below_face_tag,const TetraMesh& M,DirectedFaceDescriptor);

    friend Face				get(face_tag,			const TetraMesh& M,FaceDescriptor);
    friend Face				get(face_tag,			const TetraMesh& M,DirectedFaceDescriptor);

    friend TetraByFaceID	get(faces_tag,			const TetraMesh& M,TetraDescriptor);
};

class TetraMesh::FaceDescriptor : public WrappedInteger<unsigned>
{
public:
	explicit FaceDescriptor(unsigned i) : WrappedInteger<unsigned>(i){}
	FaceDescriptor(){}

private:
	friend class DirectedFaceDescriptor;
};

class TetraMesh::DirectedFaceDescriptor : public WrappedInteger<int>
{
public:
	/// Allow implicit conversion of FaceDescriptor into DirectedFaceDescriptor
	DirectedFaceDescriptor(const FaceDescriptor& F,bool invert=false) :
		WrappedInteger<int>(invert ? -F.m_value : F.m_value){}
	DirectedFaceDescriptor(){}

	FaceDescriptor undirected() const { return FaceDescriptor(std::abs(WrappedInteger<int>::m_value)); }


};

#ifndef SWIG

constexpr point_above_face_tag point_above_face;
TetraMesh::PointDescriptor get(point_above_face_tag,const TetraMesh& M,TetraMesh::FaceDescriptor);

constexpr point_below_face_tag point_below_face;
TetraMesh::PointDescriptor get(point_below_face_tag,const TetraMesh& M,TetraMesh::FaceDescriptor);

constexpr tetra_above_face_tag tetra_above_face;

constexpr tetra_below_face_tag tetra_below_face;

constexpr face_tag face;
Face get(face_tag,const TetraMesh& M,TetraMesh::FaceDescriptor);

constexpr faces_tag faces;
TetraByFaceID get(faces_tag,const TetraMesh& M,TetraMesh::TetraDescriptor);

inline TetraMesh::FaceRange TetraMesh::faces() const
{
	return FaceRange(FaceIterator(FaceDescriptor(0)),FaceIterator(FaceDescriptor(m_faces.size())));
}

inline TetraMesh::TetraRange TetraMesh::tetras() const
{
	return TetraRange(
			TetraIterator(TetraDescriptor(0U)),
			TetraIterator(TetraDescriptor(m_tetraPoints.size())));
}
template<typename Prop>std::array<decltype(get(std::declval<Prop>(),std::declval<const TetraMesh&>(),TetraMeshBase::PointDescriptor())),3>
	get(Prop p,const TetraMesh& M,TetraMesh::FaceDescriptor IDf)
	{
	typedef decltype(get(p, M, TetraMesh::PointDescriptor())) Result;
	FaceByPointID IDps = get(points,M,IDf);
	std::array<Result,3> res;

	for(unsigned i=0;i<3;++i)
		res[i] = get(p,M,TetraMesh::PointDescriptor(IDps[i]));

	return res;
	}

#endif

#endif
