#include "newgeom.hpp"

// Points are ABC clockwise, with D above ABC
// Faces are 0=ABC, 1=ACD, 2=ADB, 3=BDC

// for a triangle with points T[0..2], determines if P+td is in the triangle for some t

// tries to solve A + c0(B-A) + c1(C-A) = P - c2*d
// equivalently (1-c0-c1)A + c0(B) + c1(C) + c2(d) = P
bool PointInTriangle(Point<3,double> P,UnitVector<3,double> d,Point<3,double> T[3],Point<3,double>& Q,double& t)
{
    double a[3] = { T[1][0]-T[0][0], T[1][1]-T[0][1], T[1][2]-T[0][2] };    // vector from point T[0] to T[1]
    double b[3] = { T[2][0]-T[0][0], T[2][1]-T[0][1], T[2][2]-T[0][2] };    // vector from T[0] to T[2]
    double M[3][3] = { { a[0],b[0],-d[0] }, {a[1],b[1],-d[1]}, {a[2],b[2],-d[2]} };

    // make determinant
    double det = M[0][0]*(M[2][2]*M[1][1]-M[2][1]*M[1][2])
        -M[1][0]*(M[2][2]*M[0][1]-M[2][1]*M[0][2])
        +M[2][0]*(M[1][2]*M[0][1]-M[1][1]*M[0][2]);

    // make inverse (needs to multiplied by 1/det)
    double inv[3][3] = { { M[2][2]*M[1][1]-M[2][1]*M[1][2], -(M[2][2]*M[0][1]-M[2][1]*M[0][2]), M[1][2]*M[0][1]-M[1][1]*M[0][2] },
        { -(M[2][2]*M[1][0]-M[2][0]*M[1][2]), M[2][2]*M[0][0]-M[2][0]*M[0][2], -(M[1][2]*M[0][0]-M[1][0]*M[0][2]) },
        {   M[2][1]*M[1][0]-M[2][0]*M[1][1],-(M[2][1]*M[0][0]-M[2][0]*M[0][1]),  M[1][1]*M[0][0]-M[1][0]*M[0][1] } };

    double A[3] = { P[0]-T[0][0], P[1]-T[0][1],P[2]-T[0][2] };

    double c[3] = { (inv[0][0]*A[0]+inv[0][1]*A[1]+inv[0][2]*A[2])/det,
        (inv[1][0]*A[0]+inv[1][1]*A[1]+inv[1][2]*A[2])/det,
        (inv[2][0]*A[0]+inv[2][1]*A[1]+inv[2][2]*A[2])/det };

    // c[0], c[1] = coefficients for AB, AC respectively
    // c[2] = distance from P to intersection along d

    double tmp2[3] = {
        (1-c[0]-c[1])*T[0][0] + c[0]*T[1][0] + c[1]*T[2][0],
        (1-c[0]-c[1])*T[0][1] + c[0]*T[1][1] + c[1]*T[2][1],
        (1-c[0]-c[1])*T[0][2] + c[0]*T[1][2] + c[1]*T[2][2]};
    Point<3,double> q_test(tmp2);


    // test that P = (1-c0-c1)A + c0 B + c1 C - c2 (d)
    //   (ie. that travelling c2 units from d gets you to the face

    double tmp[3] = {
        (1-c[0]-c[1])*T[0][0] + c[0]*T[1][0] + c[1]*T[2][0] - c[2]*d[0],
        (1-c[0]-c[1])*T[0][1] + c[0]*T[1][1] + c[1]*T[2][1] - c[2]*d[1],
        (1-c[0]-c[1])*T[0][2] + c[0]*T[1][2] + c[1]*T[2][2] - c[2]*d[2]};

    Point<3,double> p_test(tmp);

    bool is_inside= (c[0] <= 1.0) & (c[0] >= 0.0) & (c[1] <= 1.0) & (c[1] >= 0.0) && (c[0]+c[1] <= 1.0);

    if(is_inside)
    {
        t=c[2];
        Q=q_test;
    }

/*    cout << "P=" << P << endl;
    cout << " Coeffs " << c[0] << ' ' << c[1] << ' ' << c[2] << (is_inside ? " IN " : " OUT")  << endl;
    cout << " Q_test= " << q_test << endl << " P_test=" << p_test << endl;*/

    return is_inside;
}

FaceByPointID TetraByPointID::getFace(unsigned faceNum)
{
	unsigned tmp[3];
	copy(begin(),end(),tmp);
	switch(faceNum){
		case 0: break;
		case 1: tmp[1]=(*this)[2]; tmp[2]=(*this)[3]; break;
		case 2: tmp[1]=(*this)[3]; tmp[2]=(*this)[1]; break;
		case 3: tmp[0]=(*this)[1]; tmp[1]=(*this)[3]; break;
		default: assert(0);
	}
	return FaceByPointID(tmp);
}

//unsigned TetraByPointID::getOppositePoint(unsigned faceNum) const
//{
//	switch(faceNum){
//		case 0: return (*this)[3];
//		case 1: return (*this)[1];
//		case 2: return (*this)[2];
//		case 3: return (*this)[0];
//		default: assert(0);
//	}
//    return -1;
//}

UnitVector<3,double> uvect3FromPolar(double phi,double lambda)
{
    double p[3] = { cos(phi)*sin(lambda), cos(phi)*cos(lambda), sin(phi) };
    return UnitVector<3,double>(p);
}

GeomManip plainwhite=GeomManip::plainwhite();

GeomManip operator<<(ostream& os,const GeomManip& gm_)
{
    GeomManip gm(os);
    gm.delimchar = gm_.delimchar;
    gm.parens = gm_.parens;
    gm.parenchar[0] = gm_.parenchar[0];
    gm.parenchar[1] = gm_.parenchar[1];
    gm.uvparenchar[0] = gm_.uvparenchar[0];
    gm.uvparenchar[1] = gm_.uvparenchar[1];
    gm.idparenchar[0] = gm_.idparenchar[0];
    gm.idparenchar[1] = gm_.idparenchar[1];
    return gm;
}

/*
Point<3,double> pointFrom(__m128 p)
{
    Point<3,double> P;
    float tmp_f[4];
    _mm_store_ps(tmp_f,p);

    for(unsigned i=0;i<3;++i)
        P[i]=tmp_f[i];
    return P;
}

Ray<3,double> rayFrom(__m128 p,__m128 d)
{
    Point<3,double> pv;
    float tmp_f[4];
    double tmp_d[4];
    _mm_store_ps(tmp_f,d);
    for(unsigned i=0;i<3;++i)
        tmp_d[i]=tmp_f[i];
    UnitVector<3,double> dv(tmp_d,true);

    _mm_store_ps(tmp_f,p);
    for(unsigned i=0;i<3;++i)
        pv[i] = tmp_f[i];
    return Ray<3,double>(pv,dv);
}

UnitVector<3,double> uvectFrom(__m128 v)
{
    float f[4];
    _mm_store_ps(f,v);
    double d[3];
    for(unsigned i=0;i<3;++i)
        d[i]=f[i];
    return UnitVector<3,double>(d);
}
*/
/*template<>Ray<3,double>::operator Packet() const
{
    Packet p;

    float f[4];
    f[0] = P[0];
    f[1] = P[1];
    f[2] = P[2];
    f[3] = 0;

    p.p=_mm_load_ps(f);

    f[0] = d[0];
    f[1] = d[1];
    f[2] = d[2];
    p.setDirection(_mm_load_ps(f));
    return p;
}*/

// Compute dot product
// D=number of dimensions (1..4); O = output mask
/*template<unsigned D,unsigned O=0xf>__m128 dot(__m128 a,__m128 b)
{
	return _mm_dp_ps(a,b, (((1<<D)-1) << 4)| O);
}*/


//inline float fabs(__m128 x){ return fabs(_mm_cvtss_f32(x)); }


std::ostream& operator<<(std::ostream& os,const FaceByPointID& F)
{
	os << '(';
	for(int i=0; i<3; ++i){ os << F[i] << (i<2? ',' : ')'); }
	return os;
}


std::istream& operator>>(std::istream& is,TetraByPointID& P)
{
	is >> std::skipws;
	bool paren=false;

	if(is.peek()=='('){ paren=true; is.ignore(1); }
	for(int i=0; i<4; ++i){ is >> P[i]; if(i < 3 && is.peek()==','){ is.ignore(1); } }
	if (paren){ is.ignore(1); }
	return is;
}

std::istream& operator>>(std::istream& is,FaceByPointID& F)
{
	is >> std::skipws;
	bool paren=false;

	if(is.peek()=='('){ paren=true; is.ignore(1); }
	for(int i=0; i<3; ++i){ is >> F[i]; if(i < 3 && is.peek()==','){ is.ignore(1); } }
	if (paren){ is.ignore(1); }
	return is;
}

std::ostream& operator<<(std::ostream& os,TetraByPointID& T)
{
	return os << "Tetra <" << T[0] << ',' << T[1] << ',' << T[2] << ',' << T[3] << '>';
}


std::ostream& operator<<(std::ostream& os,FaceByPointID& F)
{
	return os << "Face <" << F[0] << ',' << F[1] << ',' << F[2] << '>';
}
