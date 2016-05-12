/*
 * Test_Permutation.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: jcassidy
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test_Permutation

#include <boost/test/unit_test.hpp>

#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>

#include "Permutation.hpp"

using namespace std;

BOOST_AUTO_TEST_CASE( ConstructFromBoolVector )
{
	const unsigned N=16;
	std::vector<bool> v(N,false);
	std::vector<unsigned> idx{ 0,1,2,5,8,13,14 };

	std::vector<unsigned> idxInv{ 0,1,2,-1U,-1U,3,-1U,-1U,4,-1U,-1U,-1U,-1U,5,6,-1U };

	for(unsigned i : idx)
		v[i] = true;

	Permutation<unsigned> P = Permutation<unsigned>::buildStableFromBoolRange(v);

	// basic check: correct number of elements in forward & inverse maps
	BOOST_CHECK_EQUAL(boost::size(P.forward()), idx.size());
	BOOST_CHECK_EQUAL(boost::size(P.inverse()), N);

	// should always be true for buildStableFromBoolRange
	BOOST_CHECK(P.monotonic());

	// check against known-good values
	BOOST_CHECK(boost::equal(P.forward(),idx));
	BOOST_CHECK(boost::equal(P.inverse(),idxInv));

	// internal consistency checks
	BOOST_CHECK(P.check());
}

BOOST_AUTO_TEST_CASE( ConstructFromForwardMove )
{
	const unsigned N=32;				// full set size 32
	std::vector<unsigned> p{ 23, 27, 30, 31};

	const unsigned *p_original=p.data();

	Permutation<unsigned> P = Permutation<unsigned>::buildFromForward(N,std::move(p));

	// verify that P.p_ was move-constructed
	BOOST_CHECK_EQUAL(p_original, P.forward().data());
	BOOST_CHECK_EQUAL(p.size(),0);

	BOOST_CHECK(P.check());
}

BOOST_AUTO_TEST_CASE( ConstructFromForward )
{
	const unsigned N=32;				// full set size 32
	const std::vector<unsigned> p{ 23, 27, 30, 31};

	const unsigned *p_original=p.data();

	Permutation<unsigned> P = Permutation<unsigned>::buildFromForward(N,p);

	// verify that P.p_ was move-constructed
	BOOST_CHECK(p_original != P.forward().data());
	BOOST_CHECK(p.size() != 0);

	// check consistency
	BOOST_CHECK(P.check());
}
