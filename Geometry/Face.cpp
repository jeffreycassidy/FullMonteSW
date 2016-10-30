#include <cassert>

#include "Face.hpp"

using namespace std;

template<class T>int signum(T a)
{
    return (a>T(0)) - (a<T(0));
}


double Face::pointHeight(const Point<3,double>& P) const
{
	return dot((Vector<3,double>)P,normal)-C;
}

// creates a Face with normal and constant by using cross-product
// face coordinates should be given in clockwise order, such that the remaining point is above AB x AC
Face::Face(const Point<3,double>& Pa,const Point<3,double>& Pb,const Point<3,double>& Pc) :
    normal(cross(Vector<3,double>(Pa,Pb),Vector<3,double>(Pa,Pc))),
    C (dot(normal,Vector<3,double>(Pa)))
    { }
//
//// creates a Face from four points, ensuring that AD (dot) n > 0, ie A is above ABC
//Face::Face(const Point<3,double>& Pa,const Point<3,double>& Pb,const Point<3,double>& Pc,const Point<3,double>& Pd) :
//	normal(cross(Vector<3,double>(Pa,Pb),Vector<3,double>(Pa,Pc))),
//	C (dot(normal,Vector<3,double>(Pa)))
//{
//	if (pointHeight(Pd) < 0)
//		flip();
//}

// Intersects a ray with the face plane
// NOTE: does not guarantee that the point is actually inside the face points
pair<bool,Point<3,double> > Face::rayIntersectPoint(const Ray<3,double>& r,bool invert) const
{
	double costheta=-dot(r.getDirection(),normal);
	double h=pointHeight(r.getOrigin());
	if (signum(costheta)!=signum(h))
		return make_pair(false,r.getOrigin());
	return make_pair(true,r(h/costheta*(invert ? -1 : 1)));
}

// checks if a ray intersects the face plane within t units of travel
// NOTE: does not guarantee that the point is actually inside the face points
pair<bool,double> Face::rayIntersect(const Ray<3,double>& r,double t,bool invert) const
{
	double costheta=-dot(r.getDirection(),normal);
	double h=pointHeight(r.getOrigin());
	if(signum(costheta)!=signum(h) || (invert ^ (h >= t*costheta)))
		return make_pair(false,t);
	else
		return make_pair(true,h/costheta);
}
