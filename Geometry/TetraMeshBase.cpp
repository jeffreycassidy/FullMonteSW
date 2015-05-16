#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include "TetraMeshBase.hpp"

// Very naive search to find closest point
//unsigned TetraMeshBase::findNearestPoint(const Point<3,double>& p) const
//{
//	double d2=numeric_limits<double>::infinity(),t;
//	unsigned id=0,c=1;
//
//	for(vector<Point<3,double> >::const_iterator it=P.begin()+1; it != P.end(); ++it,++c)
//	{
//		if ((t=norm2_l2(Vector<3,double>(*it,p))) < d2)
//		{
//			d2 = t;
//			id = c;
//		}
//	}
//	return id;
//}
//
//// check linear combination of points to verify we're within tetra
////   used only for testing, very slow
//bool TetraMeshBase::isWithinByPoints(int tID,const Point<3,double>& p) const
//{
//    float M[3][4];
//    const Point<3,double> &A=P[T_p[tID][0]], &B=P[T_p[tID][1]], &C=P[T_p[tID][2]], &D=P[T_p[tID][3]];
//    Vector<3,double> e[3];
//
//    // calculate edge vectors
//    e[0]=B-A;
//    e[1]=C-A;
//    e[2]=D-A;
//
//    // build basis matrix for tetra
//    M[0][0] = e[0][0];
//    M[1][0] = e[0][1];
//    M[2][0] = e[0][2];
//    M[0][3] = p[0]-A[0];
//
//    M[0][1] = e[1][0];
//    M[1][1] = e[1][1];
//    M[2][1] = e[1][2];
//    M[1][3] = p[1]-A[1];
//
//    M[0][2] = e[2][0];
//    M[1][2] = e[2][1];
//    M[2][2] = e[2][2];
//    M[2][3] = p[2]-A[2];
//
//    double c;
//
//    // eliminate
//    for(unsigned i=0;i<3;++i)
//    {
//        // subtract rows above
//        for(unsigned j=0;j<i;++j)
//        {
//            c=M[i][j];
//            for(unsigned k=0;k<4;++k)
//                M[i][k] -= c*M[j][k];
//        }
//
//        // normalize the row
//        c=M[i][i];
//        for(unsigned j=i;j<4;++j)
//            M[i][j] /= c;
//    }
//
//    // backsub
//    for(int i=1;i>=0;--i)
//        for(unsigned j=i+1;j<3;++j)
//        {
//            c=M[i][j];
//            for(unsigned k=0;k<4;++k)
//                M[i][k] -= c*M[j][k];
//        }
//
//    float coeff[4];
//    coeff[0]=1.0;
//
//    for(unsigned i=1;i<4;++i)
//    {
//        coeff[i]=M[i-1][3];
//        coeff[0] -= coeff[i];
//    }
//
//    printf("Coeffs are: %9.5f %9.5f %9.5f %9.5f ",coeff[0],coeff[1],coeff[2],coeff[3]);
//
//    bool within=true,onedge=false;
//
//    for(unsigned i=0;i<4;++i)
//    {
//        within &= (coeff[i] > -1e-4) & (coeff[i] < 1+1e-4);
//        onedge |= (abs(coeff[i]) < 1e-4);
//    }
//
//    cout << " - " << (within ? (onedge ? "on edge" : "within") : "OUTSIDE") << endl;
//
//    return within;
//}

bool TetraMeshBase::checkValid(bool printResults) const
{
	bool IDps_ok=true;

	// check tetra point indices: all should be nonzero and fall within the size of the points vector

    for(TetraByPointID IDps : T_p)
    	for(unsigned IDp : IDps)
    	{
    		if (IDp >= P.size())
    		{
    			if (printResults)
    				cout << "  ERROR: Point index out of range (max " << P.size() << ") in Tetra: " << IDps << endl;
    			IDps_ok=false;
    		}
    	}
    cout << "Tetra point indices in range: " << (IDps_ok ? "OK" : "Error") << endl;


    return IDps_ok;
}

void TetraMeshBase::remapMaterial(unsigned from,unsigned to)
{
	for(unsigned& m : T_m)
		if (m == from)
			m = to;
}



/*
	readFileMatlabTP(fn,ids,coords)

Args
	fn			File name to open

Returns
	ids			Vector of TetraByPointID, specifies the tetrahedra in terms of their points
	coords		Vector of Point<3,double> giving the actual point coordinates

	pair<int,int> contains (number of faces, number of points)


	NOTE: Arrays are 1-based so they will agree with Matlab/Octave
			0 is used as a special value (for not-found or exited)

*/


//bool TetraMeshBase::readFileMatlabTP(string fn)
//{
//	ifstream is(fn.c_str(),ios_base::in);
//
//    if(!is.good()){
//        cerr << "Failed to open " << fn << " for reading; abort" << endl;
//        return false;
//    }
//	int Nt,Np;
//
//    // read sizes
//    is >> Np;
//	is >> Nt;
//
//	// read point coordinates -- uses 1-based addressing
//	P.resize(Np+1);
//	P[0]=Point<3,double>();
//	for (vector<Point<3,double> >::iterator it = P.begin()+1; it != P.end(); ++it)
//		is >> *it;
//
//	T_p.resize(Nt+1);
//    T_m.resize(Nt+1);
//	unsigned t[4]={0,0,0,0},i=1,max_m=0;
//	T_p[0]=TetraByPointID(t);
//    T_m[0]=0;
//    TetraByPointID IDps;
//	for (vector<TetraByPointID>::iterator it=T_p.begin()+1; it != T_p.end(); ++it,++i)
//	{
//		is >> IDps;
//		is >> T_m[i];
//        *it=IDps.getSort();
//		max_m = max(max_m,T_m[i]);
//	}
//
//	return true;
//}
//
