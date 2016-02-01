/*
 * Test_Isotropic.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_MODULE Test_Emitters
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <array>

#include "IsotropicFixture.hpp"
#include "UnitVectorFixture.hpp"
#include "OrthogonalFixture.hpp"
#include "PacketDirectionFixture.hpp"

#include "../../RandomAVX.hpp"

#include <boost/range/algorithm.hpp>



#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

boost::random::mt19937_64 			rng;
boost::random::uniform_01<float>	 u01;



/** Reference implementation that looks OK and passes unit tests */

LaunchPacket launchIsotropic(std::array<float,3> p)
{
	float lambda = u01(rng)*boost::math::constants::two_pi<float>();
	std::array<float,2> azuv{ std::cos(lambda),std::sin(lambda) };

	float sinphi = 2.0f*u01(rng)-1.0f;
	float cosphi = sqrtf(1.0-sinphi*sinphi);

	std::array<float,3> d{ azuv[0]*cosphi , azuv[1]*cosphi,  sinphi };
	std::array<float,3> a{ azuv[0]*sinphi,  azuv[1]*sinphi, -cosphi };
	std::array<float,3> b{ -azuv[1], azuv[0],  0 };

	LaunchPacket lpkt;

	lpkt.dir.d = SSE::UnitVector3(SSE::Vector3(d),SSE::NoCheck);
	lpkt.dir.a = SSE::UnitVector3(SSE::Vector3(a),SSE::NoCheck);
	lpkt.dir.b = SSE::UnitVector3(SSE::Vector3(b),SSE::NoCheck);

	lpkt.pos = SSE::Vector3(p);
	return lpkt;
}

//std::array<float,3> P{ 1.0, 2.0, 3.0};

//	Sources::PointSource PS(P);
//
//	PS.acceptVisitor(f);
//
//	auto srcs = f.csources();
//
//	BOOST_REQUIRE(boost::size(srcs)==1);

#include "IsotropicFixture.hpp"
#include "PointFixture.hpp"

#include "../Point.hpp"
#include "../Isotropic.hpp"


typedef SourceFixture<PointFixture,IsotropicFixture> IsoPS;

BOOST_FIXTURE_TEST_CASE(isops,IsoPS)
{
	std::array<float,3> p{ 1.0, 2.0, 3.0 };
	positionFixture.m_p = std::array<float,3>{ p[0], p[1], p[2] };

	Emitter::Isotropic<RNG_SFMT_AVX> is;
	Emitter::Point ps = Emitter::Point(SSE::Vector3(p));

	Emitter::PositionDirectionEmitter<RNG_SFMT_AVX,Emitter::Point,Emitter::Isotropic<RNG_SFMT_AVX>> ips(ps,is);

	for(unsigned i=0;i<100000;++i)
		testPacket(ips.emit(rng));

	write("isops");
}

BOOST_FIXTURE_TEST_CASE(dummyps, IsoPS)
{
	std::array<float,3> p{ 1.0, 2.0, 3.0 };
	positionFixture.m_p = std::array<float,3>{ p[0], p[1], p[2] };

	for(unsigned i=0;i<100000;++i)
		testPacket(launchIsotropic(p));

	write("dummyps");
}

#include "TetraFixture.hpp"

#include "../Tetra.hpp"

using namespace std;

typedef RNG_SFMT_AVX RNG;

typedef SourceFixture<TetraFixture,IsotropicFixture> IsoTet;

BOOST_FIXTURE_TEST_CASE(tetra, IsoTet)
{
	std::array<float,3> A { 1.0, 2.0, 3.0 };
	std::array<float,3> B { 2.0, 2.0, 3.0 };
	std::array<float,3> C { 1.0, 3.0, 3.0 };
	std::array<float,3> D { 1.0, 2.0, 4.0 };

	positionFixture = TetraFixture(A,B,C,D);

	Emitter::Tetra<RNG> T{SSE::Vector3(A),SSE::Vector3(B),SSE::Vector3(C),SSE::Vector3(D)};
	Emitter::Isotropic<RNG> iso;

	Emitter::PositionDirectionEmitter<RNG,Emitter::Tetra<RNG>,Emitter::Isotropic<RNG>> tet(T,iso);


	cout << "Isotropic tetrahedron source" << endl;
	for(unsigned i=0;i<100000;++i)
		testPacket(tet.emit(rng));

	write("tetra");
}


#include "TriangleFixture.hpp"
#include "DirectedFixture.hpp"

#include "../Directed.hpp"
#include "../Triangle.hpp"

typedef SourceFixture<TriangleFixture,DirectedFixture> NormTri;

BOOST_FIXTURE_TEST_CASE(triangle,NormTri)
{
	std::array<float,3> A{ -1.0, 2.0, 3.0 };
	std::array<float,3> B{  1.0, 2.5, 2.0 };
	std::array<float,3> C{  2.0, 2.25, 1.25 };

	std::array<float,3> AC{ C[0]-A[0], C[1]-A[1], C[2]-A[2] };
	std::array<float,3> AB{ B[0]-A[0], B[1]-A[1], B[2]-A[2] };

	SSE::UnitVector3 dir{SSE::UnitVector3(SSE::Vector3(normalize(cross(AB,AC))),SSE::NoCheck)};

	positionFixture = TriangleFixture(A,B,C);
	directionFixture.m_dir = dir.array();

	Emitter::Triangle<RNG> 	T{SSE::Vector3(A), SSE::Vector3(B), SSE::Vector3(C)};
	Emitter::Directed		d{dir};

	Emitter::PositionDirectionEmitter<RNG,Emitter::Triangle<RNG>,Emitter::Directed> tri(T,d);


	//cout << "Directed triangular face with normal " <<
	cout << "Directed triangular face" << endl;
	for(unsigned i=0;i<100000;++i)
	{
		LaunchPacket lpkt = tri.emit(rng);
		testPacket(lpkt);
//
//		cout << lpkt.dir.d.array()[0] << ',' << lpkt.dir.d.array()[1] << ',' << lpkt.dir.d.array()[2] << endl;
//		cout << lpkt.dir.a.array()[0] << ',' << lpkt.dir.a.array()[1] << ',' << lpkt.dir.a.array()[2] << endl;
//		cout << lpkt.dir.b.array()[0] << ',' << lpkt.dir.b.array()[1] << ',' << lpkt.dir.b.array()[2] << endl;
	}

	write("tri");
}

typedef SourceFixture<PointFixture,DirectedFixture> Pencil;

BOOST_FIXTURE_TEST_CASE(pencil,Pencil)
{
	std::array<float,3> p{ 0.5f, 1.0f, 0.4f };
	std::array<float,3> d{ 0.65f, -0.1f, -0.2f };

	SSE::UnitVector3 dir = normalize(SSE::Vector3(d));

	positionFixture = PointFixture(p);
	directionFixture = DirectedFixture(dir.array());

	Emitter::Point P{SSE::Vector3(p)};
	Emitter::Directed D{dir};

	Emitter::PositionDirectionEmitter<RNG,Emitter::Point,Emitter::Directed> pb(P,D);

	for(unsigned i=0;i<100000;++i)
		testPacket(pb.emit(rng));

	write("pencil");
}

#include "LineFixture.hpp"
#include "DiskFixture.hpp"
#include "../Line.hpp"
#include "../Disk.hpp"

typedef SourceFixture<LineFixture,DiskFixture> Line;

BOOST_FIXTURE_TEST_CASE(line,Line)
{
	std::array<float,3> p0{ 0.5f, 1.0f, -0.2f };
	std::array<float,3> p1{ 1.2f, 0.8f, 0.6f  };

	std::array<float,3> n { p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2] };

	positionFixture = LineFixture(p0,p1);
	directionFixture = DiskFixture(n);

	Emitter::Line<RNG> L{SSE::Vector3(p0),SSE::Vector3(p1)};
	Emitter::Disk<RNG> D{SSE::normalize(SSE::Vector3(p1)-SSE::Vector3(p0))};

	Emitter::PositionDirectionEmitter<RNG,Emitter::Line<RNG>,Emitter::Disk<RNG>> ls(L,D);

	for(unsigned i=0;i<100000;++i)
		testPacket(ls.emit(rng));

	write("line");
}

