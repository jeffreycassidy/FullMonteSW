#pragma once
#include "graph.hpp"
#include "newgeom.hpp"
#include <array>

class Parallelepiped {
	std::array<std::pair<UnitVector<3,double>,std::pair<double,double>>,3> faces;

public:
	Parallelepiped(){};

	/// Construct from three normal unit vectors, each with a (low,high) range for the offset
	Parallelepiped(const std::array<std::pair<UnitVector<3,double>,std::pair<double,double>>,3>& faces_);

	/// Construct from three normal unit vectors, and a pair of points for each
	Parallelepiped(const std::array<std::pair<UnitVector<3,double>,std::pair<Point<3,double>,Point<3,double>>>,3>& i_);

	/// Standard copy constructor
	Parallelepiped(const Parallelepiped&)=default;

	/// Test point for membership in the region
	bool pointWithin(const Point<3,double>& p) const;

	/*auto membership_tester() -> decltype(std::bind(std::mem_fn(&Parallelepiped::pointWithin),this,std::placeholders::_1))
		{ return std::bind(std::mem_fn(&Parallelepiped::pointWithin),this,std::placeholders::_1); }*/

	/// Print the three normals and constant ranges
	friend ostream& operator<<(ostream&,const Parallelepiped&);
};

template<class T>auto membership_tester(const T& r) ->
		decltype(std::bind(std::mem_fn(&T::pointWithin),&r,std::placeholders::_1))
				{ return std::bind(std::mem_fn(&T::pointWithin),&r,std::placeholders::_1); }

// TODO: This shouldn't be here
double dot(const UnitVector<3,double>& a,const Point<3,double>& b);
