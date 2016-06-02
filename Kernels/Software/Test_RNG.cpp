/*
 * Test_RNG.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_MODULE Test_RNG
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#define SFMT_MEXP 19937
#include <FullMonteSW/SFMT/SFMT.h>

#include "BlockRNGAdaptor.hpp"
#include "SFMTWrapper.hpp"

BOOST_AUTO_TEST_CASE(sfmtSeq)
{
	sfmt_t ref;

	const uint32_t seed=1;

	const unsigned refBlockSize=1024;
	const unsigned dutBlockSize=2048;

	BlockRNGAdaptor<sfmt_t,uint32_t,dutBlockSize,8,32>	dut;

	dut.seed(seed);
	sfmt_init_gen_rand(&ref,seed);

	uint32_t oRef[refBlockSize], oDut[8];

	for(unsigned i=0;i<1000000;i += 8)
	{
		if (i % refBlockSize == 0)
			sfmt_fill_array32(&ref,oRef,refBlockSize);

		if (i % 8 == 0)
		{
			const uint32_t* p = dut.getBlock();
			std::copy(p,p+8,oDut);
		}

		BOOST_CHECK_EQUAL(oRef[i % refBlockSize], oDut[i%8]);
	}
}


