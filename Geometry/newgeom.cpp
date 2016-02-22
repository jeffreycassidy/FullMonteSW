#include "newgeom.hpp"


/** Determines if ray defined by (P,d) intersects the triangle defined by three points T[]
 * If so, returns the point of intersection q and the distance along the ray, t
 *
 * Let AB = e1 = T[1]-T[0], AC = e2=T[2]-T[0]
 *
 * Solves P + td = A + u*AB + v*AC for (t,u,v) and checks if 0 <= u,v,u+v <= 1 and t>=0
 *
 */

PointIntersectionResult RayTriangleIntersection(Point<3,double> P,UnitVector<3,double> d,Point<3,double> T[3])
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

    // u=c[0], v=c[1], t=c[2]

    PointIntersectionResult res;

    res.intersects = c[2] >= 0 && (c[0] <= 1.0) & (c[0] >= 0.0) & (c[1] <= 1.0) & (c[1] >= 0.0) && (c[0]+c[1] <= 1.0);

    if(res.intersects)
    {
        res.t=c[2];
        res.q=Point<3,double>{
        	(1-c[0]-c[1])*T[0][0] + c[0]*T[1][0] + c[1]*T[2][0],
            (1-c[0]-c[1])*T[0][1] + c[0]*T[1][1] + c[1]*T[2][1],
            (1-c[0]-c[1])*T[0][2] + c[0]*T[1][2] + c[1]*T[2][2] };
    }

    return res;
}



FaceByPointID TetraByPointID::getFace(unsigned faceNum)
{
	std::array<unsigned,3> tmp;
	std::copy(begin(),end(),tmp.begin());
	switch(faceNum){
		case 0: break;
		case 1: tmp[1]=(*this)[2]; tmp[2]=(*this)[3]; break;
		case 2: tmp[1]=(*this)[3]; tmp[2]=(*this)[1]; break;
		case 3: tmp[0]=(*this)[1]; tmp[1]=(*this)[3]; break;
		default: assert(0);
	}
	return FaceByPointID(tmp);
}

