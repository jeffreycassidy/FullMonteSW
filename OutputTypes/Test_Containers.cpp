/*
 * Test_Containers.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test_Containers

#include <boost/test/unit_test.hpp>

#include "DenseVector.hpp"
#include "SparseVector.hpp"

#include <vector>
#include <iostream>

#include <FullMonte/OutputTypes/SpatialMapBase.hpp>

#include "FluenceMapBase.hpp"

using namespace std;

template<class Value,class Index>class Fixture
{
public:
	void genGolden(std::vector<Value>&& v)
	{
		golden_nnz=0;
		for(unsigned i=0;i<v.size();++i)
			golden_nnz += v[i] != 0;

		golden_v = std::move(v);
		golden_dim=golden_v.size();

		BOOST_REQUIRE_EQUAL(v.size(),0U);
	}

	template<class Container>void test(const Container& C)
	{
		BOOST_CHECK_EQUAL(C.nnz(), golden_nnz);
		BOOST_CHECK_EQUAL(C.dim(), golden_dim);

		BOOST_CHECK_EQUAL_COLLECTIONS(begin(C.values()), end(C.values()), begin(golden_v), end(golden_v));

		BOOST_CHECK_EQUAL(boost::size(C.nonzeros()), 	golden_nnz);
		BOOST_CHECK_EQUAL(boost::size(C.dense()), 		golden_dim);

		for(unsigned i=0;i<golden_dim;++i)
			BOOST_CHECK_EQUAL(C[i], golden_v[i]);
	}

	template<class Container>void print(const Container& C)
	{
		std::cout << "Values: ";
		for(const auto v : C.values())
			std::cout << v << ' ';
		std::cout << std::endl << "Sparse: ";
		for(const auto s : C.nonzeros())
			std::cout << '(' << s.first << ": " << s.second << ") ";
		std::cout << std::endl << "Dense: ";
		for(const auto d : C.dense())
			std::cout << '(' << d.first << ": " << d.second << ") ";
		std::cout << std::endl;
	}

	std::vector<Value> golden_v;
	unsigned golden_nnz;
	unsigned golden_dim;
};

typedef Fixture<float,unsigned> FixtureFU;

BOOST_FIXTURE_TEST_CASE(sparseFromDense, FixtureFU )
{
	genGolden({0,10.0,0,0,40.0,0,60.0});

	SparseVector<float,unsigned> sv(value_range_t(),golden_v);

	test(sv);
	print(sv);
}

BOOST_FIXTURE_TEST_CASE(denseFromDense, FixtureFU )
{
	genGolden({0,10.0,0,0,40.0,0,60.0});

	DenseVector<float,unsigned> dv(golden_v);

	test(dv);
	print(dv);
}

BOOST_AUTO_TEST_CASE(dense)
{
	cout << "====== DENSE" << endl;
	std::vector<double> v{ 1.0, 2.0, 3.0, 0.0, 5.0, 0.0 };

	SpatialMapBase<double,unsigned>* md = SpatialMapBase<double,unsigned>::newFromVector(v);

	cout << "Nonzeros: " << endl;
	for(const auto el : md->nonzeros())
		cout << el.first << ':' << el.second << ' ' << endl;

	cout << "Dense: " << endl;
	for(const auto el : md->dense())
			cout << el.first << ':' << el.second << ' ' << endl;

	cout << "Values: " << endl;
	for(const auto el : md->values())
			cout << el << ' ' << endl;


	//SpatialMapBase<double,unsigned long long> *mf = SpatialMapBase<float,unsigned long long>::newFromVector(v);
}

BOOST_AUTO_TEST_CASE(sparse10)
{
	cout << "====== SPARSE10" << endl;
	std::vector<double> v{ 1.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 9.0, 0.0 };

	SpatialMapBase<double,unsigned>* md = SpatialMapBase<double,unsigned>::newFromVector(v);

	BOOST_CHECK_EQUAL(md->dim(),v.size());
	BOOST_CHECK_EQUAL(md->nnz(),3U);

	cout << "Nonzeros: " << endl;
	for(const auto el : md->nonzeros())
		cout << el.first << ':' << el.second << ' ' << endl;

	cout << "Dense: " << endl;
	for(const auto el : md->dense())
			cout << el.first << ':' << el.second << ' ' << endl;

	cout << "Values: " << endl;
	for(const auto el : md->values())
			cout << el << ' ' << endl;
}

BOOST_AUTO_TEST_CASE(volwrap)
{
	cout << "===== VolumeAbsorptionWrapper" << endl;
	std::vector<float> v{ 1.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 9.0f, 0.0f };

	SpatialMapBase<float,unsigned>* m = SpatialMapBase<float,unsigned>::newFromVector(v);

	VolumeAbsorbedEnergyMap VW(m);

	for(const auto el : m->nonzeros())
		cout << el.first << ':' << el.second << ' ' << endl;

	for(const auto el : m->dense())
			cout << el.first << ':' << el.second << ' ' << endl;

	for(const auto el : m->values())
			cout << el << ' ' << endl;

	BOOST_CHECK_CLOSE(VW->sum(), 15.0, 1e-4);
	BOOST_CHECK_EQUAL(VW->nnz(),3U);
	BOOST_CHECK_EQUAL(VW->dim(),10U);
}




