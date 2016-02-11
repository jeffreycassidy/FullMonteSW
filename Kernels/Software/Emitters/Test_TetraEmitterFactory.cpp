/*
 * Test_EmitterFactory.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_MODULE Test_EmitterFactory
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <FullMonte/Storage/TIMOS/TIMOSAntlrParser.hpp>

#include <FullMonte/Kernels/Software/RandomAVX.hpp>

#include <FullMonte/Kernels/Software/Emitters/TetraMeshEmitterFactory.hpp>

#include <iostream>
#include <iomanip>
#include <array>

#include "../Packet.hpp"

#include "Base.hpp"

#include <FullMonte/Storage/TIMOS/TIMOSWriter.hpp>

using namespace std;

#include "TetraMeshEmitterFactory.cpp"


/** Uses the TIM-OS test file "mouse" to test loading of multiple tetra sources */

BOOST_AUTO_TEST_CASE(mouse)
{
	TIMOSAntlrParser R;

	R.setMeshFileName("/Users/jcassidy/src/FullMonteSW/data/mouse.mesh");
	R.setSourceFileName("/Users/jcassidy/src/FullMonteSW/data/mouse.source");

	TetraMesh M = R.mesh();
	Source::Base* src = R.sources();

	Emitter::TetraEmitterFactory<RNG_SFMT_AVX> factory(&M);

	TIMOSWriter W("foo");

	src->acceptVisitor(&factory);

	RNG_SFMT_AVX rng;



	auto S = factory.cemitters();

	BOOST_REQUIRE(boost::size(S) != 0);
	BOOST_REQUIRE(S[0] != nullptr);

	cout << "Constructed " << boost::size(S) << " emitters" << endl;

	for(unsigned i=0;i<10;++i)
	{
		LaunchPacket lpkt = S[0]->emit(rng);

		std::array<float,3> pos = lpkt.pos.array();
		std::array<float,3> d = lpkt.dir.d.array();
		//std::array<float,3> a = lpkt.dir.a.array();
		//std::array<float,3> b = lpkt.dir.b.array();

		cout << std::fixed << std::setprecision(4) <<
				std::setw(9) << pos[0] << ',' << std::setw(9) << pos[1] << ',' << std::setw(9) << pos[2] << "  dir " <<
				std::setw(7) << d[0]   << ',' << std::setw(7) << d[1]   << ',' << std::setw(7) << d[2]   << endl;
	}
}

//BOOST_AUTO_TEST_CASE(pencil)
//{
//	TIMOSAntlrParser R;
//
//	R.setSourceFileName("/Users/jcassidy/src/TLT_Knee/preliminary.source");
//
//	TIMSO
//
//	Source::Base* src = R.sources();
//
//	Emitter::TetraEmitterFactory<RNG_SFMT_AVX> factory(&M);
//
//	src->acceptVisitor(&factory);
//
//	RNG_SFMT_AVX rng;
//
//	auto S = factory.cemitters();
//
//	cout << "Constructed " << boost::size(S) << " emitters" << endl;
//
//	for(unsigned i=0;i<10;++i)
//	{
//		LaunchPacket lpkt = S[0]->emit(rng);
//
//		std::array<float,3> pos = lpkt.pos.array();
//		std::array<float,3> d = lpkt.dir.d.array();
//		std::array<float,3> a = lpkt.dir.a.array();
//		std::array<float,3> b = lpkt.dir.b.array();
//
//		cout << std::fixed << std::setprecision(4) <<
//				std::setw(9) << pos[0] << ',' << std::setw(9) << pos[1] << ',' << std::setw(9) << pos[2] << "  dir " <<
//				std::setw(7) << d[0]   << ',' << std::setw(7) << d[1]   << ',' << std::setw(7) << d[2]   << endl;
//	}
//}




/** Uses the TIM-OS test file "FourLayer" to test loading of a pencil beam source */

#include "Test/SourceFixture.hpp"
#include "Test/PointFixture.hpp"
#include "Test/DirectedFixture.hpp"

#include <FullMonte/Kernels/Software/Emitters/TetraMeshEmitterFactory.hpp>

typedef SourceFixture<PointFixture,DirectedFixture> Pencil;

BOOST_FIXTURE_TEST_CASE(FourLayer, Pencil)
{
	TIMOSAntlrParser R;

	R.setSourceFileName("/Users/jcassidy/src/FullMonteSW/data/FourLayer.source");
	positionFixture.m_p = std::array<float,3>{ 0.0f, 0.0f, 0.0f };
	directionFixture.m_dir = std::array<float,3> { 0.0f, 0.0f, 1.0f };

	R.setMeshFileName("/Users/jcassidy/src/FullMonteSW/data/FourLayer.mesh");

	TetraMesh M = R.mesh();
	Source::Base* src = R.sources();

	RNG_SFMT_AVX rng;

	Emitter::TetraEmitterFactory<RNG_SFMT_AVX> factory(&M);

	src->acceptVisitor(&factory);

	auto S = factory.cemitters();

	BOOST_REQUIRE(boost::size(S) == 1);

	LaunchPacket lpkt = S[0]->emit(rng);

	BOOST_CHECK_EQUAL(lpkt.element, 4537U);
	testPacket(lpkt);
}




/** Uses the TIM-OS test file "cube_5med" to test loading of a point source */

#include "Test/IsotropicFixture.hpp"

typedef SourceFixture<PointFixture,IsotropicFixture> IsoPS;

BOOST_FIXTURE_TEST_CASE(Cube_5Med, IsoPS)
{
	TIMOSAntlrParser R;

	R.setSourceFileName("/Users/jcassidy/src/FullMonteSW/data/cube_5med.source");
	positionFixture.m_p = std::array<float,3>{ 0.3f, 1.31f, 1.333f };

	R.setMeshFileName("/Users/jcassidy/src/FullMonteSW/data/cube_5med.mesh");

	TetraMesh M = R.mesh();
	Source::Base* src = R.sources();

	RNG_SFMT_AVX rng;

	Emitter::TetraEmitterFactory<RNG_SFMT_AVX> factory(&M);

	src->acceptVisitor(&factory);

	auto S = factory.cemitters();

	BOOST_REQUIRE(boost::size(S) == 1);

	LaunchPacket lpkt = S[0]->emit(rng);

	BOOST_CHECK_EQUAL(lpkt.element, 25515U);
	testPacket(lpkt);
}
