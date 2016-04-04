#include <limits>
#include <map>
#include <set>
#include <cassert>
#include <signal.h>

#include "Filters/FilterBase.hpp"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include <iomanip>

#include "TetraMesh.hpp"

using namespace std;


/** Returns a vector<unsigned> of tetra IDs for tetras which have at least one point within a radius r of point p0
 *
 */

vector<unsigned> TetraMesh::tetras_close_to(const Point<3,double> p0,const float r) const
{
	vector<unsigned> Tlist;
	const float r2=r*r;

	for(unsigned i=1;i<m_tetraPoints.size();++i)
		if (boost::algorithm::any_of(
				m_tetraPoints[i],
				[p0,r2,this](unsigned i){ return Vector<3,double>(m_points[i],p0).norm2_l2()<r2; }))
			Tlist.push_back(i);
	return Tlist;
}

TetraMesh::~TetraMesh()
{
}

vector<unsigned> TetraMesh::facesBoundingRegion(unsigned i) const
{
	vector<unsigned> idx;


	for(const auto& f : m_faceTetras | boost::adaptors::indexed(0U))
 		if (f.value()[0] != f.value()[1] && (f.value()[0]==i || f.value()[1]==i))
			idx.push_back(f.index());
	return idx;
}



void TetraMesh::buildTetrasAndFaces()
{
	cout << "Building mesh data structures" << endl;
	Point<3,double> O{0.0,0.0,0.0};

	m_pointIDsToFaceMap.clear();
	m_pointIDsToTetraMap.clear();

	m_faces.clear();
	m_facePoints.clear();
	m_faceTetras.clear();

	m_faces.push_back(Face(O,O,O));
	m_facePoints.push_back(FaceByPointID(0,0,0));
	m_faceTetras.push_back(array<unsigned,2>{{0,0}});

	m_tetraFaces = vector<TetraByFaceID>(m_tetraPoints.size(),TetraByFaceID{0,0,0,0});

	vector<double> m_faceAltitudes;

	for(auto T : m_tetraPoints | boost::adaptors::indexed(0U))
	{
		// sort indices to get canonical order for hash lookup
		TetraByPointID IDps_sort = T.value();
		boost::sort(IDps_sort);

		m_pointIDsToTetraMap.insert(make_pair(IDps_sort,T.index()));


		if (T.index() > 0)
			for(const auto& perm : tetra_face_opposite_point_indices | boost::adaptors::indexed(0U))
			{
				FaceByPointID Ft(
						IDps_sort[perm.value().faceidx[0]],
						IDps_sort[perm.value().faceidx[1]],
						IDps_sort[perm.value().faceidx[2]]);

				unsigned IDp_opposite = IDps_sort[perm.value().oppidx];

				auto p = m_pointIDsToFaceMap.insert(make_pair(Ft,m_faces.size()));

				if(p.second)			// new tuple inserted; ensure face is such that opposite point is above face
				{
					// add new face to data structures

					// NOTE: Errors are possible if this is a degenerate (or nearly-degenerate) tetra
					// (ie. opposite-point altitude nearly 0)
					//
					// May settle on wrong orientation, which will imply wrong orientation for the next tetra attached to the face
					m_faces.push_back(Face(m_points[Ft[0]], m_points[Ft[1]], m_points[Ft[2]], m_points[IDp_opposite]));
					m_faceAltitudes.push_back(m_faces.back().pointHeight(m_points[IDp_opposite]));

					m_faceTetras.push_back(array<unsigned,2>{(unsigned)T.index(),0});
					m_facePoints.push_back(Ft);

					m_tetraFaces[T.index()][perm.index()] = p.first->second;			// link the tetra to the face (positive side)
				}
				else				// already exists -> up-face already assigned -> this is down-face
				{
					// face is already present so link face <-> tetra
					m_faceTetras[p.first->second][1] = T.index();
					m_tetraFaces[T.index()][perm.index()] = -p.first->second;

					double h;

					if ((h=m_faces[p.first->second].pointHeight(m_points[IDp_opposite])) > 2e-5)
					{
						cout << "WARNING: buildTetrasAndFaces() added second face but opposite-point altitude unexpectedly positive (h=" <<
							h << ") with flipside altitude h'=" << m_faceAltitudes[p.first->second] << " in tetra " << T.index() << " bordering tetra " << m_faceTetras[p.first->second][0] << endl;

						// previous production code
//						if (std::abs(h) > std::abs(m_faceAltitudes[p.first->second]))
//						{
//							cout << "  flipping face to correct" << endl;
//							m_faces[p.first->second].flip();
//						}

						// one possible reason for an ill-oriented face is a degenerate tetra with bad numerical behavior
						// (eg. the cross product is very nearly zero)
						//
						// if that is the case, then construct the face from the other tetra which is (we hope) better behaved

						m_faces[p.first->second] = Face(m_points[Ft[0]],m_points[Ft[1]],m_points[Ft[2]],m_points[IDp_opposite]);
					}
				}
			}
	}

	std::size_t Nf_surf = boost::size(m_faceTetras |		 boost::adaptors::filtered([](array<unsigned,2> i){ return i[1]==0; }));

	cout << "New mesh construction: " << m_points.size() << " points, " << m_tetraPoints.size() << " tetras, " << m_pointIDsToFaceMap.size() <<
			" faces (" << Nf_surf << " surface)" << endl;

	assert(m_tetraPoints.size() == m_tetraFaces.size());

	makeKernelTetras();
}


/** Converts the TetraMesh representation to the packed data structures used by the kernel
 *
 */

void TetraMesh::printTetra(unsigned IDt) const
{
	cout << "Tetra ID " << IDt << endl;

	for(unsigned f=0;f<4;++f)
	{
		int IDf = m_tetraFaces[IDt][f];
		Face F = getFace(IDf);

		cout << "  Face " << f << " (ID " << IDf << ") tetra point heights (expect +): ";
		for(unsigned p=0;p<4;++p)
			cout << setw(10) << F.pointHeight(m_points[m_tetraPoints[IDt][p]]) << ' ';
		cout << endl;

		unsigned IDt_adj = IDf < 0 ?
				m_faceTetras[-IDf][0] :
				m_faceTetras[IDf][1];

		cout << "    Adjacent tetra ID " << IDt_adj << " point heights over this face (expect -): ";
		for(unsigned p=0;p<4;++p)
			cout << setw(10) << (F.pointHeight(m_points[m_tetraPoints[IDt_adj][p]])) << ' ';
		cout << endl;
	}
}

void TetraMesh::makeKernelTetras()
{
	vector<Tetra> T(m_tetraPoints.size());

	assert(T.size()   == m_tetraPoints.size());
	assert(m_tetraFaces.size() == m_tetraPoints.size());
	assert(m_tetraMaterials.size() == m_tetraPoints.size());

	assert(m_faces.size() == m_facePoints.size());
	assert(m_faceTetras.size() == m_facePoints.size());

	cout << "INFO TetraMesh::makeKernelTetras() checking " << m_tetraPoints.size() << " tetras' faces for correct orientation" << endl;
	for(unsigned IDt=1;IDt < m_tetraPoints.size();++IDt)
	{
		TetraByPointID IDps = m_tetraPoints[IDt];
		TetraByFaceID IDfs = m_tetraFaces[IDt];

		Face F[4];

		bool tetOK=true;

		std::array<std::array<double,4>,4> h;

		for(unsigned f=0;f<4;++f)
		{
			int IDf = IDfs[f];

			F[f] = getFace(IDf);

			for(unsigned p=0;p<4;++p)
			{
				if ((h[f][p] = F[f].pointHeight(m_points[IDps[p]])) < -2e-12)
				{
					cout << "ERROR! TetraMesh::makeKernelTetras reports point height less than zero (" << h[f][p] << ") for " << p << "'th point of tetra " << IDt << " over face " << IDf << endl;
					tetOK=false;
				}
			}
		}

		if (!tetOK)
			printTetra(IDt);
	}

	cout << "INFO TetraMesh::makeKernelTetras() creating " << T.size() << " kernel tetras" << endl;

	for(auto tet : T | boost::adaptors::indexed(0U))
	{
		tet.value().IDfs  = m_tetraFaces[tet.index()];
		tet.value().matID = m_tetraMaterials[tet.index()];
		tet.value().faceFlags=0;

		UnitVector<3,double> n[4];
		double C[4];

		// add the faces
		for(unsigned f=0; f<4; ++f)
		{
			int IDf = tet.value().IDfs[f];

			Face F = getFace(IDf);
			n[f] = F.getNormal();
			C[f] = F.getConstant();

			array<unsigned,2> IDts_adj = m_faceTetras[std::abs(IDf)];

			assert(IDts_adj[0] == tet.index() || IDts_adj[1] == tet.index() || tet.index()==0);
			assert(IDts_adj[0] >= 0 && IDts_adj[1] >= 0);

			if (IDts_adj[0] == tet.index())
				tet.value().adjTetras[f] = IDts_adj[1];
			else if (IDts_adj[1] == tet.index())
				tet.value().adjTetras[f] = IDts_adj[0];
			else
				assert(tet.index()==0);

		}

		tet.value().nx=_mm_setr_ps(n[0][0],n[1][0],n[2][0],n[3][0]);
		tet.value().ny=_mm_setr_ps(n[0][1],n[1][1],n[2][1],n[3][1]);
		tet.value().nz=_mm_setr_ps(n[0][2],n[1][2],n[2][2],n[3][2]);
		tet.value().C =_mm_setr_ps(C[0],C[1],C[2],C[3]);
	}

	m_tetras=T;

	if (!checkFaces())
		cerr << "WARNING: Failure in TetraMesh::checkFaces" << endl;
}

void TetraMesh::setFacesForFluenceCounting(const FilterBase<int>* TF)
{
	for(Tetra& tet : m_tetras)
	{
		unsigned flag=0;

		TetraByFaceID IDfs = tet.IDfs;

		for(unsigned i=0;i<4;++i)
		{
			if ((*TF)(IDfs[i]))
				flag |= (1<<(i<<3));
		}

		tet.faceFlags=flag;
	}
}

bool TetraMesh::checkFaces() const
{
	bool status_ok=true;

	cout << "INFO: Checking faces on " << m_tetras.size() << " tetras" << endl;

	// check faces (normals/constants) inside the Tetra structures
	for(unsigned IDt=1; IDt < m_tetras.size(); ++IDt)
	{
		TetraByPointID IDps = m_tetraPoints[IDt];
		std::array<Point<3,double>,4> doubleTetraPointCoords;

		std::array<std::array<float,3>,4> tetraPointCoords;

		for(unsigned i=0;i<4;++i)
		{
			doubleTetraPointCoords[i] = m_points[IDps[i]];
			for(unsigned j=0;j<3;++j)
				tetraPointCoords[i][j] = doubleTetraPointCoords[i][j];
		}

		bool tet_ok=true;
		for(int f=0;f<4;++f)
		{
			std::array<float,4> h = m_tetras[IDt].heights(tetraPointCoords[f]);

			for(unsigned i=0;i<4;++i)
				if (h[i] > 2e-5)
				{
					tet_ok=status_ok=false;
					cout << "Error: incorrect height of " << f << "'th tetra point (ID " << IDps[f] << ") over " << i << "'th face (ID" << m_tetras[IDt].IDfs[i] << "): " << h[i] << endl;
				}

			if (!tet_ok)
			{
				cout << "tet " << IDt << " face " << f << " opposite-corner heights: ";
				for(unsigned i=0;i<4;++i)
					cout << h[i] << ' ';
				cout << endl;
			}
		}
	}
	return status_ok;
}


// Very naive search to find closest point
unsigned TetraMesh::findNearestPoint(const Point<3,double>& p) const
{
	double d2=numeric_limits<double>::infinity(),t;
	unsigned id=0,c=1;

	for(vector<Point<3,double> >::const_iterator it=m_points.begin()+1; it != m_points.end(); ++it,++c)
	{
		if ((t=norm2_l2(Vector<3,double>(*it,p))) < d2)
		{
			d2 = t;
			id = c;
		}
	}
	return id;
}

unsigned TetraMesh::findEnclosingTetra(const Point<3,double>& p) const
{
	const float f[4]{ (float)p[0], (float)p[1], (float)p[2], 0.0 };
	__m128 pv = _mm_load_ps(f);
	unsigned N=0,IDt=0;

	for(unsigned i=1;i<m_tetras.size(); ++i)
		if (m_tetras[i].pointWithin(pv))
		{
			IDt=i;
			++N;
		}

	if (N > 1)
		cerr << "Warning: enclosed by " << N << " tetras" << endl;
	return IDt;
}

/** Verifies that a point is within the specified tetrahedron, using tolerance eps.
 */
bool Tetra::pointWithin(__m128 p,float eps) const
{
    // compute p (dot) n_i minus C_i for i in [0,3]
    __m128 dot =         _mm_mul_ps(nx,_mm_shuffle_ps(p,p,_MM_SHUFFLE(0,0,0,0)));
    dot = _mm_add_ps(dot,_mm_mul_ps(ny,_mm_shuffle_ps(p,p,_MM_SHUFFLE(1,1,1,1))));
    dot = _mm_add_ps(dot,_mm_mul_ps(nz,_mm_shuffle_ps(p,p,_MM_SHUFFLE(2,2,2,2))));
    dot = _mm_sub_ps(dot,C);

    dot = _mm_sub_ps(dot,_mm_set1_ps(eps));

    return _mm_movemask_ps(dot) == 0;
}


// check linear combination of points to verify we're within tetra
//   used only for testing, very slow
bool TetraMesh::isWithinByPoints(int tID,const Point<3,double>& p) const
{
    float M[3][4];
    const Point<3,double> &A=m_points[m_tetraPoints[tID][0]], &B=m_points[m_tetraPoints[tID][1]], &C=m_points[m_tetraPoints[tID][2]], &D=m_points[m_tetraPoints[tID][3]];
    Vector<3,double> e[3];

    // calculate edge vectors
    e[0]=B-A;
    e[1]=C-A;
    e[2]=D-A;

    // build basis matrix for tetra
    M[0][0] = e[0][0];
    M[1][0] = e[0][1];
    M[2][0] = e[0][2];
    M[0][3] = p[0]-A[0];

    M[0][1] = e[1][0];
    M[1][1] = e[1][1];
    M[2][1] = e[1][2];
    M[1][3] = p[1]-A[1];

    M[0][2] = e[2][0];
    M[1][2] = e[2][1];
    M[2][2] = e[2][2];
    M[2][3] = p[2]-A[2];

    double c;

    // eliminate
    for(unsigned i=0;i<3;++i)
    {
        // subtract rows above
        for(unsigned j=0;j<i;++j)
        {
            c=M[i][j];
            for(unsigned k=0;k<4;++k)
                M[i][k] -= c*M[j][k];
        }

        // normalize the row
        c=M[i][i];
        for(unsigned j=i;j<4;++j)
            M[i][j] /= c;
    }

    // backsub
    for(int i=1;i>=0;--i)
        for(unsigned j=i+1;j<3;++j)
        {
            c=M[i][j];
            for(unsigned k=0;k<4;++k)
                M[i][k] -= c*M[j][k];
        }

    float coeff[4];
    coeff[0]=1.0;

    for(unsigned i=1;i<4;++i)
    {
        coeff[i]=M[i-1][3];
        coeff[0] -= coeff[i];
    }

    printf("Coeffs are: %9.5f %9.5f %9.5f %9.5f ",coeff[0],coeff[1],coeff[2],coeff[3]);

    bool within=true,onedge=false;

    for(unsigned i=0;i<4;++i)
    {
        within &= (coeff[i] > -1e-4) & (coeff[i] < 1+1e-4);
        onedge |= (abs(coeff[i]) < 1e-4);
    }

    cout << " - " << (within ? (onedge ? "on edge" : "within") : "OUTSIDE") << endl;

    return within;
}



/** Finds the next face along the given ray
 *
 * An optional parameter IDf_exclude allows exclusion of one face (eg. if the ray starts at such face)
 *
 * TODO: Should include a tolerance on the >= 0 calc, in case the point is ever-so-slightly on the wrong side of the face,
 * 			eg. when trying to find the bounding face
 */

std::tuple<PointIntersectionResult,int> TetraMesh::findNextFaceAlongRay(Point<3,double> p,UnitVector<3,double> dir,int IDf_exclude) const
{
	unsigned IDf=0;
	double dMin = std::numeric_limits<double>::infinity();
	PointIntersectionResult best;

	for(unsigned i=0;i<m_faces.size();++i)
	{
		if (i == abs(IDf_exclude))
			continue;

		FaceByPointID IDps = m_facePoints[i];
		Point<3,double> T[3]{
			m_points[IDps[0]],
			m_points[IDps[1]],
			m_points[IDps[2]]
		};

		PointIntersectionResult res = RayTriangleIntersection(p,dir,T);

		if (res.intersects && res.t < dMin)
		{
			best = res;
			dMin = res.t;
			IDf = i;
		}
	}

	return make_tuple(best,IDf);
}

unsigned TetraMesh::getTetraFromFace(int IDf) const
{
    return IDf > 0 ? m_faceTetras[IDf][0] : m_faceTetras[-IDf][1];
}

double TetraMesh::getFaceArea(const FaceByPointID& f) const
{
    Vector<3,double> AB(m_points[f[0]],m_points[f[1]]);
    Vector<3,double> AC(m_points[f[0]],m_points[f[2]]);

    return cross(AB,AC).norm_l2()/2;
}



vector<unsigned> TetraMesh::getRegionBoundaryTris(unsigned r) const
{
	vector<unsigned> tri;
    for(unsigned IDf=0; IDf<m_faces.size(); ++IDf)
    	if(faceBoundsRegion(r,IDf))
    		tri.push_back(IDf);

    return tri;
}




/** returns a vector containing (face ID, tetra ID) for each face on the boundary of r0; if r1 is specified, then
 * it also requires that the face bound r1 (ie. is an r0-r1 boundary)
 *
 * The tetra ID returned is the tetra in region r0.
 */

// TODO: return signed int indicating face orientation?

vector<pair<unsigned,unsigned>> TetraMesh::getRegionBoundaryTrisAndTetras(unsigned r0,unsigned r1) const
{
	vector<pair<unsigned,unsigned>> v;

	for(unsigned i=0;i<m_faceTetras.size();++i)
	{
		unsigned Ta = m_faceTetras[i][0], Tb = m_faceTetras[i][1];
		assert(Ta < m_tetraMaterials.size() && Tb < m_tetraMaterials.size());

		unsigned ma = m_tetraMaterials[Ta], mb = m_tetraMaterials[Tb];

		if (ma == mb)														// no boundary
			continue;
		else if (ma == r0 && (r1==-1U || mb==r1))							// boundary; Ta is within region
			v.push_back(make_pair(i,Ta));
		else if (mb == r0 && (r1 ==-1U || ma == r1))						// boundary; Tb is within region
			v.push_back(make_pair(i,Tb));
	}

	return v;
}
