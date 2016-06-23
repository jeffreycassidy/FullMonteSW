/*
 * Test_AffineTransform.cpp
 *
 *  Created on: Apr 3, 2016
 *      Author: jcassidy
 */


#include "AffineTransform.hpp"

#define BOOST_TEST_MODULE Test_AffineTransform
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <FullMonteHW/Geometry/Basis.hpp>

typedef AffineTransform<float,3> AffineTransform3f;

BOOST_AUTO_TEST_CASE(scale)
{
	AffineTransform3f T = AffineTransform3f::scale(2.0,std::array<float,3>{ 1.0, 2.0, 3.0});

	BOOST_CHECK_SMALL(norm(T(std::array<float,3>{1.0f, 2.0f, 3.0f}) - std::array<float,3>{1.0f,2.0f, 3.0f}),1e-6f);
	BOOST_CHECK_SMALL(norm(T(std::array<float,3>{2.0f, 1.0f,-3.0f}) - std::array<float,3>{3.0f,0.0f,-9.0f}),1e-6f);
}
