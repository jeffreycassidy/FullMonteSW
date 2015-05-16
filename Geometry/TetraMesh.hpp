#ifndef TETRAMESH_HPP_INCLUDED_
#define TETRAMESH_HPP_INCLUDED_

#ifndef SWIG
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <inttypes.h>
#include <functional>
#include <unordered_map>

#include "TriSurf.hpp"
#include "Face.hpp"
#include "Tetra.hpp"
#include "TetraMeshBase.hpp"

#include <boost/shared_array.hpp>
#include <boost/functional/hash.hpp>

#include "newgeom.hpp"

#include <emmintrin.h>

constexpr std::array<std::array<unsigned char,3>,4> tetra_face_indices {
	std::array<unsigned char,3>{ 0, 1, 2 },
	std::array<unsigned char,3>{ 0, 1, 3 },
	std::array<unsigned char,3>{ 0, 2, 3 },
	std::array<unsigned char,3>{ 1, 2, 3 }
};

struct face_opposite {
	std::array<unsigned char,3> faceidx;
	unsigned oppidx;
};

constexpr std::array<face_opposite,4> tetra_face_opposite_point_indices {
	face_opposite{std::array<unsigned char,3>{ 0, 1, 2 },3},
	face_opposite{std::array<unsigned char,3>{ 0, 1, 3 },2},
	face_opposite{std::array<unsigned char,3>{ 0, 2, 3 },1},
	face_opposite{std::array<unsigned char,3>{ 1, 2, 3 },0}
};

constexpr std::array<std::array<unsigned char,2>,6> tetra_edge_indices {
	std::array<unsigned char,2>{ 0, 1},
	std::array<unsigned char,2>{ 0, 2},
	std::array<unsigned char,2>{ 0, 3},
	std::array<unsigned char,2>{ 1, 2},
	std::array<unsigned char,2>{ 1, 3},
	std::array<unsigned char,2>{ 2, 3}
};

using namespace std;

#endif // SWIG

struct LegendEntry {
	std::string 		label;
	std::array<float,3> colour;
};

class TetraMesh : public TetraMeshBase {
	vector<TetraByFaceID>	    T_f;        		// tetra -> 4 face IDs
    vector<FaceByPointID>       F_p;        		// face ID -> 3 point IDs
	vector<Face>			    F;          		// faces (with normals and constants)
	vector<std::array<unsigned,2>>      F_t;  	// for each face f, vecFaceID_Tetra[f] gives the tetras adjacent to the face
    vector<Tetra>               tetras;     		// new SSE-friendly data structure

    vector<vector<unsigned>>	tetra_perm;

    template<class Archive>void serialize(Archive& ar,const unsigned ver)
    {
    	ar & boost::serialization::base_object<TetraMesh>(*this);
    	tetrasToFaces();
    }

    void make_tetra_perm();

    std::unordered_map<TetraByPointID,unsigned,boost::hash<std::array<unsigned,4>>> tetraMap;

	void tetrasToFaces();
	vector<Tetra> makeKernelTetras() const;

    std::unordered_map<FaceByPointID,unsigned,boost::hash<std::array<unsigned,3>>> faceMap;

	public:

    // iterators
    typedef vector<Point<3,double> >::const_iterator point_const_iterator;
    point_const_iterator pointBegin() const { return P.begin()+1; }
    point_const_iterator pointEnd()   const { return P.end(); }

    typedef vector<Face>::const_iterator face_const_iterator;
    face_const_iterator faceBegin() const { return F.begin()+1; }
    face_const_iterator faceEnd()   const { return F.end(); }

    typedef vector<FaceByPointID>::const_iterator face_id_const_iterator;
    face_id_const_iterator faceIDBegin() const { return F_p.begin()+1; }
    face_id_const_iterator faceIDEnd()   const { return F_p.end(); }

    typedef vector<TetraByPointID>::const_iterator tetra_const_iterator;
    tetra_const_iterator tetraIDBegin() const { return T_p.begin()+1; }
    tetra_const_iterator tetraIDEnd()   const { return T_p.end();     }

    typedef vector<Tetra>::const_iterator tetra_struct_const_iterator;

    tetra_struct_const_iterator tetra_begin() const { return tetras.begin(); }
    tetra_struct_const_iterator tetra_end()   const { return tetras.end(); }

	bool checkValid() const;

	enum TetraFileType { MatlabTP };

	virtual void Delete(){ delete this; }

	TetraMesh(){};
	TetraMesh(const TetraMeshBase& Mb) : TetraMeshBase(Mb)
		{ tetrasToFaces(); }

    TetraMesh(unsigned Np_,unsigned Nt_,unsigned Nf_) : TetraMeshBase(Np_,Nt_),T_f(Nf_+1),F_p(Nf_+1),F(Nf_+1),
        F_t(Nf_+1),tetras(Nt_+1){}
	//TetraMesh(string,TetraFileType);
	TetraMesh(const vector<Point<3,double> >& P_,const vector<TetraByPointID>& T_p_,const vector<unsigned>& T_m_)
		: TetraMeshBase(P_,T_p_,T_m_) { tetrasToFaces(); }
    TetraMesh(const double*,unsigned Np,const unsigned*,unsigned Nt);
    ~TetraMesh();

    void fromBinary(const string& pts,const string& tetras,const string& faces=string());

    vector<unsigned> facesBoundingRegion(unsigned i) const;
    const vector<unsigned>& tetrasInRegion(unsigned i) const;

	// query size of mesh
	unsigned getNf() const { return F.size()-1; }
//
//    unsigned getNf_boundary() const { return F_boundary_ID.size(); }
//    unsigned getNp_boundary() const { return P_boundary_ID.size(); }

	// Accessors for various point/face constructs
    Face                    getFace(int id)                 const { Face f = F[abs(id)]; if(id<0){ f.flip(); } return f; }
    int                     getFaceID(FaceByPointID)        const;
    const FaceByPointID&    getFacePointIDs(unsigned id)    const { assert(id < F_p.size()); return F_p[id]; }
	const TetraByFaceID&    getTetraByFaceID(unsigned id)   const { return T_f[id]; }
    unsigned                getTetraFromFace(int IDf)       const;

    unsigned                getTetraID(TetraByPointID IDt) const {
        auto it = tetraMap.find(IDt);
        return (it == tetraMap.end() ? 0 : it->second);
    }

    Tetra                   getTetra(unsigned IDt) const { return tetras[IDt]; }

    // returns the tetra that the given face points into
    unsigned                getTetraIDFromFaceID(int IDf) const
        { return IDf > 0 ? F_t[IDf][0] : F_t[-IDf][1]; }

    double                  getFaceArea(const FaceByPointID&)   const;
    double                  getFaceArea(int IDf)                const { return getFaceArea(F_p[abs(IDf)]); }
    double                  getFaceArea(unsigned IDf)           const { return getFaceArea(F_p[IDf]); }


	// find nearest point or enclosing tetra
	unsigned findEnclosingTetra(const Point<3,double>&) const;
	unsigned findNearestPoint  (const Point<3,double>&) const;
	//pair<Point<3,double>,double> findNearestDelaunay(const Point<3,double>&) const;

    // checks if a point is within a given tetra by expressing as a linear combination of the corner points
    bool isWithinByPoints(int,const Point<3,double>&) const;

    // check if a ray intersects a face of a tetra
    //StepResult getIntersection(unsigned,const Ray<3,double>&,double=std::numeric_limits<double>::infinity(),int=0) const;

    // find the surface element hit by an incoming ray
    //pair<pair<unsigned,int>,Point<3,double> > getSurfaceElement(const Ray<3,double>&) const;

    vector<unsigned> getMaterialMap() const { return T_m; }

	// checks if faces are oriented correctly
	bool checkFaces() const;

	std::vector<unsigned> tetras_close_to(Point<3,double> p0,float) const;

    // does a number of data-structure integrity checks
    bool checkIntegrity(bool printResults=true) const;

    bool faceBoundsRegion(unsigned region,int IDf) const {
        	array<unsigned,2> p = F_t[abs(IDf)];		// Get the two incident tetras
        	unsigned r0 = T_m[p[0]], r1 = T_m[p[1]];				// check material types
        	return r0!=r1 && (r1==region || r0==region);
        }

    /// Creates a copy of the points and faces comprising the boundary of given material
    TriSurf extractMaterialBoundary(unsigned matID) const;
    vector<unsigned> getRegionBoundaryTris(unsigned r) const;

    /// Extracts the face IDs and tetra IDs corresponding to a given surface
    vector<pair<unsigned,unsigned>> getRegionBoundaryTrisAndTetras(unsigned r0,unsigned r1=-1U) const;

    TriSurf extractRegionSurface(const vector<unsigned>& tetIDs) const;

    // functions for saving tetramesh representations
    pair<unsigned,boost::shared_array<const uint8_t> > tetrasAsBinary() const;
    pair<unsigned,boost::shared_array<const uint8_t> > pointsAsBinary() const;

    friend class TetraGraph;
};

#endif
