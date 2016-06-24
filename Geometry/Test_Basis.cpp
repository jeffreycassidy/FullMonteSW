/*
 * Test_Basis.cpp
 *
 *  Created on: Mar 7, 2016
 *      Author: jcassidy
 */


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test_Basis

#include <boost/test/unit_test.hpp>

#include <FullMonteSW/Geometry/Basis.hpp>
#include <FullMonteSW/Geometry/BasisIO.hpp>

#include <iostream>
#include <iomanip>

using namespace std;

#define SELF_CROSS(v) cout << v << " x " << v << " = " << cross(v,v) << endl;

#define CHECK_VECTOR_CLOSE(lhs,rhs,eps)		BOOST_CHECK_SMALL(norm(lhs-rhs),eps)
#define CHECK_UNIT(v,eps)					BOOST_CHECK_SMALL(norm(v)-1,eps)
#define CHECK_ORTHOGONAL(lhs,rhs,eps)		BOOST_CHECK_SMALL(dot(lhs,rhs),eps)

BOOST_AUTO_TEST_CASE(ijk)
{
	Basis B = Basis::standard();

	Point3 i{1,0,0};
	Point3 j{0,1,0};
	Point3 k{0,0,1};

	cout << B << endl;

	cout << B.invert(i) << endl;
	cout << B.invert(j) << endl;
	cout << B.invert(k) << endl;

	B.origin(Point3{1,2,3});

	cout << B << endl;

	cout << B.invert(i) << endl;
	cout << B.invert(j) << endl;
	cout << B.invert(k) << endl;

	SELF_CROSS(i)
	SELF_CROSS(j)
	SELF_CROSS(k)

	CHECK_VECTOR_CLOSE(cross(i,j), k,1e-6f);
	CHECK_VECTOR_CLOSE(cross(j,i),-k,1e-6f);
	CHECK_VECTOR_CLOSE(cross(i,k),-j,1e-6f);
	CHECK_VECTOR_CLOSE(cross(k,j),-i,1e-6f);
	CHECK_VECTOR_CLOSE(cross(j,k), i,1e-6f);
	CHECK_VECTOR_CLOSE(cross(k,j),-i,1e-6f);

	cout << i << " x " << j << " = " << cross(i,j) << endl;
	cout << j << " x " << i << " = " << cross(j,i) << endl;
	cout << i << " x " << k << " = " << cross(i,k) << endl;
	cout << k << " x " << i << " = " << cross(k,i) << endl;
	cout << j << " x " << k << " = " << cross(j,k) << endl;
	cout << k << " x " << j << " = " << cross(k,j) << endl;
};

BOOST_AUTO_TEST_CASE(knee)
{
	Point3 src{55.67f, -67.5f, 135.45f };	// posterior source location
	Point3 dst{53.7f, -59.7f, 135.3f };		// joint centre
	Point3 up{.0f,.0f,1.f};					// up vector from joint centre

	Basis B = Basis::standard();
	B.origin(Point3{1.f,1.f,1.f});

	UnitVector3 dir_in = normalize(dst-src);
	UnitVector3 dir_up = normalize(up-dst);
	CHECK_UNIT(dir_in,1e-6f);
	CHECK_UNIT(dir_up,1e-6f);
	// dir_in not necessarily orthogonal to dir_up

	UnitVector3 dir_lat = normalize(cross(dir_in,dir_up));
	CHECK_UNIT(dir_lat,1e-6f);
	CHECK_ORTHOGONAL(dir_lat,dir_in,1e-6f);
	CHECK_ORTHOGONAL(dir_lat,dir_up,1e-6f);
	// dir_lat now orthogonal to both dir_in and dir_up

	UnitVector3 dir_y = cross(dir_lat,dir_in);
	CHECK_UNIT(dir_y,1e-6f);
	CHECK_ORTHOGONAL(dir_lat,dir_y,1e-6f);
	CHECK_ORTHOGONAL(dir_in,dir_y,1e-6f);
	// now we have an orthonormal basis


	// relative source coordinates

	float r=1.75f;

	std::array<Point2,5> source905{
		Point2{ .0f,	.0f},
		Point2{ r,	.0f},
		Point2{-r,	.0f},
		Point2{	.0f,r},
		Point2{ .0f,-r}
	};

	const float delta = std::sqrt(0.5f)*r;

	std::array<Point2,4> source660{
		Point2{delta,delta},
		Point2{delta,-delta},
		Point2{-delta,delta},
		Point2{-delta,-delta}
	};

	cout << "lat = (" << dir_lat << ")" << endl;
	cout << "  y = (" << dir_y   << ")" << endl;;
	cout << " in = (" << dir_in  << ")" << endl;

	B = Basis(dir_lat,dir_y,dir_in,Vector3{.0f,.0f,.0f});

	cout << "905 sources:" <<endl;

	for(const Point2 pr : source905)
		cout << "rel (" << pr << ") -> (" << B.invert(pr) << ")" << endl;

	cout << "660 sources: " << endl;

	for(const Point2 pr : source660)
		cout << "rel (" << pr << ") -> (" << B.invert(pr) << ")" << endl;
};



// source: +-x +-y +-z extend

// plane: 6DOF (x,y,z,3 angles)

//
//class PlaneConstrained
//{
//public:
//	Vector2		positionRel();
//	Vector3		position();
//
//private:
//	Vector2		m_pos;
//};
//
//class PlaneConstrainedLine
//{
//private:
//	std::pair<FT,FT>	m_depth;
//};
//
//class PlaneConstrainedPoint
//{
//private:
//	float		m_depth;
//};
