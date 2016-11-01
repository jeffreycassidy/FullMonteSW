#include <limits>
#include <map>
#include <set>
#include <cassert>
#include <signal.h>

#include "Filters/FilterBase.hpp"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include "StandardArrayKernel.hpp"
#include "BoundingBox.hpp"

#include <iomanip>

#include "TetraMesh.hpp"

using namespace std;


TetraMesh::~TetraMesh()
{
}

void TetraMesh::buildTetrasAndFaces(const vector<FaceHint>& hint)
{
	cout << "Building mesh data structures" << endl;
	m_timer.start();

	if (hint.size() == 0)
	{
		mapTetrasToFaces();
		cout << "INFO: Tetra-face mapping finished at " << m_timer.elapsed().wall*1e-9 << "s" << endl;
	}
	else
	{
		m_facePoints.resize(hint.size());
		m_faceTetras.resize(hint.size());

		for(unsigned i=0;i<hint.size();++i)
		{
			m_facePoints[i] = hint[i].fIDps;
			m_faceTetras[i] = hint[i].IDts;
		}

		cout << "INFO: Used face hints to skip tetra-face mapping at " << m_timer.elapsed().wall*1e-9 << endl;
	}

	createFaceNormals();
	cout << "INFO: Face normals finished at " << m_timer.elapsed().wall*1e-9 << "s" << endl;

	if (hint.size() == 0)
	{
		orientFaces();
		cout << "INFO: Face orientation check finished at " << m_timer.elapsed().wall*1e-9 << "s" << endl;
	}
	else
		cout << "INFO: Using face hints to skip face orientation" << endl;

	makeKernelTetras();
	cout << "INFO: Kernel tetras built at " << m_timer.elapsed().wall*1e-9 << "s" << endl;

	std::size_t Nf_surf = boost::size(m_faceTetras |		 boost::adaptors::filtered([](array<unsigned,2> i){ return i[1]==0; }));

	cout << "New mesh construction: " << m_points.size() << " points, " << m_tetraPoints.size() << " tetras, " << m_pointIDsToFaceMap.size() <<
			" faces (" << Nf_surf << " surface)" << endl;

	assert(m_tetraPoints.size() == m_tetraFaces.size());
}

/* Works fine but temporarily disabled; should be moved to Tetra class & source file
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
 */

void TetraMesh::createFaceNormals()
{
	m_faces.clear();

	Point<3,double> O{0.0,0.0,0.0};

	m_faces.resize(m_facePoints.size());
	m_faces[0] = Face(O,O,O);

	for(unsigned i=1;i<m_facePoints.size();++i)
		m_faces[i] = Face(m_points[m_facePoints[i][0]], m_points[m_facePoints[i][1]], m_points[m_facePoints[i][2]]);
}

void TetraMesh::orientTetras()
{
//	for(const auto t : tetras())
//	{
//		std::array<Point<3,double>,4> P = get(point_coords,*this,t);
//
//		//if (scalartriple(,,) < 0)
//	}
}

void TetraMesh::orientFaces()
{
	for(const auto f : faces())
	{
		const auto IDtd = get(tetra_below_face,*this,f);

		const auto IDpu = get(point_above_face,*this,f);
		const auto IDpd = get(point_below_face,*this,f);

		Point<3,double> Pu = get(point_coords,*this,IDpu);
		Point<3,double> Pd = get(point_coords,*this,IDpd);

		Face F = get(face,*this,f);

		double h_up 	= F.pointHeight(Pu);
		double h_down 	= F.pointHeight(Pd);

		// If this is a boundary tetra, make sure face is going the right way
		if (h_up < 0 && IDtd.value() == 0)
			m_faces[f.value()] = -m_faces[f.value()];
		// If not a boundary tetra, check if orientation is backwards
		else if (IDtd.value() && IDpu.value() && IDpd.value() && h_up < h_down)
			m_faces[f.value()] = -m_faces[f.value()];
	}
}


void TetraMesh::mapTetrasToFaces()
{
	m_facePoints.clear();
	m_facePoints.push_back(FaceByPointID(0,0,0));

	m_pointIDsToFaceMap.clear();

	Point<3,double> O{0.0,0.0,0.0};

	m_faceTetras.clear();
	m_faceTetras.push_back(array<unsigned,2>{{0,0}});

	m_tetraFaces = vector<TetraByFaceID>(m_tetraPoints.size(),TetraByFaceID{0,0,0,0});

	for(auto T : m_tetraPoints | boost::adaptors::indexed(0U))
	{
		// sort indices to get canonical order for hash lookup
		TetraByPointID IDps_sort = T.value();
		boost::sort(IDps_sort);

		if (T.index() > 0)
			for(const auto& perm : tetra_face_opposite_point_indices | boost::adaptors::indexed(0U))
			{
				FaceByPointID Ft(
						IDps_sort[perm.value().faceidx[0]],
						IDps_sort[perm.value().faceidx[1]],
						IDps_sort[perm.value().faceidx[2]]);

				//unsigned IDp_opposite = IDps_sort[perm.value().oppidx];

				auto p = m_pointIDsToFaceMap.insert(make_pair(Ft,m_faceTetras.size()));

				if(p.second)			// new tuple inserted, this is first tetra referenced by the face
				{
					// add new face to data structures

					// NOTE: Errors are possible if this is a degenerate (or nearly-degenerate) tetra
					// (ie. opposite-point altitude nearly 0)
					//
					// May settle on wrong orientation, which will imply wrong orientation for the next tetra attached to the face

					m_faceTetras.push_back(array<unsigned,2>{(unsigned)T.index(),0});

					m_tetraFaces[T.index()][perm.index()] = p.first->second;			// link the tetra to the face (positive side)
				}
				else				// face already exists -> this tetra is the second attached to the face
				{
					// face is already present so link face <-> tetra
					m_faceTetras[p.first->second][1] = T.index();
					m_tetraFaces[T.index()][perm.index()] = -p.first->second;
				}
			}
	}

	m_facePoints.resize(m_faceTetras.size());
	for(const auto p : m_pointIDsToFaceMap)
		m_facePoints[p.second] = p.first;
}


bool TetraMesh::checkFaces() const
{
	unsigned err=0;
	assert(T.size()   == m_tetraPoints.size());
	assert(m_tetraFaces.size() == m_tetraPoints.size());
	assert(m_tetraMaterials.size() == m_tetraPoints.size());

	assert(m_faces.size() == m_facePoints.size());
	assert(m_faceTetras.size() == m_facePoints.size());

	if (m_enableVerboseConstruction)
		cout << "INFO TetraMesh::checkFaces() checking " << m_tetraPoints.size() << " tetras' faces for correct orientation" << endl;

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
				if ((h[f][p] = F[f].pointHeight(m_points[IDps[p]])) < -m_pointHeightTolerance)
				{
					if (m_enableVerboseConstruction)
						cout << "ERROR! TetraMesh::makeKernelTetras reports point height less than zero (" << h[f][p] << ") for " << p << "'th point of tetra " << IDt << " over face " << IDf << endl;
					tetOK=false;
				}

				if (h[f][p] < -1e-4)
				{
					if (m_enableVerboseConstruction)
						cout << "  NOTE: Face should be flipped" << endl;
					++err;
//					F[f].flip();
				}
			}
		}

//		if (m_enableVerboseConstruction && !tetOK)
//			printTetra(IDt);
	}

	return err==0;

	cout << "INFO: checkFaces() done at " << m_timer.elapsed().wall*1e-9 << 's' << endl;
}




/** Converts the TetraMesh representation to the packed data structures used by the kernel.
 *
 * Uses: m_faces, m_tetraFaces, m_tetraFaces, m_tetraMaterials
 * Produces: m_tetras
 *
 */

void TetraMesh::makeKernelTetras()
{
	vector<Tetra> T(m_tetraPoints.size());

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

	if (m_enableFaceChecks)
		if (!checkKernelFaces())
			cerr << "WARNING: Failure in TetraMesh::checkKernelFaces" << endl;
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



/** Validates the orientation of faces within the kernel tetra structure.
 *
 */

bool TetraMesh::checkKernelFaces() const
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
			std::array<float,4> h = to_array<float,4>(m_tetras[IDt].heights(to_m128(tetraPointCoords[f])));

			for(unsigned i=0;i<4;++i)
				if (h[i] < -4e-5)
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
	cout << "INFO: Face check completed at " << m_timer.elapsed().wall*1e-9 << 's' << endl;
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


	OrthoBoundingBox<float,3> bb;

	array<float,3> pf;

	cout << "There are " << m_points.size() << " points" << endl;

	for(unsigned i=0;i<m_points.size();++i)
	{
		pf[0] = m_points[i][0];
		pf[1] = m_points[i][1];
		pf[2] = m_points[i][2];
		bb.insert(pf);
	}

	const auto lims = bb.corners();

	cout << "Bounding box is [" << lims.first[0] << ',' << lims.second[0] << "] [" << lims.first[1] << ',' << lims.second[1] << "] [" << lims.first[2] << ',' << lims.second[2] << ']' << endl;
	cout << "Searching for tetra to enclose " << p[0] << ' ' << p[1] << ' ' << p[2] << endl;

	for(unsigned i=1;i<m_tetras.size(); ++i)
	{
		array<float,4> h;
		_mm_store_ps(h.data(), m_tetras[i].heights(pv));

		bool in=true;

		for(unsigned j=0;j<4;++j)
			in &= h[j] > -2e-5;

		if (in)
		{
			cout << "  Tetra " << i << " encloses with heights ";
			for(unsigned j=0;j<4;++j)
				cout << h[j] << ' ';

			TetraByFaceID IDfs = get(::faces, *this, TetraMeshBase::TetraDescriptor(i) );
			cout << "    Face|adj tetra: ";

			for(unsigned i=0;i<4;++i)
				cout << IDfs[i] << '|' << get(tetra_below_face, *this, FaceDescriptor(IDfs[i])).value() << "  ";
			cout << endl;

			++N;
			IDt=i;
		}
	}

	if (N < 1)
		cout << "ERROR: Could not find enclosing tetra" << endl;
	else if (N > 1)
		cout << "WARNING: enclosed by " << N << " tetras" << endl;
	return IDt;
}


/** Finds the surface face closest to the ray (p,d)
 * If predicate F is provided, only faces matching the predicate F are considered
 *
 * Returns the intersection point, distance to intersection, the tetra just after the interface,
 * and the face oriented to point along the ray
 */


RTIntersection TetraMesh::findSurfaceFace(array<float,3> p,array<float,3> d,const FilterBase<int>* F) const
{
	float dMin = numeric_limits<float>::infinity();
	int fMin = 0;

	if (!F)
		throw std::logic_error("TetraMesh::findSurfaceFace(p,d,F) given null filter (not supported)");

	for(unsigned i=1;i<m_faces.size(); ++i)
	{
		int IDf;

		if ((*F)(i))
			IDf = i;
		else if ((*F)(-i))
			IDf = -i;
		else
			continue;

		Face f = m_faces[i];

		const auto nd = f.getNormal();
		array<float,3> n{float(nd[0]),float(nd[1]),float(nd[2])};
		float c = f.getConstant();

		Point<3,double> T[3] = {
				Point<3,double>(m_points[m_facePoints[i][0]]),
				Point<3,double>(m_points[m_facePoints[i][1]]),
				Point<3,double>(m_points[m_facePoints[i][2]])
		};

		unsigned IDt_from 	= m_faceTetras[i][0];
		unsigned IDt_to 	= m_faceTetras[i][1];

		if (IDf < 0)
		{
			c=-c;
			n=-n;

			std::swap(IDt_from,IDt_to);
		}
		else
			std::swap(T[1],T[2]);

		float costheta = dot(d,n);

		float h = dot(p,n) - c;
		float dist = std::abs(h/costheta);

		//cout << "    Face " << IDf << " distance " << dist << " from tet " << IDt_from << " (mat " << m_tetraMaterials[IDt_from] << ") into tet " << IDt_to << " (material " << m_tetraMaterials[IDt_to] << ")" << endl;

		PointIntersectionResult res = RayTriangleIntersection(Point<3,double>(p), UnitVector<3,double>(d), T);
//		if (res.intersects)
//			cout << "      has intersection with ray, distance " << dist << endl;

		if (h >= 0 && dist >= 0 && dist < dMin && res.intersects)
		{
			dMin = dist;
			fMin = IDf;
		}
	}

	return RTIntersection {
		p + dMin*d,
		dMin,
		fMin,
		fMin < 0 ? m_faceTetras[-fMin][1] : m_faceTetras[fMin][0]			// return the tetra from which ray enters
	};
}


// check linear combination of points to verify we're within tetra
//   used only for testing, very slow
bool TetraMesh::isWithinByPoints(int tID,const Point<3,double>& p) const
{
    float M[3][4];
    const Point<3,double> &A=m_points[m_tetraPoints[tID][0]], &B=m_points[m_tetraPoints[tID][1]], &C=m_points[m_tetraPoints[tID][2]], &D=m_points[m_tetraPoints[tID][3]];
    Vector<3,double> e[3];

    // calculate edge vectors
    e[0]=(Vector<3,double>)(B)-(Vector<3,double>)(A);
    e[1]=(Vector<3,double>)(C)-(Vector<3,double>)(A);
    e[2]=(Vector<3,double>)(D)-(Vector<3,double>)(A);

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

FaceByPointID get(points_tag,const TetraMesh& M,TetraMesh::FaceDescriptor IDf)
{
	return M.m_facePoints[IDf.value()];
}

/** Returns the point descriptor that is present int the tetra tIDps but not the face fIDps
 *
 */

TetraMesh::PointDescriptor oddOneOut(TetraByPointID tIDps,FaceByPointID fIDps)
{
	bool incl[4]{ false,false,false,false };

	for(unsigned t=0;t<4;++t)
		for(unsigned f=0;f<3;++f)
			incl[t] |= fIDps[f] == tIDps[t];

	for(unsigned t=0;t<4;++t)
		if (!incl[t])
			return TetraMesh::PointDescriptor(tIDps[t]);

	return TetraMesh::PointDescriptor(0);
}

TetraMesh::PointDescriptor get(point_above_face_tag,const TetraMesh& M,TetraMesh::FaceDescriptor IDf)
{
	return oddOneOut(
			get(points,M,
				get(tetra_above_face,M,IDf)),
			get(points,M,IDf));
}

TetraMesh::PointDescriptor get(point_below_face_tag,const TetraMesh& M,TetraMesh::FaceDescriptor IDf)
{
	return oddOneOut(
			get(points,M,
				get(tetra_below_face,M,IDf)),
			get(points,M,IDf));
}


TetraMesh::TetraDescriptor get(tetra_above_face_tag,const TetraMesh& M,TetraMesh::DirectedFaceDescriptor IDf)
{
	int i = IDf.value();
	return TetraMesh::TetraDescriptor(i > 0 ? M.m_faceTetras[i][0] : M.m_faceTetras[-i][1]);
}

TetraMesh::TetraDescriptor get(tetra_below_face_tag,const TetraMesh& M,TetraMesh::DirectedFaceDescriptor IDf)
{
	int i = IDf.value();
	return TetraMesh::TetraDescriptor(i > 0 ? M.m_faceTetras[i][1] : M.m_faceTetras[-i][0]);
}

Face get(face_tag,const TetraMesh& M,TetraMesh::FaceDescriptor IDf)
{
	unsigned i = IDf.value();
	return M.m_faces[i];
}

Face get(face_tag,const TetraMesh& M,TetraMesh::DirectedFaceDescriptor IDf)
{
	int i = IDf.value();
	return i > 0 ? M.m_faces[i] : -M.m_faces[-i];
}

TetraByFaceID get(faces_tag,const TetraMesh& M,TetraMesh::TetraDescriptor d)
{
	return M.m_tetraFaces[d.value()];
}


/** Builds a layered representation similar to MCML. Face normals point up (0,0,1).
 * N layers of thickness t_i, i = 1..N cover depths [z_(i-1), z_i]
 *
 * Layer 0 			is the semi-infinite space above z=0
 * Layer i (1..N)	is the slab of thickness t_i between [z_i, z_(i+1)] with z_1=0
 * Layer N+1		is the semi-infinite space below z=z_N
 *
 * Slabs i and i+1 are separated by face i+1 at depth z_i
 *
 * NOTE: Point representations of the tetras are not valid.
 * Scoring should be done using spatial coordinates, not tetra/face IDs.
 */


TetraMesh TetraMesh::buildLayered(const std::vector<float>& thickness,std::array<float,3> normal)
{
	float tz=0.0f;
	std::vector<float> z(thickness.size()+1);

	// generate depths z_i
	z[0] = 0.0f;
	for(unsigned i=0;i<thickness.size();++i)
		z[i+1] = (tz += thickness[i]);

	m_tetraPoints.clear();
	m_facePoints.clear();
	m_points.clear();

	for(unsigned i=0;i<thickness.size();++i)
	{
		m_tetraFaces[i][0] =
		m_tetraFaces[i][1] =
		m_tetraFaces[i][2] = m_tetraFaces[i][3] = 0;

		m_tetraMaterials[i]=

		m_faces[i] =

		m_faceTetras[i][0] =
		m_faceTetras[i][1] =
		m_tetras[i] =
	}

}
