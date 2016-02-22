/*
 * Test_OutputTypes.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test_OutputTypes

#include <boost/test/unit_test.hpp>

#include "MCConservationCounts.hpp"
#include "MCEventCounts.hpp"
#include "OutputData.hpp"

#include "OutputDataSummarize.hpp"

#include <iostream>

using namespace std;

BOOST_AUTO_TEST_CASE(counts)
{
	MCConservationCountsOutput cc;
	MCEventCountsOutput ec;

	OutputDataSummarize summ(cout);

	summ.visit(&cc);
	summ.visit(&ec);
}


