#pragma once

#include "newgeom.hpp"

using namespace std;
 
class Face
{
public:
	Face() : C(std::numeric_limits<double>::quiet_NaN()) {};
    Face(const Point<3,double>&,const Point<3,double>&,const Point<3,double>&);
	Face(const Point<3,double>&,const Point<3,double>&,const Point<3,double>&,const Point<3,double>&);

	// flip the face orientation
	void flip(){ C=-C; normal=-normal; }

    // get the normal
    UnitVector<3,double> getNormal() const { return normal; }
    double getConstant() const { return C; }

	// query points
	double pointHeight(const Point<3,double>&) const;

	// Test if a ray intersects the face
	pair<bool,double>           rayIntersect(const Ray<3,double>&,double=numeric_limits<double>::infinity(),bool=false) const;

    // Gets the point where a ray intersects the face (returns false if no intersection)
	pair<bool,Point<3,double> > rayIntersectPoint(const Ray<3,double>&,bool=false) const;

    Face operator-() const { Face f; f.normal = -normal; f.C=-C; return f; }

	// get projections of a vector onto the face
	Vector<3,double> project(const Vector<3,double>&) const;
	Vector<3,double> normalComponent(const Vector<3,double>&) const;
	pair<UnitVector<2,double>,UnitVector<3,double> > reflectionBasis(const UnitVector<3,double>&,bool=false) const;

    double nx() const { return normal[0]; }
    double ny() const { return normal[1]; }
    double nz() const { return normal[2]; }
    double nC() const { return C; }

private:
	// plane definition
	UnitVector<3,double> normal;        // normal points into the tetrahedron
	double               C;
};
