/*
 * Test_BlockRandomDistribution.cpp
 *
 *  Created on: Feb 12, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_MODULE Test_BlockRandomDistribution
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "BlockRandomDistribution.hpp"

#include "BlockRNGAdaptor.hpp"

#include <boost/random/mersenne_twister.hpp>
#include <boost/range/algorithm.hpp>

#include <array>

struct DummyConstRNGVector
{
	uint32_t 	m_val=0;

	const uint32_t* getBlock()
	{
		boost::fill(m_buf,m_val);
		return m_buf.data();
	}

	std::array<uint32_t,8> m_buf;
};

namespace URNG
{
	template<class RNG>void seed(RNG&,unsigned);
	template<class RNG,typename InputType>void generate(RNG&,InputType*,std::size_t);
	template<class RNG>void clear(RNG&);

	template<>void seed(DummyConstRNGVector& rng,unsigned s)
	{
		rng.m_val=s;
	}

	template<>void generate(DummyConstRNGVector& rng,uint32_t* dst,std::size_t N)
	{
		std::fill(dst,dst+N,rng.m_val);
	}

	template<>void clear(DummyConstRNGVector& rng)
	{
	}




	template<>void seed(boost::mt19937& rng,unsigned s)
	{
		rng.seed(s);
	}

	template<>void generate(boost::mt19937& rng,uint32_t* dst,std::size_t N)
	{
		std::generate(dst,dst+N,std::ref(rng));
	}

	template<>void clear(boost::mt19937& rng)
	{
	}
};

#include <iostream>
#include <iomanip>
using namespace std;

#include "FloatU01Distribution.hpp"
#include "FloatPM1Distribution.hpp"

BOOST_AUTO_TEST_CASE(extremes)
{
	uint32_t rngbuf[8];
	float u01rnd[8];

	DummyConstRNGVector constrng;
	boost::mt19937 mtrng;

	FloatU01Distribution u01;

	for(const uint32_t s : std::vector<uint32_t>{ 0U, 1U, 511U, 512U, -1U })
	{
		URNG::seed(constrng,s);
		URNG::generate(constrng,rngbuf,8);

		u01.calculate(constrng,u01rnd);

		for(unsigned i=0;i<8;++i)
		{
			//cout << rngbuf[i] << ':' << std::scientific << u01rnd[i] << ' ';
			BOOST_CHECK(u01rnd[i] <  1.0);
			BOOST_CHECK(u01rnd[i] >= 0.0);
		}

		cout << endl;
	}


	URNG::seed(mtrng,0);
	URNG::generate(mtrng,rngbuf,8);
}

BOOST_AUTO_TEST_CASE(extremes_pm1)
{
	uint32_t rngbuf[8];
	float pm1rnd[8];

	DummyConstRNGVector constrng;
	boost::mt19937 mtrng;

	FloatPM1Distribution pm1;

	for(const uint32_t s : std::vector<uint32_t>{ 0U, 1U, 511U, 512U, -1U })
	{
		URNG::seed(constrng,s);
		URNG::generate(constrng,rngbuf,8);

		pm1.calculate(constrng,pm1rnd);

		for(unsigned i=0;i<8;++i)
		{
			cout << rngbuf[i] << ':' << std::scientific << pm1rnd[i] << ' ';
			BOOST_CHECK(pm1rnd[i] <  1.0);
			BOOST_CHECK(pm1rnd[i] >= -1.0);
		}

		cout << endl;
	}


	URNG::seed(mtrng,0);
	URNG::generate(mtrng,rngbuf,8);
}

#include "FloatUnitExpDistribution.hpp"

BOOST_AUTO_TEST_CASE(extremes_exprnd)
{
	uint32_t rngbuf[8];
	float exprnd[8];

	DummyConstRNGVector constrng;
	boost::mt19937 mtrng;

	FloatUnitExpDistribution exp;

	cout << "Unit exponential" << endl;

	for(const uint32_t s : std::vector<uint32_t>{ 0U, 1U, 511U, 512U, -1U })
	{
		URNG::seed(constrng,s);
		URNG::generate(constrng,rngbuf,8);

		exp.calculate(constrng,exprnd);

		for(unsigned i=0;i<8;++i)
		{
			cout << rngbuf[i] << ':' << std::scientific << exprnd[i] << ' ';
			BOOST_CHECK(exprnd[i] >=  0.0);
			BOOST_CHECK(exprnd[i] < std::numeric_limits<float>::infinity());
		}

		cout << endl;
	}

	URNG::seed(mtrng,0);
	URNG::generate(mtrng,rngbuf,8);
}

#include "FloatUVect2Distribution.hpp"

BOOST_AUTO_TEST_CASE(sincos_test)
{
	uint32_t rngbuf[8];
	float sincosrnd[16];

	DummyConstRNGVector constrng;
	boost::mt19937 mtrng;

	FloatUVect2Distribution sincos;

	for(const uint32_t s : std::vector<uint32_t>{ 0U, 1U, 511U, 512U, -1U })
	{
		URNG::seed(constrng,s);
		URNG::generate(constrng,rngbuf,8);

		sincos.calculate(constrng,sincosrnd);

		for(unsigned i=0;i<16;i += 2)
		{
			BOOST_CHECK(sincosrnd[i] 	>= -1.0);
			BOOST_CHECK(sincosrnd[i]  	<=  1.0);

			BOOST_CHECK(sincosrnd[i+1] 	>= -1.0);
			BOOST_CHECK(sincosrnd[i+1] 	<=  1.0);

			// Check that unit vector is indeed unit (note result tolerance is in terms of ||x||^2)
			BOOST_CHECK_SMALL(sincosrnd[i]*sincosrnd[i] + sincosrnd[i+1]*sincosrnd[i+1] - 1.0f,1e-12f);
		}

		cout << endl;
	}
}

#include "HenyeyGreenstein.hpp"

BOOST_AUTO_TEST_CASE(hgRandom)
{
	// Test parameters
	const float g = 0.99f;
	const unsigned N = 8000000U;

	//
	float sumg = 0.0f;
	float o[32];

	// DUT
	HenyeyGreenstein8f HG;

	// RNG
	BlockRNGAdaptor<boost::random::mt19937,uint32_t,32,8> rng;

	// Setup
	HG.gParam(g);

	for(unsigned i=0;i<N;i += 8)
	{
		HG.calculate(rng,o);

		for(unsigned j=0;j<8;++j)
		{
			float cosdefl = o[(j<<2)|0];
			float sindefl = o[(j<<2)|1];
			float cosaz   = o[(j<<2)|2];
			float sinaz   = o[(j<<2)|3];

			//cout << fixed << setprecision(6) << setw(9) << cosdefl << ',' << setw(9) << sindefl << "  " << setw(9) << cosaz << ',' << setw(9) << sinaz << endl;

			// check unit
			BOOST_CHECK_CLOSE(cosdefl*cosdefl + sindefl*sindefl, 1.0f, 1e-3f);
			BOOST_CHECK_CLOSE(cosaz  *cosaz   + sinaz  *sinaz  , 1.0f, 1e-3f);

			// accumulate average
			sumg += cosdefl;
		}
	}

	cout << "<cos(theta)> = " << sumg/float(N) << endl;

	// check <cos(theta)> = g
	BOOST_CHECK_SMALL(sumg/float(N) -g, 1e-4f);
}

BOOST_AUTO_TEST_CASE(hgExtremes)
{
	// Test parameters
	const float g = 0.9f;

	// DUT
	HenyeyGreenstein8f HG;

	// RNG
	BlockRNGAdaptor<DummyConstRNGVector,uint32_t,16,8> rng;
	float o[32];

	// Setup
	HG.gParam(g);

	for(unsigned s : std::vector<unsigned>{ 0U, 1U, 2U, 1U<<29, 2U<<29, 3U<<29, 4U<<29, 5U<<29, 6U<<29, -1U })
	{
		rng.rng().m_val=s;
		HG.calculate(rng,o);

		for(unsigned j=0;j<8;++j)
		{
			float cosdefl = o[(j<<2)|0];
			float sindefl = o[(j<<2)|1];
			float cosaz   = o[(j<<2)|2];
			float sinaz   = o[(j<<2)|3];

			cout << fixed << setprecision(6) << setw(9) << cosdefl << ',' << setw(9) << sindefl << "  " << setw(9) << cosaz << ',' << setw(9) << sinaz << endl;

			// check unit
			BOOST_CHECK_CLOSE(cosdefl*cosdefl + sindefl*sindefl, 1.0f, 1e-3f);
			BOOST_CHECK_CLOSE(cosaz  *cosaz   + sinaz  *sinaz  , 1.0f, 1e-3f);
		}
	}
}

struct Open_tag {};
struct Closed_tag {};

constexpr Open_tag Open{};
constexpr Closed_tag Closed{};

template<typename T,typename U,class Comp=std::less<T>>bool ocCompare(Open_tag,const T lhs,const U rhs,const Comp& c = std::less<T>())
{
	return c(lhs,rhs);
}

template<typename T,typename U,class Comp=std::less<T>>bool ocCompare(Closed_tag,const T lhs,const U rhs,const Comp& c = std::less<T>())
{
	return !c(rhs,lhs);
}

template<typename T,typename LowerType=Open_tag,typename UpperType=Closed_tag,class Comp=std::less<T>>struct Interval
{
	T lower;
	T upper;

	Comp comp;

	Interval(T lb,T ub,Comp c=Comp()) : lower(lb),upper(ub),comp(c){}

	bool operator()(const T x) const { return ocCompare(LowerType(),lower,x,comp) && ocCompare(UpperType(),x,upper,comp); }
};

BOOST_AUTO_TEST_CASE(comparisons)
{
	BOOST_CHECK(!ocCompare(Open,1U,1U));
	BOOST_CHECK( ocCompare(Open,1U,2U));
	BOOST_CHECK(!ocCompare(Open,2U,1U));

	BOOST_CHECK(!ocCompare(Closed,1U,0U));
	BOOST_CHECK( ocCompare(Closed,1U,1U));
	BOOST_CHECK( ocCompare(Closed,1U,2U));

	Interval<float,Closed_tag,Open_tag> i(0.0f,1.0f);

	BOOST_CHECK(i(0.0f));
	BOOST_CHECK(!i(1.0f));
	BOOST_CHECK(!i(-1e-9f));
	BOOST_CHECK(!i(std::numeric_limits<float>::infinity()));
	BOOST_CHECK(!i(-std::numeric_limits<float>::infinity()));
	BOOST_CHECK(!i(std::numeric_limits<float>::quiet_NaN()));
}

// quartiles
// mean
// variance

// scalar -> 1:1 correspondence to ICDF
// vector -> no simple relationship
