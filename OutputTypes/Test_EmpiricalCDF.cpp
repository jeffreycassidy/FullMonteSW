/*
 * Test_EmpiricalCDF.cpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */

#include "EmpiricalCDF.hpp"

#include <vector>
#include <utility>

#define BOOST_TEST_MODULE Test_EmpiricalCDF
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

BOOST_AUTO_TEST_CASE(simpleCDF_FF)
{
	vector<std::pair<float,float>> v{
		make_pair(1.0f,1.0f),
		make_pair(3.0f,1.0f),
		make_pair(0.0f,0.0f),
		make_pair(2.0f,1.0f)
	};

	EmpiricalCDF<float,float> CDF(v);

	BOOST_CHECK_EQUAL(CDF.dim(),4);

	BOOST_CHECK_CLOSE(CDF.totalWeight(),3.0f,1e-6f);

	CDF.print();
}
