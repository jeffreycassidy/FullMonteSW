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
#include <iostream>

#include "SourceFixture.hpp"
#include "IsotropicFixture.hpp"
#include "UnitVectorFixture.hpp"
#include "OrthogonalFixture.hpp"
#include "PacketDirectionFixture.hpp"

#include "../../../../Geometry/Sources/Line.hpp"

#include <FullMonte/Geometry/TetraMesh.hpp>

#include "../TetraMeshEmitterFactory.hpp"


#include "../Base.hpp"

#include "../../RNG_SFMT_AVX.hpp"

#include <boost/range/adaptor/indexed.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

#include <FullMonte/Storage/TIMOS/TIMOSAntlrParser.hpp>


const TetraMesh mouse;

struct GlobalMouseFixture
{
	GlobalMouseFixture()
	{
		TIMOSAntlrParser R;
		R.setMeshFileName("/Users/jcassidy/src/FullMonteSW/data/TIM-OS/mouse/mouse.mesh");

		const_cast<TetraMesh&>(mouse) = R.mesh();
	}

	~GlobalMouseFixture()
	{
	}

};

BOOST_GLOBAL_FIXTURE(GlobalMouseFixture);

boost::random::mt19937_64 			rng;
boost::random::uniform_01<float>	 u01;

using namespace std;

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
	Emitter::Point ps = Emitter::Point(1,SSE::Vector3(p));

	Emitter::PositionDirectionEmitter<RNG_SFMT_AVX,Emitter::Point,Emitter::Isotropic<RNG_SFMT_AVX>> ips(ps,is);

	cout << "Isotropic tetrahedron source (1)" << endl;
	for(unsigned i=0;i<100000;++i)
		testPacket(ips.emit(rng));

	write("isops");
}

BOOST_FIXTURE_TEST_CASE(dummyps, IsoPS)
{
	std::array<float,3> p{ 1.0, 2.0, 3.0 };
	positionFixture.m_p = std::array<float,3>{ p[0], p[1], p[2] };

	cout << "Isotropic point source" << endl;
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

	Emitter::Tetra<RNG> T{1,SSE::Vector3(A),SSE::Vector3(B),SSE::Vector3(C),SSE::Vector3(D)};
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

	Emitter::Triangle<RNG> 	T{4, SSE::Vector3(A), SSE::Vector3(B), SSE::Vector3(C)};
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

	Emitter::Point P{1,SSE::Vector3(p)};
	Emitter::Directed D{dir};

	Emitter::PositionDirectionEmitter<RNG,Emitter::Point,Emitter::Directed> pb(P,D);

	cout << "Pencil beam" << endl;
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
	// random line within the Digimouse
	std::array<float,3> p0{ 23.5f, 14.8f, 11.8f };
	std::array<float,3> p1{ 16.6f, 21.6f, 14.8f  };

	std::array<float,3> n = p1-p0;

	positionFixture = LineFixture(p0,p1);
	directionFixture = DiskFixture(n);

	Source::Line L(1.0f,p0,p1);
	L.pattern(Source::Line::Normal);

	Emitter::TetraEmitterFactory<RNG> F(&mouse);

	F.visit(&L);

	Emitter::EmitterBase<RNG>* e = F.cemitters()[0];

	std::cout << "Line source" << std::endl;
	for(unsigned i=0;i<10000;++i)
	{
		LaunchPacket lp = e->emit(rng);
		cout << lp.pos[0] << ' ' << lp.pos[1] << ' ' << lp.pos[2] << " tet " << lp.element << endl;
		testPacket(lp);

		BOOST_CHECK(lp.element != 0);

		std::array<float,4> vf;
		_mm_store_ps(vf.data(),mouse.getTetra(lp.element).heights(to_m128(lp.pos.array())));

		bool inside_tetra = !boost::algorithm::any_of(vf, [](float i){ return i < 0; });
		if (!inside_tetra)
			{
			cout << "ERROR: Point not inside tetra" << endl;
			for(unsigned i=0;i<4;++i)
				cout << ' ' << vf[i];
			cout << endl;
			}
		BOOST_CHECK(inside_tetra);
	}

	write("line");
}

#include <vtkCellArray.h>
#include <vtkPolyDataWriter.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkCellData.h>

#include <FullMonte/Geometry/Filters/TriFilterRegionBounds.hpp>

BOOST_AUTO_TEST_CASE( mouseSurfaceFaces )
{
	vector<pair<array<float,3>,array<float,3>>> P{
		make_pair(array<float,3>{37.7, 44.8, 30.1}, array<float,3>{16.3, 36.9,  14.7}),		// outside in
		make_pair(array<float,3>{16.2, 60.8,  7.9}, array<float,3>{11.6, 61.2, -16.2})		// inside out
	};


	// set up to check external surface of the mouse
	TriFilterRegionBounds TF(&mouse);
	TF.includeRegion(0,true);
	TF.includeRegion(18,false);
	TF.bidirectional(true);

	// vis: the specification (start/end point without surface intersection)
	vtkPoints *specP = vtkPoints::New();
	vtkCellArray *specCA = vtkCellArray::New();
	vtkUnsignedIntArray *specNum = vtkUnsignedIntArray::New();


	// vis: the realized probe (line truncated at surface, surface tri, and incoming tet)
	vtkUnstructuredGrid* probe = vtkUnstructuredGrid::New();
	vtkPoints *probeP = vtkPoints::New();

	probe->SetPoints(probeP);

	vtkUnsignedIntArray *probeNum = vtkUnsignedIntArray::New();

	probe->GetCellData()->SetScalars(probeNum);

	for(const auto p : P | boost::adaptors::indexed(0U))
	{
		array<float,3> dir = normalize(p.value().second-p.value().first);

		// insert probe specifier
		specNum->InsertNextValue(p.index());

		specCA->InsertNextCell(2);
		specCA->InsertCellPoint(specP->InsertNextPoint(p.value().first.data()));
		specCA->InsertCellPoint(specP->InsertNextPoint(p.value().second.data()));

		RTIntersection res = mouse.findSurfaceFace(p.value().first,dir,&TF);

		// insert probe realization
		vtkIdType lineIDs[2]{ probeP->InsertNextPoint(p.value().first.data()), probeP->InsertNextPoint(res.q.data()) };

		FaceByPointID IDps = mouse.getFacePointIDs(abs(res.IDf));

		vtkIdType triIDs[3]{
			probeP->InsertNextPoint(mouse.getPoint(IDps[0]).data()), probeP->InsertNextPoint(mouse.getPoint(IDps[1]).data()), probeP->InsertNextPoint(mouse.getPoint(IDps[2]).data()) };

		TetraByPointID tetIDps = mouse.getTetraPointIDs(res.IDt);
		vtkIdType tetIDs[4];
		for(unsigned i=0;i<4;++i)
			tetIDs[i] = probeP->InsertNextPoint(mouse.getPoint(tetIDps[i]).data());

		probe->InsertNextCell(VTK_LINE,2,lineIDs);
		probe->InsertNextCell(VTK_POLYGON,3,triIDs);

		probe->InsertNextCell(VTK_TETRA,4,tetIDs);

		probeNum->InsertNextValue(p.index());
		probeNum->InsertNextValue(p.index());
		probeNum->InsertNextValue(p.index());
	}

	vtkPolyData *spec = vtkPolyData::New();

	spec->SetPoints(specP);
	spec->SetLines(specCA);
	spec->GetCellData()->SetScalars(specNum);

	vtkPolyDataWriter *W = vtkPolyDataWriter::New();
	W->SetFileName("line.spec.vtk");
	W->SetInputData(spec);
	W->Update();


	vtkUnstructuredGridWriter *UGW = vtkUnstructuredGridWriter::New();
	UGW->SetInputData(probe);
	UGW->SetFileName("line.probe.vtk");
	UGW->Update();

//
//	vtkPoints *probeP=vtkPoints::New();
//	vtkCellArray *probeCA=vtkCellArray::New();
//	vtkCellArray *probeTri=vtkCellArray::New();
//
//	vtkPolyData *probe=vtkPolyData::New();
//	vtkUnsignedArray *probeNum=vtkUnsignedArray::New();
//
//	for(const auto p : P | boost::adaptors::indexed(0U))
//	{

//
//		probeTri->InsertNextCell(3);
//		probeTri->InsertCellPoint(probeP->InsertNextPoint());
//		probeTri->InsertCellPoint(probeP->InsertNextPoint());
//		probeTri->InsertCellPoint(probeP->InsertNextPoint());
//
//		probeNum->InsertNextTuple(p.index());
//	}
//
//	probe->SetPoints(probeP);
//	probe->SetLines(probeCA);
//	probe->GetCellData()->SetScalars(probeNum);
}

