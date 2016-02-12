/*
 * Test_BlockRandomDistribution.cpp
 *
 *  Created on: Feb 12, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_MODULE Test_BlockRandomDistribution
#define BOOST_TEST_DYN_LINK

#include "BlockRandomDistribution.hpp"

struct DummyConstRNGVector
{
	uint32_t 	m_val=0;

};

namespace URNG
{
	template<>void seed(DummyConstRNGVector& rng,unsigned s)
	{
		rng.m_val=s;
	}

	template<>void fill(DummyConstRNGVector& rng,uint32_t* dst,std::size_t N)
	{
		std::fill(dst,dst+N,rng.m_val);
	}
};

BOOST_AUTO_TEST_CASE(extremes)
{
	DummyConstRNGVector constrng;

	URND::seed(constrng,0);
	URND::fill(constrng,,8);

	for(unsigned i=0;i<10;++i)
		cout <<

}


