#include <limits>
#include <map>
#include <set>
#include <cassert>
#include <signal.h>

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include "TetraMesh.hpp"

#include "../Kernels/Software/sse.hpp"

using namespace std;


/** Returns a vector<unsigned> of tetra IDs for tetras which have at least one point within a radius r of point p0
 *
 */

vector<unsigned> TetraMesh::tetras_close_to(const Point<3,double> p0,const float r) const
{
	vector<unsigned> Tlist;
	const float r2=r*r;

	for(unsigned i=1;i<T_p.size();++i)
		if (boost::algorithm::any_of(
				T_p[i],
				[p0,r2,this](unsigned i){ return Vector<3,double>(this->P[i],p0).norm2_l2()<r2; }))
			Tlist.push_back(i);
	return Tlist;
}
//
//TetraMesh::TetraMesh(const double* p,unsigned Np_,const unsigned* t,unsigned Nt_)
//{
//    P.clear();
//    P.resize(Np_+1);
//    T_m.clear();
//    T_m.resize(Nt_+1);
//    T_p.clear();
//    T_p.resize(Nt_+1);
//
//    unsigned zeros[4]={0,0,0,0};
//
//    P[0] = Point<3,double>();
//    T_m[0] = 0;
//    T_p[0] = TetraByPointID(zeros);
//
//    for(unsigned i=1;i<=Np_;++i,p+=3)
//        P[i] = Point<3,double>(p);
//
//    for(unsigned i=1;i<=Nt_;++i,t+=5)
//    {
//        T_p[i]=TetraByPointID(t);        // direct copy from unsigned*
//        T_m[i]=t[4];
//    }
//
//	tetrasToFaces();
//
//	make_tetra_perm();
//}
//
TetraMesh::~TetraMesh()
{
}

vector<unsigned> TetraMesh::facesBoundingRegion(unsigned i) const
{
	vector<unsigned> idx;


	for(const auto& f : F_t | boost::adaptors::indexed(0U))
 		if (f.value()[0] != f.value()[1] && (f.value()[0]==i || f.value()[1]==i))
			idx.push_back(f.index());
	return idx;
}

const vector<unsigned>& TetraMesh::tetrasInRegion(unsigned i) const
{
	return tetra_perm[i];
}

void TetraMesh::make_tetra_perm()
{
	// get max region code
	unsigned max_region = *boost::max_element(T_m);

	// init permutation vector
	tetra_perm.resize(max_region+1);
	for(auto& v : tetra_perm)
		v.clear();

		// create permutation vector
	for(const auto r : T_m | boost::adaptors::indexed(0U))
		tetra_perm[r.value()].push_back(r.index());

	// print region summary
	size_t sum=0;

	
for(const auto& v : tetra_perm | boost::adaptors::indexed(0U))
	{
		sum += v.value().size();
 		cout << "  Region " << v.index() << ": " << v.value().size() << " elements" << endl;

	}
	cout << "Total " << sum << " elements" << endl;
}

//void TetraMesh::fromBinary(const string& pts,const string& tetras,const string& faces)
//{
//    P.clear();
//    P.resize(pts.size()/24+1);
//    T_m.clear();
//    T_m.resize(tetras.size()/20+1);
//    T_p.clear();
//    T_p.resize(tetras.size()/20+1);
//
//    unsigned zeros[4]={0,0,0,0};
//    P[0] = Point<3,double>();
//    T_m[0] = 0;
//    T_p[0] = TetraByPointID(zeros);
//
//    unsigned i=1;
//
//    // copy points over
//    for(const double* p=(const double*)pts.data(); p < (const double*)pts.data()+pts.size()/8; p += 3,++i)
//    {
//        P[i][0] = p[0];
//        P[i][1] = p[1];
//        P[i][2] = p[2];
//    }
//
//    i=1;
//    for(const uint32_t* p=(const uint32_t*)tetras.data(); p < (const uint32_t*)tetras.data()+tetras.size()/4; p += 5,++i)
//    {
//        T_p[i][0] = p[0];
//        T_p[i][1] = p[1];
//        T_p[i][2] = p[2];
//        T_p[i][3] = p[3];
//        T_m[i] = p[4];
//    }
//	tetrasToFaces();
//}



/* 	pair<unsigned id,bool flip> getFaceID(m,f)

Args
	m		Map of FaceByPointID to unsigned
	f		FaceByPointID to find

Returns
	id		Face ID
	flip	True if orientation is opposite
*/

int TetraMesh::getFaceID(FaceByPointID f) const
{
	bool neg;

	FaceByPointID tmpR = f.getRotateMin();

	if ((neg = (tmpR[2] < tmpR[1])))
		swap(tmpR[2],tmpR[1]);

	auto it = faceMap.find(tmpR);
	assert(it != faceMap.end());

    return it == faceMap.end() ? 0 : (neg ? it->second : -it->second);
}


void TetraMesh::tetrasToFaces()
{
	cout << "Building mesh data structures" << endl;
	Point<3,double> O{0.0,0.0,0.0};

	faceMap.clear();
	tetraMap.clear();

	F.clear();
	F_p.clear();
	F_t.clear();

	F.push_back(Face(O,O,O));
	F_p.push_back(FaceByPointID(0,0,0));
	F_t.push_back(array<unsigned,2>{{0,0}});

	T_f = vector<TetraByFaceID>(T_p.size(),TetraByFaceID{0,0,0,0});

	for(auto T : T_p | boost::adaptors::indexed(0U))
	{
		TetraByPointID IDps_sort = T.value();
		boost::sort(IDps_sort);

		tetraMap.insert(make_pair(IDps_sort,T.index()));


		if (T.index() > 0)
			for(const auto& perm : tetra_face_opposite_point_indices | boost::adaptors::indexed(0U))
			{
				FaceByPointID Ft(
						IDps_sort[perm.value().faceidx[0]],
						IDps_sort[perm.value().faceidx[1]],
						IDps_sort[perm.value().faceidx[2]]);


				auto p = faceMap.insert(make_pair(Ft,F.size()));

				if(p.second)			// new tuple inserted; ensure face is such that opposite point is above face
				{
					F.push_back(Face(P[Ft[0]], P[Ft[1]], P[Ft[2]]));

					if (F.back().pointHeight(P[IDps_sort[perm.value().oppidx]]) < 0)
						F.back().flip();
					F_t.push_back(array<unsigned,2>{(unsigned)T.index(),0});
					F_p.push_back(Ft);
					T_f[T.index()][perm.index()] = p.first->second;			// link the tetra to the face
				}
				else				// already exists -> up-face already assigned -> this is down-face
				{
					F_t[p.first->second][1] = T.index();
					T_f[T.index()][perm.index()] = -p.first->second;			// link the tetra to the face
				}
			}
	}

	std::size_t Nf_surf = boost::size(F_t |		 boost::adaptors::filtered([](array<unsigned,2> i){ return i[1]==0; }));

	cout << "New mesh construction: " << P.size() << " points, " << T_p.size() << " tetras, " << faceMap.size() <<
			" faces (" << Nf_surf << " surface)" << endl;

	assert(T_p.size() == T_f.size());


	// stable partition to place all surface faces first
//	vector<NewFaceDef>::const_iterator end_surf = boost::stable_partition(faces,[](const NewFaceDef& f){ return f.IDts[1] == 0; });
//	unsigned Nfsurf = end_surf-faces.cbegin();

	// create vector to permute back to "old-style" FullMonte ordering
//	vector<unsigned> perm(faceMap.size());
//
//	boost::copy(
//			boost::counting_range(0U,(unsigned)(faceMap.size())),
//			perm.begin());

	//checkFaces();

	tetras = makeKernelTetras();
}


/** Converts the TetraMesh representation to the packed data structures used by the kernel
 *
 */

vector<Tetra> TetraMesh::makeKernelTetras() const
{
	vector<Tetra> T(T_p.size());

	assert(T.size()   == T_p.size());
	assert(T_f.size() == T_p.size());
	assert(T_m.size() == T_p.size());

	assert(F.size() == F_p.size());
	assert(F_t.size() == F_p.size());

	for(auto tet : T | boost::adaptors::indexed(0U))
	{
		tet.value().IDfs  = T_f[tet.index()];
		tet.value().matID = T_m[tet.index()];

		UnitVector<3,double> n[4];
		double C[4];

		for(unsigned i=0; i<4; ++i)
		{
			int IDf = tet.value().IDfs[i];

			if (IDf < 0)
			{
				IDf = -IDf;
				n[i] = -F[IDf].getNormal();
				C[i] = -F[IDf].getConstant();
			}
			else
			{
				n[i] = F[IDf].getNormal();
				C[i] = F[IDf].getConstant();
			}

			assert(IDf >= 0);
			array<unsigned,2> IDts_adj = F_t[IDf];

			assert(IDts_adj[0] == tet.index() || IDts_adj[1] == tet.index() || tet.index()==0);
			assert(IDts_adj[0] >= 0 && IDts_adj[1] >= 0);

			if (IDts_adj[0] == tet.index())
				tet.value().adjTetras[i] = IDts_adj[1];
			else if (IDts_adj[1] == tet.index())
				tet.value().adjTetras[i] = IDts_adj[0];
			else
				assert(tet.index()==0);
		}

		tet.value().nx=_mm_setr_ps(n[0][0],n[1][0],n[2][0],n[3][0]);
		tet.value().ny=_mm_setr_ps(n[0][1],n[1][1],n[2][1],n[3][1]);
		tet.value().nz=_mm_setr_ps(n[0][2],n[1][2],n[2][2],n[3][2]);
		tet.value().C =_mm_setr_ps(C[0],C[1],C[2],C[3]);
	}

	return T;
}



// checks validity of TetraMesh construct
bool TetraMesh::checkValid(bool) const
{
	bool valid=true;

	valid &= TetraMeshBase::checkValid();

	// TODO: more checks here
	return valid;
}

bool TetraMesh::checkFaces() const
{
	bool status_ok=true;
	unsigned i=1;
	for(vector<TetraByPointID>::const_iterator it=T_p.begin()+1; it != T_p.end(); ++it,++i){
		for(int j=0;j<4;++j)
		{
			unsigned pID = (*it)[tetra_face_opposite_point_indices[j].oppidx];
			Point<3,double> pt=P[pID];
			int f=T_f[i][j];
			double h;
			if ((h=(f<0?-1:1)*F[abs(f)].pointHeight(pt)) < 0){
				status_ok=false;
				cout << "Error: height of opposite point to face " << f << " on tetrahedron " << i << " is negative (" << h << ")" << endl;
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

	for(vector<Point<3,double> >::const_iterator it=P.begin()+1; it != P.end(); ++it,++c)
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

	for(unsigned i=1;i<tetras.size(); ++i)
		if (tetras[i].pointWithin(pv))
		{
			IDt=i;
			++N;
			cout << "Enclosed by tetra " << i << endl;
		}

	if (IDt == 0)
		cerr << "Error: no enclosing tetra!" << endl;
	else if (N == 1)
		cout << "Single tetra only" << endl;
	else if (N > 1)
		cerr << "Warning: enclosed by " << N << " tetras" << endl;
	return IDt;
}

// verifies that a point is within the specified tetrahedron
bool Tetra::pointWithin(__m128 p) const
{
    // compute p (dot) n_i minus C_i for i in [0,3]
    __m128 dot =         _mm_mul_ps(nx,_mm_shuffle_ps(p,p,_MM_SHUFFLE(0,0,0,0)));
    dot = _mm_add_ps(dot,_mm_mul_ps(ny,_mm_shuffle_ps(p,p,_MM_SHUFFLE(1,1,1,1))));
    dot = _mm_add_ps(dot,_mm_mul_ps(nz,_mm_shuffle_ps(p,p,_MM_SHUFFLE(2,2,2,2))));
    dot = _mm_sub_ps(dot,C);

    return _mm_movemask_ps(dot) == 0;
}
//
//StepResult Tetra::getIntersection(__m128 p,__m128 d,__m128 s) const
//{
//    StepResult result;
//
//    result.idx=-1;
//
//    s = _mm_shuffle_ps(s,s,_MM_SHUFFLE(0,0,0,0));
//
//    // calculate dot = n (dot) d, height = n (dot) p - C
//    __m128 dot    =             _mm_mul_ps(nx,_mm_shuffle_ps(d,d,_MM_SHUFFLE(0,0,0,0)));
//    __m128 h1 =             _mm_mul_ps(nx,_mm_shuffle_ps(p,p,_MM_SHUFFLE(0,0,0,0)));
//
//    dot    = _mm_add_ps(dot,    _mm_mul_ps(ny,_mm_shuffle_ps(d,d,_MM_SHUFFLE(1,1,1,1))));
//    h1 = _mm_add_ps(h1, _mm_mul_ps(ny,_mm_shuffle_ps(p,p,_MM_SHUFFLE(1,1,1,1))));
//
//    dot    = _mm_add_ps(dot,    _mm_mul_ps(nz,_mm_shuffle_ps(d,d,_MM_SHUFFLE(2,2,2,2))));
//    h1 = _mm_add_ps(h1, _mm_mul_ps(nz,_mm_shuffle_ps(p,p,_MM_SHUFFLE(2,2,2,2))));
//
//    // height (=C - p dot n) should be negative if inside tetra, may occasionally be (small) positive due to numerical error
//    // dot negative means facing outwards
//    h1 = _mm_sub_ps(C,h1);
//
//
//    // dist = height/dot
//    __m128 dist = _mm_div_ps(h1,dot);
//
////  selects dist where dist>0 and dot<0 (facing outwards), s otherwise
//    // very, very rarely ( < 1e-8? ) gives an error where no intersection is found
//    // used to be s below instead of infinity - would return at most s; gave wrong nearest-face results though
//    // dist = _mm_blendv_ps(a,b,mask)
//    //  dist_i = (mask_i & 0x80000000) ? b_i : a_i;
//    dist = _mm_blendv_ps(_mm_set1_ps(std::numeric_limits<float>::infinity()),dist,_mm_and_ps(_mm_cmpgt_ps(dist,_mm_setzero_ps()),dot));
//
//    // at most three of the dot products should be negative
//    // ideally none of the heights should be negative (assuming we're in the tetra)
//
//    //      height  dot     h/dot   meaning
//    //      +       +       +       OK: inside, facing away (no possible intersection)
//    //      +       -       -       OK: inside, facing towards (intersection possible)
//    //      -       +       -       OK: outside, facing in (this is the entry face with roundoff error, no possible intersection)
//    //      -       -       +       ERROR: outside, facing out (problem!! this must be the entry face, but exiting!)
//
//    // require p dot n - C > 0 (above face) and d dot n < 0
//
//    pair<unsigned,__m128> min_idx_val = getMinIndex4p(dist);
//
//    result.hit = _mm_ucomilt_ss(min_idx_val.second,s);
//
//
//    result.IDfe = IDfs[min_idx_val.first&3];
//    result.IDte = adjTetras[min_idx_val.first&3];
//    result.idx = min_idx_val.first;					// will be 4 if no min found
//    result.distance=_mm_min_ps(min_idx_val.second,s);
//    result.Pe = _mm_add_ps(p,_mm_mul_ps(d,result.distance));
//
//    return result;
//}

// check linear combination of points to verify we're within tetra
//   used only for testing, very slow
bool TetraMesh::isWithinByPoints(int tID,const Point<3,double>& p) const
{
    float M[3][4];
    const Point<3,double> &A=P[T_p[tID][0]], &B=P[T_p[tID][1]], &C=P[T_p[tID][2]], &D=P[T_p[tID][3]];
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

// get the surface element hit by an incoming ray

// alg needs improvement; currently scans all boundary faces
// returns a pair of <tetraID,faceID>
//pair<pair<unsigned,int>,Point<3,double> > TetraMesh::getSurfaceElement(const Ray<3,double>& r) const
//{
//    const UnitVector<3,double> &d=r.getDirection();
//    const Point<3,double> &p=r.getOrigin();
//    Point<3,double> Q;
//    int IDf=0;
//    unsigned IDt=0;
//    double t,t_min = std::numeric_limits<double>::infinity();
//
//    for(map<unsigned,unsigned>::const_iterator it=F_boundary_ID.begin(); it != F_boundary_ID.end(); ++it)
//    {
//        Point<3,double> A(P[F_p[it->first][0]]),B(P[F_p[it->first][1]]),C(P[F_p[it->first][2]]);
//        Vector<3,double> OA(p,A),OB(p,B),OC(p,C);
//        Vector<3,double> AB(A,B),AC(A,C);
//        if (F[it->first].pointHeight(p) < 0 &&            // make sure we're on correct side
//            dot(F[it->first].getNormal(),d) > 0        // and face is directed the correct way
//            )
//            {
//                double c1,c2;
//                pair<bool,Point<3,double> > tmp(F[it->first].rayIntersectPoint(r,t_min));
//                Vector<3,double> PQ(p,tmp.second);
//                if ((t=PQ.norm_l2()) < t_min){
//                    Vector<3,double> AQ(A,tmp.second);
//                    c1 = dot(AB,AQ)/AB.norm2_l2();
//                    c2 = dot(Vector<3,double>(PQ-AB*c1),AC)/AC.norm2_l2();
//                    // check that intersection point is within the face
//
//                    if (0.0 <= c1 && c1 <= 1.0 && 0.0 <= c2 && c2 <= 1.0 && c1+c2 <= 1.0)
//                    {
//                        t_min = t;
//                        IDf=it->first;
//                        Q=tmp.second;
//                    }
//                }
//            }
//    }
//
//    assert(IDf>0);
//    IDt = vecFaceID_Tetra[IDf].first;
//
//    return make_pair(make_pair(IDt,IDf),Q);
//}



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

	for(unsigned i=0;i<F.size();++i)
	{
		if (i == abs(IDf_exclude))
			continue;

		FaceByPointID IDps = F_p[i];
		Point<3,double> T[3]{
			P[IDps[0]],
			P[IDps[1]],
			P[IDps[2]]
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


bool sameOrientation(FaceByPointID f0,FaceByPointID f1)
{
    return f0.orderCount() == f1.orderCount();
}

unsigned TetraMesh::getTetraFromFace(int IDf) const
{
    return IDf > 0 ? F_t[IDf][0] : F_t[-IDf][1];
}

double TetraMesh::getFaceArea(const FaceByPointID& f) const
{
    Vector<3,double> AB(P[f[0]],P[f[1]]);
    Vector<3,double> AC(P[f[0]],P[f[2]]);

    return cross(AB,AC).norm_l2()/2;
}
//
//pair<unsigned,boost::shared_array<const uint8_t> > TetraMesh::pointsAsBinary() const
//{
//    // create large object for points
//    unsigned Nb = 3*sizeof(double)*getNp();
//    double *p=new double[3*getNp()],*q=p;
//
//    for(point_const_iterator it=pointBegin(); it != pointEnd(); ++it)
//    {
//        *(q++) = (*it)[0];
//        *(q++) = (*it)[1];
//        *(q++) = (*it)[2];
//    }
//    return make_pair(Nb,boost::shared_array<const uint8_t>((const uint8_t*)p));
//}
//
//pair<unsigned,boost::shared_array<const uint8_t> > TetraMesh::tetrasAsBinary() const
//{
//    unsigned Nb,Nt = getNt();
//    unsigned *t = new unsigned[5*Nt],*u = t;
//    Nb=5*sizeof(unsigned)*Nt;
//    vector<unsigned>::const_iterator Mit=T_m.begin()+1;
//
//    for(tetra_const_iterator it=tetraIDBegin(); it != tetraIDEnd(); ++it)
//    {
//        *(u++) = (*it)[0];
//        *(u++) = (*it)[1];
//        *(u++) = (*it)[2];
//        *(u++) = (*it)[3];
//        *(u++) = *(Mit++);
//    }
//    return make_pair(Nb,boost::shared_array<const uint8_t>((const uint8_t*)t));
//}

// TODO: Add more integrity checks
bool TetraMesh::checkIntegrity(bool printResults) const
{
    bool status_ok=true,status_fp=true;

    if (printResults)
        cout << "Checking face ordering in F_p - " << flush;
    for(vector<FaceByPointID>::const_iterator it=F_p.begin()+1; it != F_p.end(); ++it)
        status_fp &= ((*it)[0] < (*it)[1]) & ((*it)[1] < (*it)[2]);

    if (printResults)
        cout << (status_fp ? "OK" : "Error") << endl;
    status_ok &= status_fp;


    if (printResults)
        cout << "Integrity check complete - status " << (status_ok ? "OK" : "Error") << endl;

    return status_ok;
}


vector<unsigned> TetraMesh::getRegionBoundaryTris(unsigned r) const
{
	vector<unsigned> tri;
    for(unsigned IDf=0; IDf<F.size(); ++IDf)
    	if(faceBoundsRegion(r,IDf))
    		tri.push_back(IDf);

    return tri;
}

/** returns a vector containing (face ID, tetra ID) for each face on the boundary of r0; if r1 is specified, then
 * it also requires that the face bound r1 (ie. is an r0-r1 boundary)
 *
 * The tetra ID returned is the tetra in region r0.
 */

vector<pair<unsigned,unsigned>> TetraMesh::getRegionBoundaryTrisAndTetras(unsigned r0,unsigned r1) const
{
	vector<pair<unsigned,unsigned>> v;

	for(unsigned i=0;i<F_t.size();++i)
	{
		unsigned Ta = F_t[i][0], Tb = F_t[i][1];
		assert(Ta < T_m.size() && Tb < T_m.size());

		unsigned ma = T_m[Ta], mb = T_m[Tb];

		if (ma == mb)														// no boundary
			continue;
		else if (ma == r0 && (r1==-1U || mb==r1))							// boundary; Ta is within region
			v.push_back(make_pair(i,Ta));
		else if (mb == r0 && (r1 ==-1U || ma == r1))						// boundary; Tb is within region
			v.push_back(make_pair(i,Tb));
	}

	return v;
}


TetraMesh* buildMesh(const TetraMeshBase& M)
{
	return new TetraMesh(M);
}
