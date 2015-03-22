#pragma once
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <inttypes.h>
#include <functional>

#include "TriSurf.hpp"
#include "Face.hpp"
#include "Tetra.hpp"
#include "TetraMeshBase.hpp"

#include <boost/shared_array.hpp>

#include "newgeom.hpp"

#include <emmintrin.h>

using namespace std;

struct LegendEntry {
	std::string 		label;
	std::array<float,3> colour;
};

class TetraMesh : public TetraMeshBase {
	vector<TetraByFaceID>	    T_f;        		// tetra -> 4 face IDs
    vector<FaceByPointID>       F_p;        		// face ID -> 3 point IDs
	vector<Face>			    F;          		// faces (with normals and constants)
	vector<pair<int,int> >      vecFaceID_Tetra;  	// for each face f, vecFaceID_Tetra[f] gives the tetras adjacent to the face
    vector<Tetra>               tetras;     		// new SSE-friendly data structure

    vector<vector<unsigned>>	tetra_perm;

    void make_tetra_perm();

    // boundary data structures; map with key=volume-set ID, value=surface-set ID
    map<unsigned,unsigned> P_boundary_ID;
	map<unsigned,unsigned> F_boundary_ID;

    map<TetraByPointID,unsigned> tetraMap;

	int  tetrasToFaces(vector<Face>&,vector<TetraByPointID>&,const vector<Point<3,double> >&,vector<TetraByFaceID>&);

    map<FaceByPointID,unsigned> faceMap;

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

    class boundary_f_const_iterator : public map<unsigned,unsigned>::const_iterator {
        const TetraMesh& __m;

        public:
        using map<unsigned,unsigned>::const_iterator::operator++;
        using map<unsigned,unsigned>::const_iterator::operator==;
        using map<unsigned,unsigned>::const_iterator::operator!=;
        boundary_f_const_iterator(const TetraMesh& m_,map<unsigned,unsigned>::const_iterator it_) :
            map<unsigned,unsigned>::const_iterator(it_),__m(m_){ };

        // returns the current face in terms of its surface point IDs (indexes into the boundary point set)
        FaceByPointID operator*() const {
            assert((*this)->first != 0);
            FaceByPointID f_original(__m.F_p[(*this)->first]);

            map<unsigned,unsigned>::const_iterator it=__m.P_boundary_ID.find((unsigned)abs((int)f_original[0]));
            assert(it != __m.P_boundary_ID.end());
            f_original[0] = it->second;

            it = __m.P_boundary_ID.find((unsigned)abs((int)f_original[1]));
            assert(it != __m.P_boundary_ID.end());
            f_original[1] = it->second;

            it = __m.P_boundary_ID.find((unsigned)abs((int)f_original[2]));
            assert(it != __m.P_boundary_ID.end());
            f_original[2] = it->second;

            return f_original;
            };
    };

    boundary_f_const_iterator boundaryFaceBegin() const { return boundary_f_const_iterator(*this,F_boundary_ID.begin()); }
    boundary_f_const_iterator boundaryFaceEnd()   const { return boundary_f_const_iterator(*this,F_boundary_ID.end());   }

    typedef vector<Tetra>::const_iterator tetra_struct_const_iterator;

    tetra_struct_const_iterator tetra_begin() const { return tetras.begin(); }
    tetra_struct_const_iterator tetra_end()   const { return tetras.end(); }

	bool checkValid() const;
	bool writeFileMatlabF(string) const; 

	enum TetraFileType { MatlabTP };

	virtual void Delete(){ delete this; }

	TetraMesh(){};
	TetraMesh(const TetraMeshBase& Mb) : TetraMeshBase(Mb){ tetrasToFaces(F,T_p,P,T_f); }
    TetraMesh(unsigned Np_,unsigned Nt_,unsigned Nf_) : TetraMeshBase(Np_,Nt_),T_f(Nf_+1),F_p(Nf_+1),F(Nf_+1),
        vecFaceID_Tetra(Nf_+1),tetras(Nt_+1){}
	TetraMesh(string,TetraFileType);
	TetraMesh(const vector<Point<3,double> >& P_,const vector<TetraByPointID>& T_p_,const vector<unsigned>& T_m_)
		: TetraMeshBase(P_,T_p_,T_m_) { tetrasToFaces(F,T_p,P,T_f); }
    TetraMesh(const double*,unsigned Np,const unsigned*,unsigned Nt);
    ~TetraMesh();

    void fromBinary(const string& pts,const string& tetras,const string& faces=string());

    vector<unsigned> facesBoundingRegion(unsigned i) const;
    const vector<unsigned>& tetrasInRegion(unsigned i) const;

	// query size of mesh
	unsigned getNf() const { return F.size()-1; }

    unsigned getNf_boundary() const { return F_boundary_ID.size(); }
    unsigned getNp_boundary() const { return P_boundary_ID.size(); }

	// Accessors for various point/face constructs
	const Point<3,double>&  getPoint(unsigned id)           const { return P[id]; }
    Face                    getFace(int id)                 const { Face f = F[abs(id)]; if(id<0){ f.flip(); } return f; }
    int                     getFaceID(FaceByPointID)        const;
    const FaceByPointID&    getFacePointIDs(unsigned id)    const { assert(id < F_p.size()); return F_p[id]; }
	const TetraByFaceID&    getTetraByFaceID(unsigned id)   const { return T_f[id]; }
	const TetraByPointID&   getTetraPointIDs(unsigned id)  const { return T_p[id]; }
    unsigned                getTetraFromFace(int IDf)       const;
    Point<3,double>         getTetraPoint(unsigned IDt,unsigned i) const { return P[T_p[IDt][i]]; }

    unsigned                getTetraID(TetraByPointID IDt) const {
        map<TetraByPointID,unsigned>::const_iterator it = tetraMap.find(IDt);
        return (it == tetraMap.end() ? 0 : it->second);
    }

    Tetra                   getTetra(unsigned IDt) const { return tetras[IDt]; }
    unsigned                getMaterial(unsigned IDt) const { return T_m[IDt]; }

    // returns the tetra that the given face points into
    unsigned                getTetraIDFromFaceID(int IDf) const
        { return IDf > 0 ? vecFaceID_Tetra[IDf].first : vecFaceID_Tetra[-IDf].second; }

    double                  getFaceArea(const FaceByPointID&)   const;
    double                  getFaceArea(int IDf)                const { return getFaceArea(F_p[abs(IDf)]); }
    double                  getFaceArea(unsigned IDf)           const { return getFaceArea(F_p[IDf]); }

    double                  getTetraVolume(TetraByPointID IDps) const
        { return abs(scalartriple(P[IDps[0]],P[IDps[1]],P[IDps[2]],P[IDps[3]])/6); }
    double                  getTetraVolume(unsigned IDt) const { return getTetraVolume(T_p[IDt]); }

    const vector<Point<3,double> >& getPoints() const { return P; }
    const vector<TetraByPointID>& getTetrasByPointID() const { return T_p; }

	// find nearest point or enclosing tetra
	unsigned findEnclosingTetra(const Point<3,double>&) const;
	unsigned findNearestPoint  (const Point<3,double>&) const;
	pair<Point<3,double>,double> findNearestDelaunay(const Point<3,double>&) const;

    // checks if a point is within a given tetra by expressing as a linear combination of the corner points
    bool isWithinByPoints(int,const Point<3,double>&) const;

    // check if a ray intersects a face of a tetra
    //StepResult getIntersection(unsigned,const Ray<3,double>&,double=std::numeric_limits<double>::infinity(),int=0) const;

    // find the surface element hit by an incoming ray
    pair<pair<unsigned,int>,Point<3,double> > getSurfaceElement(const Ray<3,double>&) const;

    vector<unsigned> getMaterialMap() const { return T_m; }

	// checks if faces are oriented correctly
	bool checkFaces() const;

	std::vector<unsigned> tetras_close_to(Point<3,double> p0,float) const;

    // does a number of data-structure integrity checks
    bool checkIntegrity(bool printResults=true) const;

    bool faceBoundsRegion(unsigned region,int IDf) const {
        	pair<unsigned,unsigned> p = vecFaceID_Tetra[abs(IDf)];		// Get the two incident tetras
        	unsigned r0 = T_m[p.first], r1 = T_m[p.second];				// check material types
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
